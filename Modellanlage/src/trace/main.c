// Standard includes
#include <stdint.h>						// uintX_t 
#include <stdio.h>

// FreeRTOS includes
#include <FreeRTOS.h>
#include "task.h"
#include "trace\global_switch.h"
#ifndef _WIN_PIPE_
  #include "FreeRTOS_UDP_IP.h"
  #include "FreeRTOS_Sockets.h"
#endif // #ifndef _WIN_PIPE_

#ifdef _WINDOWS_
#ifndef _WIN_PIPE_
/* The default IP and MAC address used by the demo.  The address configuration defined here will be used if ipconfigUSE_DHCP is 0,
or if ipconfigUSE_DHCP is 1 but a DHCP server could not be contacted.  See the online documentation for more information. */
static const uint8_t ucIPAddress[ 4 ] = { configIP_ADDR0, configIP_ADDR1, configIP_ADDR2, configIP_ADDR3 };
static const uint8_t ucNetMask[ 4 ] = { configNET_MASK0, configNET_MASK1, configNET_MASK2, configNET_MASK3 };
static const uint8_t ucGatewayAddress[ 4 ] = { configGATEWAY_ADDR0, configGATEWAY_ADDR1, configGATEWAY_ADDR2, configGATEWAY_ADDR3 };
static const uint8_t ucDNSServerAddress[ 4 ] = { configDNS_SERVER_ADDR0, configDNS_SERVER_ADDR1, configDNS_SERVER_ADDR2, configDNS_SERVER_ADDR3 };

/* Default MAC address configuration.  The demo creates a virtual network connection that uses this MAC address by accessing the raw Ethernet data
to and from a real network connection on the host PC.  See the configNETWORK_INTERFACE_TO_USE definition for information on how to configure the real network connection to use. */
static const uint8_t ucMACAddress[ 6 ] = { configMAC_ADDR0, configMAC_ADDR1, configMAC_ADDR2, configMAC_ADDR3, configMAC_ADDR4, configMAC_ADDR5 };
#endif // #ifndef _WIN_PIPE_
#endif // _WINDOWS_


int main( void )
{
const uint32_t ulLongTime_ms = 250UL;
#if defined(_WIN_PIPE_) && defined(ENABLE_TRACE)
  int count;
  char cmd;
#endif // #ifdef _WIN_PIPE_

#ifndef _WINDOWS_
// initialize HW here
#endif // _WINDOWS_

#ifdef LOGTRACEMACROS_H
	logBufferInit();	// initialize log-buffer to report activity from here
#endif // LOGTRACEMACROS_H

#ifndef _WIN_PIPE_
/* Initialise the network interface. Tasks that use the network are created in the network event hook when the network is connected and ready for use.
The address values passed in here are used if ipconfigUSE_DHCP is set to 0, or if ipconfigUSE_DHCP is set to 1 but a DHCP server cannot be contacted. */
	FreeRTOS_IPInit( ucIPAddress, ucNetMask, ucGatewayAddress, ucDNSServerAddress, ucMACAddress );
#else
#if defined(ENABLE_TRACE)
// create Windows Pipes
	if( pipeInit() == RET_PIPE_FAIL ) { return RET_PIPE_FAIL; }
	for( ; ; )
	{
		count = pipeRead( &cmd );
		if( (count != 0) && (cmd == LOG_CONNECT_REQUEST) ) { break; }
	}
#endif // #if defined(ENABLE_TRACE)
#endif // #ifndef _WIN_PIPE_

#if defined(LOGTRACEMACROS_H) && defined(ENABLE_TRACE)
	logStartTasks();					// start trace log tasks
#else
	startApplTask();
#endif // LOGTRACEMACROS_H

	vTaskStartScheduler();				// Start the RTOS scheduler

/* If all is well, the scheduler will now be running, and the following line will never be reached.
If the following line does execute, then there was insufficient FreeRTOS heap memory available for the idle and/or timer tasks to be created.
See the memory management section on the FreeRTOS web site for more details (this is standard text that is not not really applicable to the Win32 simulator port). */
	for( ;; )
	{
		Sleep( ulLongTime_ms );
	}
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
const unsigned long ulMSToSleep = 5;

	/* This function is called on each cycle of the idle task if
	configUSE_IDLE_HOOK is set to 1 in FreeRTOSConfig.h.  Sleep to reduce CPU
	load. */
	Sleep( ulMSToSleep );
}
/*-----------------------------------------------------------*/

void vAssertCalled( void )
{
const unsigned long ulLongSleep = 1000UL;
volatile uint32_t ulBlockVariable = 0UL;

	/* Setting ulBlockVariable to a non-zero value in the debugger will allow
	this function to be exited. */
	taskDISABLE_INTERRUPTS();
	{
		printf("\n\nSystemfailure\n\n");
		while( ulBlockVariable == 0UL )
		{
			Sleep( ulLongSleep );
		}
	}
	taskENABLE_INTERRUPTS();
}
/*-----------------------------------------------------------*/


#ifndef _WIN_PIPE_
void vApplicationIPNetworkEventHook( eIPCallbackEvent_t eNetworkEvent )	// Called by FreeRTOS+UDP when the network connects
{
uint32_t ulIPAddress, ulNetMask, ulGatewayAddress, ulDNSServerAddress;
int8_t cBuffer[ 16 ];					// 123.456.789.012 ==> 16 chars
static portBASE_TYPE xTasksAlreadyCreated = pdFALSE;

	if( eNetworkEvent == eNetworkUp )
	{
		/* Create the tasks that use the IP stack if they have not already been created. */
		if( xTasksAlreadyCreated == pdFALSE )
		{
			#ifdef LOGTRACEMACROS_H
				logStartTasks();		// start trace log tasks
			#else
				startApplTask();
			#endif // LOGTRACEMACROS_H

			xTasksAlreadyCreated = pdTRUE;
		}
		FreeRTOS_GetAddressConfiguration( &ulIPAddress, &ulNetMask, &ulGatewayAddress, &ulDNSServerAddress );
		FreeRTOS_inet_ntoa( ulIPAddress, cBuffer );
		printf( "\r\nIP Address: %s", cBuffer );
		FreeRTOS_inet_ntoa( ulNetMask, cBuffer );
		printf( "\r\nSubnet Mask: %s\r\n\r\n", cBuffer );
	}
}
#endif // #ifndef _WIN_PIPE_


void vApplicationMallocFailedHook( void )
{
	/* vApplicationMallocFailedHook() will only be called if
	configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
	function that will get called if a call to pvPortMalloc() fails.
	pvPortMalloc() is called internally by the kernel whenever a task, queue,
	timer or semaphore is created.  It is also called by various parts of the
	demo application.  If heap_1.c, heap_2.c or heap_4.c are used, then the 
	size of the heap available to pvPortMalloc() is defined by 
	configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize() 
	API function can be used to query the size of free heap space that remains 
	(although it does not provide information on how the remaining heap might 
	be fragmented). */
	taskDISABLE_INTERRUPTS();
	for( ;; );
}


extern xTaskHandle thLogOut;

void vApplicationTickHook( void )
{
#ifdef ENABLE_TRACE
	static unsigned portBASE_TYPE TickCount = 0;
	TickCount++;
	if(getBufferState() == 2 || (getBufferState() == 1 && TickCount >= 100))
	{
		vTaskResume(thLogOut);
		TickCount = 0;
	}
#endif
}

