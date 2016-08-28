// 140407 fahr: LOG_CALL_TASK_SWIT moved to new handler logPutTaskSwitch()

#ifndef LOGTRACEMACROS_H
#define LOGTRACEMACROS_H

#include <stdint.h>	// uintX_t
#include "global_switch.h"

#pragma pack(push,1)					// CCS: no space allowed between brackets
struct clk_t							// report clock ticks per second; LITTLE ENDIAN!							
{
	uint8_t	cmd;						// LOG_CMD_CLOCK
	unsigned short clkpersec;			// clock ticks per second (configTICK_RATE_HZ)
};

struct msg_t							// report clock ticks per second; LITTLE ENDIAN!							
{
	uint8_t	cmd;						// LOG_CMD_MSG
	uint8_t count;						// bytes message follows
};

struct info_t							// report task info; LITTLE ENDIAN!
{
	uint8_t cmd;						// LOG_CMD_INFO
	uint8_t id;							// task number
	uint8_t state;						// Ready, Suspend, Delayed, Blocked (from enum eTaskState)
	uint8_t prio;						// current priority
	uint8_t baseprio;					// mutex, before inherit
//	uint32_t runtime;					// unit ticks?
	uint16_t stack;						// stack high water mark
	char	name[configMAX_TASK_NAME_LEN];	// task name
};

struct log_t							// log trace; LITTLE ENDIAN!
{
	uint8_t	cmd;						// LOG_CMD_PLAIN/PARA(CREATE			PLAIN	PARA	PARA2	PARA3	PARA4	PARAT	CREATE
	uint8_t id;							// current task							valid	valid	valid	valid	valid	valid	valid
	uint8_t call;						// FreeRTOS-call (prio@create?)			valid	valid	valid	valid	valid	valid	valid
	uint32_t time;						// current tick stamp					valid	valid	valid	valid	valid	valid	valid
	uint8_t	para;						// div. usage							unused	valid	valid	valid	valid	valid	valid
	uint8_t	para2;						// div. usage							unused	unused	valid	valid	valid	unused	unused
	uint8_t	para3;						// div. usage							unused	unused	unused	valid	valid	unused	unused
	uint8_t	para4;						// div. usage							unused	unused	unused	unused	valid	unused	unused
	char	name[configMAX_TASK_NAME_LEN];	// task name from taskCreate()		unused	unused	unused	unused	unused	unused	valid
};
#pragma pack(pop)
extern struct log_t logTraceSet;		// build trace data set here
#define LOG_SET_LEN_PLAIN ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) )
#define LOG_SET_LEN_PARAT  ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) + sizeof( logTraceSet.para ) )
#define LOG_SET_LEN_PARA  ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) + sizeof( logTraceSet.para ) )
#define LOG_SET_LEN_PARA2  ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) + sizeof( logTraceSet.para ) + sizeof( logTraceSet.para2 ) )
#define LOG_SET_LEN_PARA3  ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) + sizeof( logTraceSet.para ) + sizeof( logTraceSet.para2 ) + sizeof( logTraceSet.para3 ) )
#define LOG_SET_LEN_PARA4  ( sizeof( logTraceSet.cmd ) + sizeof( logTraceSet.id ) + sizeof( logTraceSet.call ) + sizeof( logTraceSet.time ) + sizeof( logTraceSet.para ) + sizeof( logTraceSet.para2 ) + sizeof( logTraceSet.para3 ) + sizeof( logTraceSet.para4 ) )

