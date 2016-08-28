// 140407 fahr: LOG_CALL_TASK_SWIT moved to new handler logPutTaskSwitch()

// FreeRTOS includes
#include <FreeRTOS.h>
#include "task.h"
#ifdef _WIN_PIPE_
  #include "trace\frtos_pipe.h"
#else
  #include "FreeRTOS_UDP_IP.h"
  #include "FreeRTOS_Sockets.h"
#endif // #ifdef _WIN_PIPE_

#ifdef LOG_TASK_NAME_IN					// macros: task may be traced or hide activity
  #define LOG_MASK_TASK_LOGI() 	logGetCurrentTaskId(); 	logFilterMask |= 1UL << logTraceSet.id;
#else
  #define LOG_MASK_TASK_LOGI()
#endif // LOG_TASK_NAME_IN

#ifdef LOG_TASK_NAME_OUT
  #define LOG_MASK_TASK_LOGO() 	logGetCurrentTaskId(); 	logFilterMask |= 1UL << logTraceSet.id;
#else
  #define LOG_MASK_TASK_LOGO()
#endif // LOG_TASK_NAME_OUT

#ifdef LOG_TASK_NAME_UDP
  #define LOG_MASK_TASK_UDP() 	if( strncmp( logTraceSet.name, LOG_TASK_NAME_UDP, configMAX_TASK_NAME_LEN ) == 0 ) { logFilterMask |= 1UL << logTraceSet.para; }
#else 	
  #define LOG_MASK_TASK_UDP()
#endif // LOG_TASK_NAME_UDP

#ifdef LOG_TASK_NAME_TMR
  #define LOG_MASK_TASK_TMR() 	if( strncmp( logTraceSet.name, LOG_TASK_NAME_TMR, configMAX_TASK_NAME_LEN ) == 0 ) { logFilterMask |= 1UL << logTraceSet.para; }
#else 	
  #define LOG_MASK_TASK_TMR()
#endif // LOG_TASK_NAME_TMR

#ifdef LOG_TASK_NAME_MAC
  #define LOG_MASK_TASK_MAC() 	if( strncmp( logTraceSet.name, LOG_TASK_NAME_MAC, configMAX_TASK_NAME_LEN ) == 0 ) { logFilterMask |= 1UL << logTraceSet.para; }
#else 	
  #define LOG_MASK_TASK_MAC()
#endif // LOG_TASK_NAME_MAC
  
#ifdef LOG_TASK_NAME_IDLE
  #define LOG_MASK_TASK_IDLE() 	if( strncmp( logTraceSet.name, LOG_TASK_NAME_IDLE, configMAX_TASK_NAME_LEN ) == 0 ) { logFilterMask |= 1UL << logTraceSet.para; }
#else 	
  #define LOG_MASK_TASK_IDLE()
#endif // LOG_TASK_NAME_IDLE

#ifdef LOG_TASK_NAME_COM
  #define LOG_MASK_TASK_COM() 	if( strncmp( logTraceSet.name, LOG_TASK_NAME_COM, configMAX_TASK_NAME_LEN ) == 0 ) { logFilterMask |= 1UL << logTraceSet.para; }
#else 	
  #define LOG_MASK_TASK_COM()
#endif // LOG_TASK_NAME_COM
  
#ifdef LOG_FILTER_CREATE_TASK
	#define	LOG_MASK_TCREATE()	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!
#else
	#define	LOG_MASK_TCREATE()
#endif // LOG_FILTER_CREATE


// 140319 show taskCreate for first usertask "APPL"
#ifdef LOG_TASK_CREATE_APPL
  #define LOG_MASK_TASK_APPL()	if( strncmp( logTraceSet.name, LOG_TASK_NAME_APPL, configMAX_TASK_NAME_LEN ) != 0 )
#else 	
  #define LOG_MASK_TASK_APPL()
#endif // LOG_TASK_CREATE_APPL
  
struct log_t logTraceSet;				// log data set, trace message assembled here and then written by lobBufferPlace() into the ringbuffer 

static portBASE_TYPE logFilterMask;		// first (here 31) task can be masked out from trace, unkown task or scheduler not running never traced
#define LOG_FILTER_MASK_BITS (sizeof(logFilterMask)*8)	// 32 bits total here

