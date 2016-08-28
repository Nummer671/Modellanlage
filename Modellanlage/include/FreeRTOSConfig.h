/*
FreeRTOS V8.2.3 - Copyright (C) 2015 Real Time Engineers Ltd.
All rights reserved

VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

This file is part of the FreeRTOS distribution.

FreeRTOS is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License (version 2) as published by the
Free Software Foundation >>>> AND MODIFIED BY <<<< the FreeRTOS exception.

***************************************************************************
>>!   NOTE: The modification to the GPL is included to allow you to     !<<
>>!   distribute a combined work that includes FreeRTOS without being   !<<
>>!   obliged to provide the source code for proprietary components     !<<
>>!   outside of the FreeRTOS kernel.                                   !<<
***************************************************************************

FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  Full license text is available on the following
link: http://www.freertos.org/a00114.html

***************************************************************************
*                                                                       *
*    FreeRTOS provides completely free yet professionally developed,    *
*    robust, strictly quality controlled, supported, and cross          *
*    platform software that is more than just the market leader, it     *
*    is the industry's de facto standard.                               *
*                                                                       *
*    Help yourself get started quickly while simultaneously helping     *
*    to support the FreeRTOS project by purchasing a FreeRTOS           *
*    tutorial book, reference manual, or both:                          *
*    http://www.FreeRTOS.org/Documentation                              *
*                                                                       *
***************************************************************************

http://www.FreeRTOS.org/FAQHelp.html - Having a problem?  Start by reading
the FAQ page "My application does not run, what could be wrong?".  Have you
defined configASSERT()?

http://www.FreeRTOS.org/support - In return for receiving this top quality
embedded software for free we request you assist our global community by
participating in the support forum.

http://www.FreeRTOS.org/training - Investing in training allows your team to
be as productive as possible as early as possible.  Now you can receive
FreeRTOS training directly from Richard Barry, CEO of Real Time Engineers
Ltd, and the world's leading authority on the world's leading RTOS.

http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
including FreeRTOS+Trace - an indispensable productivity tool, a DOS
compatible FAT file system, and our tiny thread aware UDP/IP stack.

http://www.FreeRTOS.org/labs - Where new FreeRTOS products go to incubate.
Come and try FreeRTOS+TCP, our new open source TCP/IP stack for FreeRTOS.

http://www.OpenRTOS.com - Real Time Engineers ltd. license FreeRTOS to High
Integrity Systems ltd. to sell under the OpenRTOS brand.  Low cost OpenRTOS
licenses offer ticketed support, indemnification and commercial middleware.

http://www.SafeRTOS.com - High Integrity Systems also provide a safety
engineered and independently SIL3 certified version for use in safety and
mission critical applications that require provable dependability.

1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H


/*
Two interrupt examples are provided -

+ Method 1 does everything in C code.
+ Method 2 uses an assembly file wrapper.

Code size:
Method 1 uses assembly macros to save and restore the task context, whereas
method 2 uses functions. This means method 1 will be faster, but method 2 will
use less code space.

Simplicity:
Method 1 is very simplistic, whereas method 2 is more elaborate. This
elaboration results in the code space saving, but also requires a slightly more
complex procedure to define interrupt service routines.

Interrupt efficiency:
Method 1 uses the compiler generated function prologue and epilogue code to save
and restore the necessary registers within an interrupt service routine (other
than the RTOS tick ISR). Should a context switch be required from within the ISR
the entire processor context is saved. This can result in some registers being saved
twice - once by the compiler generated code, and then again by the FreeRTOS code.
Method 2 saves and restores all the processor registers within each interrupt service
routine, whether or not a context switch actually occurs. This means no registers
ever get saved twice, but imposes an overhead on the occasions that no context switch
occurs.
*/

#define configINTERRUPT_EXAMPLE_METHOD 1

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION			1
#define configCPU_CLOCK_HZ				( ( unsigned long ) 50000000UL	) // DO NOT TOUCH!
#define configTICK_RATE_HZ				(( portTickType )1000) // must be lower than 5000 /* In this non-real time simulated environment the tick frequency has to be at least a multiple of the Win32 tick frequency, and therefore very slow. */
#define configMAX_PRIORITIES			( 7 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 60 ) /* In this simulated case, the stack only has to hold one small structure as the real stack is part of the Win32 thread. */
#define configTOTAL_HEAP_SIZE			( ( size_t ) ( 32U * 1024U ) )
#define configMAX_TASK_NAME_LEN			( 7 )	// higher value increase RAM usage, reduce trace events til buffer overflow
#define configQUEUE_REGISTRY_SIZE		0
#define configIDLE_SHOULD_YIELD			1
#define configMAXTasks					10
#define configUSE_MUTEXES				1
#define configUSE_RECURSIVE_MUTEXES		1
#define configUSE_COUNTING_SEMAPHORES	1
#define configUSE_APPLICATION_TASK_TAG	0
#define configUSE_ALTERNATIVE_API		0
#define configUSE_TRACE_FACILITY		1
#define configUSE_16_BIT_TICKS			0


/* Hook function related definitions. */
#define configUSE_TICK_HOOK				1
#define configUSE_IDLE_HOOK				1	
#define configUSE_MALLOC_FAILED_HOOK	1
#define configCHECK_FOR_STACK_OVERFLOW	0	// not applicable to the Win32 port