// cmd-fields:
#define LOG_CMD_NONE	0x00			// empty/unknown
#define LOG_CMD_MSG		0x02			// ASCII-text follows til stopp
#define LOG_CMD_INFO	0x05			// task info block
#define LOG_CMD_CLOCK	0x06			// ushort configTICK_RATE_HZ follows
#define LOG_CMD_CREAT	0x09			// trace log set from task create
#define LOG_CMD_PLAIN	0x10			// trace log set w/o parameter
#define LOG_CMD_PARAT	0x11			// start log set with one parameter for Tasks
#define LOG_CMD_PARA	0x12			// start log set with one parameter
#define LOG_CMD_PARA2	0x13			// start log set with tow parameter
#define LOG_CMD_PARA3	0x14			// start log set with three parameter
#define LOG_CMD_PARA4	0x15			// start log set with four parameter
#define LOG_CMD_ERRBUFF	0xF8			// buffer overflow 
// 0x04 QueueRegistry, not implemented yet

// id-field: task number begins at 1 or this
#define LOG_TASKID_UNKNOWN	0x00		// unknown task id or scheduler not runnung

// syscall-field
#define LOG_CALL_TASK_CREAT			0			// Task Create
#define LOG_CALL_TASK_DEL			1			// Task Delete
#define LOG_CALL_TASK_SUSP			2			// Task Suspend
#define LOG_CALL_TASK_RESM			3			// Task Resume
#define LOG_CALL_TASK_PRIO 			5			// Task set priority
#define LOG_CALL_TASK_DLY 			6			// Task delay
#define LOG_CALL_TASK_DLY_UNTIL		7			// Task delay until
#define LOG_CALL_TASK_SWIT			8			// Task switch
#define LOG_CALL_TASK_RDY			9			// Task ready
#define LOG_CALL_TASK_YIELD			10			// Task yield

#define LOG_CALL_SEM_CREAT 			20			// semaphore create
#define LOG_CALL_SEM_DEL			21
#define LOG_CALL_SEM_TAKE			22
#define LOG_CALL_SEM_TAKE_BLK		23
#define LOG_CALL_SEM_GIVE			24

#define LOG_CALL_CNT_SEM_CREAT		30			// counting semaphore create
#define LOG_CALL_CNT_SEM_DEL		31
#define LOG_CALL_CNT_SEM_TAKE		32
#define LOG_CALL_CNT_SEM_TAKE_BLK	33
#define LOG_CALL_CNT_SEM_GIVE		34

#define LOG_CALL_MTX_CREAT			40			// mutex create
#define LOG_CALL_MTX_DEL			41
#define LOG_CALL_MTX_TAKE			42
#define LOG_CALL_MTX_TAKE_BLK		43
#define LOG_CALL_MTX_GIVE			44


#define LOG_CALL_MSG_CREAT			50			// msgq create
#define LOG_CALL_MSG_DEL			51			
#define LOG_CALL_MSG_RCV			52			// message receive
#define LOG_CALL_MSG_RCV_BLK		53			// message receive blocked
#define LOG_CALL_MSG_SND			54			// message send
#define LOG_CALL_MSG_SND_BLK		55			// message send blocked

// (remote/local) port and CLI 
#define TRACE_UDP_PORT 55001			// test
// #define TRACE_UDP_PORT 29005			// release
#define LOG_OPEN_PROMPT "\002logOpenTrace\003"	// DEBUG only
#define LOG_CONNECT_REQUEST 'o'			// < connect/open request
#define LOG_CONNECT_ACK '#'				// > connect-ACK response
#define LOG_START_TRACE 'c'				// < start/continue trace request
#define LOG_STOPP_TRACE	's'				// < stopp/halt trace request
#define LOG_EXIT_TRACE	'e'				// < exit/terminate request
#define LOG_TASK_INFO   't'				// < taskInfo request
#define LOG_CMD_ERROR	'?'				// > unknown command received / NAK

//#############################################################################
// this part from here not for Qt!