static enum { LOG_TRACE_ON, LOG_TRACE_OFF } logHideTrace;

#define LOG_ERR_OK			0			// no error detected
#define LOG_ERR_FAIL		-1			// error detected
static portBASE_TYPE logErrorFlag;

#ifndef _WIN_PIPE_
 static xSocket_t xSocket;				// socket to be used
 static struct freertos_sockaddr xClient;	// with address and port info
 static socklen_t xClientAddrLen = sizeof( struct freertos_sockaddr );	// required as a parameter to maintain the sendto() Berkeley sockets API - but it is not actually used so can take any value
#endif // #ifndef _WIN_PIPE_

#define TRACE_BUFFER_LENGTH 1024
static char logBuffer[TRACE_BUFFER_LENGTH];
static struct { char * base; char * read; char * write; char * end; size_t free; size_t used; } logRB;	// log ring buffer structure

xTaskHandle thLogIn = NULL;
xTaskHandle thLogOut = NULL;
// UDP-helper
static void udpWrite( const void *buf, size_t cnt )
{
#ifndef _WIN_PIPE_
	size_t rc;

	if( (rc = FreeRTOS_sendto( xSocket, buf, cnt, 0, &xClient, sizeof( xClient ) ) ) != cnt )
#else
	if( pipeWrite( buf, cnt ) == RET_PIPE_FAIL )
#endif // #ifndef _WIN_PIPE_
	{
		vTaskEndScheduler(); // exit freeRTOS, if UDP-write fail
	}
}


void logSendMessage( const void *buf, size_t cnt )
{
	struct msg_t msg;

	msg.cmd = LOG_CMD_MSG;
	msg.count = (uint8_t)cnt;

	taskENTER_CRITICAL();				// message must not be fragmented 
	udpWrite( &msg, sizeof( msg ) );
	udpWrite( buf, cnt );
	taskEXIT_CRITICAL(); 
}


// ring buffer
void logBufferInit( void )				// initialize logBuffer and vars, must be called first
{
	logFilterMask = 1 << 0;				// BIT0 set => scheduler not running or unknown task
	logHideTrace = LOG_TRACE_OFF;
	logErrorFlag = LOG_ERR_OK;

	logRB.base = logRB.read = logRB.write = logBuffer;
	logRB.end  = logBuffer + TRACE_BUFFER_LENGTH;
	logRB.free = TRACE_BUFFER_LENGTH;
	logRB.used = 0;
}

 
// return LOG_ERR_FAIL, if there aren´t place for count chars
// return LOG_ERR_OK, if count chars from src are copied to ring buffer
// should be in critical section when called by traceMACRO
portBASE_TYPE logBufferPlace( const char * src, size_t count )
{
	portBASE_TYPE rc = LOG_ERR_FAIL;
	size_t blkCnt;
	size_t blkRest;
	
	if( logRB.free >= count )
	{
		if( (blkCnt = logRB.end - logRB.write) >= count ) { blkCnt = count; }
		
		memcpy( logRB.write, src, blkCnt );	// all copied or til end of ringbuffer

		if( (logRB.write += blkCnt) == logRB.end )
		{
			if( blkCnt != count )
			{
				blkRest = count - blkCnt; 
				memcpy( logRB.base, src + blkCnt, blkRest );	// and the rest, if needed
				logRB.write = logRB.base + blkRest;
			}
		}
		logRB.used += count;
		logRB.free -= count;
		rc = LOG_ERR_OK;
	}
	return rc;
}