/* Software timer related definitions. */
#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( configMINIMAL_STACK_SIZE * 2 )

/* Software timer related definitions. */
#define configUSE_TIMERS				1
#define configTIMER_TASK_PRIORITY		( configMAX_PRIORITIES - 1 )
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_STACK_DEPTH	( configMINIMAL_STACK_SIZE * 2 )



/* Run time stats gathering definitions. */
// unsigned long ulGetRunTimeCounterValue( void );
// fahr
//void vConfigureTimerForRunTimeStats( void );
//#define configGENERATE_RUN_TIME_STATS	1
#define configGENERATE_RUN_TIME_STATS	0
//#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vConfigureTimerForRunTimeStats()
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
//#define portGET_RUN_TIME_COUNTER_VALUE() ulGetRunTimeCounterValue()


/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		0
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet			1
#define INCLUDE_uxTaskPriorityGet			1
#define INCLUDE_vTaskDelete					1
#define INCLUDE_vTaskCleanUpResources		0
#define INCLUDE_vTaskSuspend				1
#define INCLUDE_vTaskDelayUntil				1
#define INCLUDE_vTaskDelay					1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetSchedulerState		1
#define INCLUDE_xTimerGetTimerTaskHandle	0
#define INCLUDE_xTaskGetIdleTaskHandle		0
#define INCLUDE_xQueueGetMutexHolder		1

/* This demo makes use of one or more example stats formatting functions.  These
format the raw data provided by the uxTaskGetSystemState() function in to human
readable ASCII form.  See the notes in the implementation of vTaskList() within
FreeRTOS/Source/tasks.c for limitations. */
#define configUSE_STATS_FORMATTING_FUNCTIONS	1


#define USE_FREERTOS_CONTROL
#define SemInfo
#define QInfo
#define TInfo
#define MSGQ


#define REMOTESTOP




/* Assert call defined for debug builds. */
#ifdef _DEBUG
extern void vAssertCalled(void);
#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled()
#endif /* _DEBUG */

#ifndef _WIN_PIPE_
/* Application specific definitions follow. **********************************/

/* If configINCLUDE_DEMO_DEBUG_STATS is set to one, then a few basic IP trace
macros are defined to gather some UDP stack statistics that can then be viewed
through the CLI interface. */
// fahr #define configINCLUDE_DEMO_DEBUG_STATS 1
#define configINCLUDE_DEMO_DEBUG_STATS 0

/* The size of the global output buffer that is available for use when there
are multiple command interpreters running at once (for example, one on a UART
and one on TCP/IP).  This is done to prevent an output buffer being defined by
each implementation - which would waste RAM.  In this case, there is only one
command interpreter running, and it has its own local output buffer, so the
global buffer is just set to be one byte long as it is not used and should not
take up unnecessary RAM. */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE 1

/* Only used when running in the FreeRTOS Windows simulator. Defines the priority of the task used to simulate Ethernet interrupts. */
#define configMAC_ISR_SIMULATOR_PRIORITY	( configMAX_PRIORITIES - 1 )

/* This demo creates a virtual network connection by accessing the raw Ethernet or WiFi data to and from a real network connection.
Many computers have more than one real network port, and configNETWORK_INTERFACE_TO_USE is used to tell the demo which real port should be used to create the virtual port.
The ports available are displayed on the console when the application is executed. */
#define configNETWORK_INTERFACE_TO_USE 1L
//#define configNETWORK_INTERFACE_TO_USE 2L

// Default MAC address configuration.
// fahr #define configMAC_ADDR0	0x00
#define configMAC_ADDR0	0x40			// force local flag 
#define configMAC_ADDR1	0x11
#define configMAC_ADDR2	0x22
#define configMAC_ADDR3	0x33
#define configMAC_ADDR4	0x44
#define configMAC_ADDR5	0x55

#define configIP_ADDR0	192				// Default IP address
#define configIP_ADDR1	168
#define configIP_ADDR2	230
#define configIP_ADDR3	139

#define configNET_MASK0	255				// Default netmask configuration
#define configNET_MASK1	255
#define configNET_MASK2	255
#define configNET_MASK3	0

#define configGATEWAY_ADDR0	192			// Default gateway IP address configuration
#define configGATEWAY_ADDR1	168
#define configGATEWAY_ADDR2	230
#define configGATEWAY_ADDR3	1

#define configDNS_SERVER_ADDR0 	208		// Default DNS server configuration. OpenDNS addresses are 208.67.222.222 and 208.67.220.220.
#define configDNS_SERVER_ADDR1 	67
#define configDNS_SERVER_ADDR2 	222
#define configDNS_SERVER_ADDR3 	222
#endif // #ifndef _WIN_PIPE_




// this function starts the user application task
#define LOG_TASK_NAME_APPL "APPL"		/* name of the first user task */
void startApplTask(void);				// see application.c



// Set the following line (to 1) to activate Trace with FreeRTOSControl
#if 0
#define ENABLE_TRACE
#endif

// Set the following line (to 1) to activate the Communication to the "Modellbahn-Simulation"
#if 1
#define ENABLE_SIMCOM
#endif

// Do not Edit
#ifdef ENABLE_SIMCOM
#undef configUSE_TIME_SLICING
#define configUSE_TIME_SLICING 0
#endif

// for freeRTOS-trace, next line must be included
#include "trace/logTraceMacros.h"

#endif /* FREERTOS_CONFIG_H */