// names of task that should be filtered (if defined here)
#define LOG_TASK_NAME_UDP "UDP/IP"		// freeRTOS+-UDP task
#define LOG_TASK_NAME_MAC "MAC_IS"		// Windows: simulated network interrupts task
#define LOG_TASK_NAME_OUT "LOGO"		// task write ringbuffer trace to UDP
#define LOG_TASK_NAME_IN  "LOGI"		// command interface to trace PC
#define LOG_TASK_NAME_TMR "Tmr Sv"		// Timer Service (used by network)
#define LOG_TASK_NAME_IDLE "IDLE"		// IDLE-Task
#define LOG_TASK_NAME_COM "SIMCOM"		// SIMCOM-
#define LOG_FILTER_CREATE_TASK			// if defined => hide taskCreate trace log (network and trace utils)
#define LOG_TASK_CREATE_APPL			// if defined => taskCreate of task "APPL" will be traced


#define traceTASK_CREATE( pxNewTCB )			logTraceSet.call = LOG_CALL_TASK_CREAT;	\
												logTraceSet.para = (uint8_t)pxNewTCB->uxTCBNumber; \
												strncpy( (char*)&logTraceSet.name, (char*)pxNewTCB->pcTaskName, configMAX_TASK_NAME_LEN ); \
												logPutTaskCreate();

#define traceTASK_DELETE( pxTCB )				logTraceSet.call = LOG_CALL_TASK_DEL; \
												logTraceSet.para = (uint8_t)pxTCB->uxTCBNumber;	\
												logPutParaTask();

#define traceTASK_SUSPEND( pxTCB )				logTraceSet.call = LOG_CALL_TASK_SUSP; \
												logTraceSet.para = (uint8_t)pxTCB->uxTCBNumber; \
												logPutParaTask();

#define traceTASK_RESUME( pxTCB )				logTraceSet.call = LOG_CALL_TASK_RESM; \
												logTraceSet.para = (uint8_t)pxTCB->uxTCBNumber; \
												logPutParaTask();

#define traceTASK_RESUME_FROM_ISR( pxTCB )		logTraceSet.call = LOG_CALL_TASK_RESM; \
												logTraceSet.para = (uint8_t)pxTCB->uxTCBNumber; \
												logPutParaTask();

#define traceTASK_PRIORITY_SET( pxTCB, nPrio )	logTraceSet.call = LOG_CALL_TASK_PRIO; \
												logTraceSet.para = (uint8_t)pxTCB->uxTCBNumber; \
												logPutParaTask();

#define traceTASK_DELAY()						logTraceSet.call = LOG_CALL_TASK_DLY; \
												logPutPlain();

#define traceTASK_DELAY_UNTIL()					logTraceSet.call = LOG_CALL_TASK_DLY_UNTIL; \
												logPutPlain();

#define traceTASK_SWITCHED_OUT()				logTraceSet.para = (uint8_t)pxCurrentTCB->uxTCBNumber;	// local report only, no trace

#define traceTASK_SWITCHED_IN()					if( logTraceSet.para != pxCurrentTCB->uxTCBNumber ) { \
													logTraceSet.call = LOG_CALL_TASK_SWIT; \
													logPutTaskSwitch(); }

#define traceMOVED_TASK_TO_READY_STATE(xTask)	logTraceSet.call = LOG_CALL_TASK_RDY; \
												logTraceSet.para = (uint8_t)xTask->uxTCBNumber; \
												logPutParaTask();

#define traceTASK_YIELD()						logTraceSet.call = LOG_CALL_TASK_YIELD; \
												logPutPlain();


#define traceCREATE_MUTEX(xQueue)				logTraceSet.call = LOG_CALL_MTX_CREAT; \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logTraceSet.para2 = (uint8_t)xQueue->uxLength; \
												logTraceSet.para3= (uint8_t)xQueue->uxMessagesWaiting; \
												logPutPara3();

#define traceCREATE_COUNTING_SEMAPHORE(xQueue)	logTraceSet.call = LOG_CALL_CNT_SEM_CREAT; \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logTraceSet.para2 = (uint8_t)xQueue->uxLength; \
												logTraceSet.para3= (uint8_t)xQueue->uxMessagesWaiting; \
												logPutPara3();