// log tasks
static void taskLogOut( void *pvParameters )	// remote IP is parameter 
{
	size_t count;
	const struct clk_t LOG_CLOCK_INFO = { LOG_CMD_CLOCK, configTICK_RATE_HZ };	// report current TickRate

	(void) pvParameters;				// just to prevent compiler warnings.

	LOG_MASK_TASK_LOGO();				// disable or enable trace
		
	udpWrite( &LOG_CLOCK_INFO, sizeof( LOG_CLOCK_INFO ) );
	logHideTrace = LOG_TRACE_ON;
	startApplTask();					// trace established, now enter user application

	for( ;; )
	{
		taskENTER_CRITICAL();			// ringbuffer must not be fragmented 
		count = (logRB.used > (size_t)(logRB.end - logRB.read) ? (logRB.end - logRB.read) : logRB.used);	// bytes til physical limit or all used

		if( count != 0 )				// there are some bytes in ring buffer
		{
			udpWrite( logRB.read, count );
			if( (logRB.read += count) == logRB.end )	// adjust read pointer, still more byte in buffer?
			{
				count = logRB.used - count;	// that the rest size
				udpWrite( logRB.base, count );
				logRB.read = logRB.base + count;	// adjust read pointer
			}
			logRB.used = 0;				// buffer is empty now
			logRB.free = TRACE_BUFFER_LENGTH;

			if( logErrorFlag != LOG_ERR_OK )	// buffer overflow detected?
			{
				logTraceSet.cmd = LOG_CMD_ERRBUFF;
				udpWrite( (char *)&logTraceSet, sizeof( logTraceSet.cmd ) );
			}
		}
		else
		{
			vTaskSuspend(NULL);					// force task switch
		}
		taskEXIT_CRITICAL(); 
	}
}


// CLI/Supervisor: Wait for Connect and send ACK, create taskLogOut-task, wait for start/stop/exit/info/... command  
static void taskLogIn( void *pvParameters )
{
	const char LOG_TASK_ESTATE[] = {'*', 'R', 'B', 'S', 'D'};	// see task.h: enum eTaskState
	char cmd;							// single char received command or one byte ACK/NACK to send; 
	size_t count;						// size of bytes/TaskInfos received/to be send from/to UDP
	size_t cntArrays;					// count of TaskInfoFields
	struct info_t infoBlock;			// task info will be assembled here
	xTaskStatusType *pTaskStateBase, *pTaskStateCur;	// ptr to array reported by uxTaskGetSystemState()

	(void) pvParameters;				// just to prevent compiler warnings.

	LOG_MASK_TASK_LOGI();				// disable or enable trace

#ifndef _WIN_PIPE_
	xSocket = FreeRTOS_socket( FREERTOS_AF_INET, FREERTOS_SOCK_DGRAM, FREERTOS_IPPROTO_UDP );
	if( xSocket != FREERTOS_INVALID_SOCKET)
	{
		memset( (void*) &xClient, 0, sizeof( xClient ) );
		xClient.sin_port = FreeRTOS_htons( TRACE_UDP_PORT );

		if( FreeRTOS_bind( xSocket, &xClient, sizeof( xClient ) ) != 0 ) { vTaskEndScheduler(); }	// exit freeRTOS, if bind() fails
	}
	
	for( ;; )							// wait until connect from remote PC
	{
		count = FreeRTOS_recvfrom( xSocket, ((void*)&cmd), sizeof( cmd ), 0, &xClient, &xClientAddrLen );

		if( (count != 0) && (cmd == LOG_CONNECT_REQUEST) ) { break; }
	}
#endif // #ifndef _WIN_PIPE_
 
	cmd = LOG_CONNECT_ACK;
	udpWrite( &cmd, sizeof( cmd ) );

// start task, that handles the trace ring buffer, remote IP is parameter 
	xTaskCreate( taskLogOut, (signed char *)LOG_TASK_NAME_OUT, configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &thLogOut );

	for( ;; )							// wait for remote commands
	{
#ifndef _WIN_PIPE_
		count = FreeRTOS_recvfrom( xSocket, ((void*)&cmd), sizeof( cmd ), 0, &xClient, &xClientAddrLen );
#else
		vTaskDelay( 50 );
		count = pipeRead( &cmd );
#endif // #ifndef _WIN_PIPE_

		if( count != 0 )
		{
			switch( cmd )
			{
				case LOG_START_TRACE: { logHideTrace = LOG_TRACE_ON;  break; }
				case LOG_STOPP_TRACE: { logHideTrace = LOG_TRACE_OFF; break; }
				case LOG_EXIT_TRACE:  { vTaskEndScheduler(); }	// exit
				case LOG_TASK_INFO:
				{

					cntArrays = uxTaskGetNumberOfTasks();	// // how many tasks?; allocate memory space for report
					pTaskStateCur = pTaskStateBase = pvPortMalloc( cntArrays * sizeof( xTaskStatusType ) );

					if( pTaskStateBase != NULL )
					{

						cntArrays = uxTaskGetSystemState(pTaskStateBase, (UBaseType_t) cntArrays, NULL);
						infoBlock.cmd = LOG_CMD_INFO;

						for( count = 0; count < cntArrays; count++ )
						{
							infoBlock.id = (uint8_t)pTaskStateCur->xTaskNumber;
							infoBlock.state = LOG_TASK_ESTATE[pTaskStateCur->eCurrentState];
							infoBlock.prio = (uint8_t)pTaskStateCur->uxCurrentPriority;
							infoBlock.baseprio = (uint8_t)pTaskStateCur->uxBasePriority;
//							infoBlock.runtime = pTaskStateCur->ulRunTimeCounter;
							infoBlock.stack = pTaskStateCur->usStackHighWaterMark;
							strncpy( (char*)&infoBlock.name, (char*)pTaskStateCur->pcTaskName, configMAX_TASK_NAME_LEN );
							pTaskStateCur++;
							udpWrite( &infoBlock, sizeof( infoBlock ) );
						}
						vPortFree( pTaskStateBase );	// free dynamic memory
					}
					break;	// case LOG_TASK_INFO:
				}
				default:	// unknown command
				{
					cmd = LOG_CMD_ERROR;
					udpWrite( &cmd, sizeof( cmd ) );
				}
			}
		}
	}
}

void logStartTasks( void ) { xTaskCreate( taskLogIn,  (signed char *)LOG_TASK_NAME_IN,  configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES-1, &thLogIn ); }


// help functions, called by macros defined in logTraceMacros.h
// handle all macros w/o parameter here
void logPutPlain( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PLAIN;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PLAIN );
}


// 140407 fahr: LOG_CALL_TASK_SWIT moved to new logPutTaskSwitch()
// handle all macros with one parameter here (w/o LOG_CALL_TASK_SWIT)
void logPutParaTask( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PARAT;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PARAT );
}

void logPutPara( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PARA;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PARA );
}

void logPutPara2( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PARA2;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PARA2 );
}

void logPutPara3( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( logTraceSet.id < LOG_FILTER_MASK_BITS ) { if( (1UL << logTraceSet.id) & logFilterMask ) { return; } }	// no trace, if task filter active!

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PARA3;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PARA3 );
}

// handle LOG_CALL_TASK_SWIT here, Para is switches from
void logPutTaskSwitch( void )
{
	logGetCurrentTaskId();				// current task
	if( logHideTrace ) { return; }		// no trace yet!
	if( (logTraceSet.id < LOG_FILTER_MASK_BITS) && (logTraceSet.para < LOG_FILTER_MASK_BITS) )
	{
		if( ((1UL << logTraceSet.id) & logFilterMask ) && ((1UL << logTraceSet.para) & logFilterMask )) { return; } 	// no trace, if both task filters active!
	}

	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_PARAT;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), LOG_SET_LEN_PARA );
}


// all taskCreate-Calls processed here
void logPutTaskCreate( void )
{
	logGetCurrentTaskId();				// what task is running yet? Check against network-task and filter 
	LOG_MASK_TASK_UDP();
	LOG_MASK_TASK_TMR();
	LOG_MASK_TASK_MAC();
	LOG_MASK_TASK_IDLE();
	LOG_MASK_TASK_COM();

//140319 show taskCreate "APPL"
	LOG_MASK_TASK_APPL() LOG_MASK_TCREATE();	// don't split this line
	logTraceSet.time = xTaskGetTickCount();
	logTraceSet.cmd = LOG_CMD_CREAT;
	logErrorFlag |= logBufferPlace( ((char *)&logTraceSet), sizeof( logTraceSet ) );
}


char getBufferState(void)
{
	if(logRB.used > logRB.free)
	{
		return 2;
	}
	else if(logRB.used > 0)
	{
		return 1;
	}
	return 0;
}

void taskApplication( void *);
void taskSimCom(void*);

void startApplTask( void )
{
#ifdef ENABLE_SIMCOM
	xTaskCreate( taskSimCom, (const signed char* const)"SIMCOM", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
#endif
	xTaskCreate( taskApplication, (signed char*)LOG_TASK_NAME_APPL, configMINIMAL_STACK_SIZE, NULL, tskIDLE_PRIORITY + 3, NULL );
}