#define traceQUEUE_CREATE(xQueue)				if(xQueue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_SEM_CREAT; \
													logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
													logTraceSet.para2 = (uint8_t)xQueue->uxLength; \
													logTraceSet.para3= (uint8_t)xQueue->uxMessagesWaiting; \
													logPutPara3();} \
												else if (xQueue->ucQueueType == queueQUEUE_TYPE_BASE) { \
													logTraceSet.call = LOG_CALL_MSG_CREAT; \
													logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
													logTraceSet.para2 = (uint8_t)xQueue->uxLength; \
													logTraceSet.para3= (uint8_t)xQueue->uxMessagesWaiting; \
													logPutPara3();}


#define traceQUEUE_DELETE(xQueue)				if( xQueue->ucQueueType == queueQUEUE_TYPE_MUTEX ) { \
													logTraceSet.call = LOG_CALL_MTX_DEL; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_CNT_SEM_DEL; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_SEM_DEL; }\
												else { \
													logTraceSet.call = LOG_CALL_MSG_DEL; }\
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();

#define traceQUEUE_SEND( xQueue )				if( xQueue->ucQueueType == queueQUEUE_TYPE_MUTEX ) { \
													logTraceSet.call = LOG_CALL_MTX_GIVE; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_CNT_SEM_GIVE; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_SEM_GIVE; } \
												else { \
													logTraceSet.call = LOG_CALL_MSG_SND; }\
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();

#define traceQUEUE_RECEIVE( xQueue )			if( xQueue->ucQueueType == queueQUEUE_TYPE_MUTEX ) { \
													logTraceSet.call = LOG_CALL_MTX_TAKE; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_CNT_SEM_TAKE; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_SEM_TAKE; }\
												else { \
													logTraceSet.call = LOG_CALL_MSG_RCV; }\
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();

#define traceBLOCKING_ON_QUEUE_SEND(xQueue)		logTraceSet.call = LOG_CALL_MSG_SND_BLK; \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();

#define traceBLOCKING_ON_QUEUE_RECEIVE(xQueue)	if( xQueue->ucQueueType == queueQUEUE_TYPE_MUTEX ) { \
													logTraceSet.call = LOG_CALL_MTX_TAKE_BLK; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_COUNTING_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_CNT_SEM_TAKE_BLK; } \
												else if(xQueue->ucQueueType == queueQUEUE_TYPE_BINARY_SEMAPHORE) { \
													logTraceSet.call = LOG_CALL_SEM_TAKE_BLK; } \
												else { \
													logTraceSet.call = LOG_CALL_MSG_RCV_BLK; } \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();
/*
#define traceQUEUE_SEND_FAILED( xQueue )		logTraceSet.call = LOG_CALL_MSG_ERR; \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();	

#define traceQUEUE_RECEIVE_FAILED( xQueue )		logTraceSet.call = LOG_CALL_MSG_ERR; \
												logTraceSet.para = (uint8_t)xQueue->uxQueueNumber; \
												logPutPara();	*/




void logBufferInit( void );				// initialize logBuffer and vars, must be called first
void logSendMessage( const void *buf, size_t cnt );	// send a message over UDP
void logStartTasks( void );				// start log tasks, called vApplicationIPNetworkEventHook() 
void logPutTaskCreate( void );			// used by macro traceTASK_CREATE() only
void logPutPlain( void );				// used by different macros above, no parameter passed
void logPutParaTask( void );				// also, but one parameter passed to trace
void logPutPara( void );				// also, but one parameter passed to trace
void logPutPara2( void );				// also, but one parameter passed to trace
void logPutPara3( void );				// also, but one parameter passed to trace
void logPutTaskSwitch( void );			// also, but TaskSwitch with one parameter passed to trace
void logGetCurrentTaskId( void );		// add by fahr in task.c
char getBufferState(void);				// returns true if buffer is filled more than half
#endif // LOGTRACEMACROS_H