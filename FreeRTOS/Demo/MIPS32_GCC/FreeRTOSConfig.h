/*
	FreeRTOS V8.1.2 - Copyright (C) 2014 Real Time Engineers Ltd.
	All rights reserved

	VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

	***************************************************************************
	 *                                                                       *
	 *    FreeRTOS provides completely free yet professionally developed,    *
	 *    robust, strictly quality controlled, supported, and cross          *
	 *    platform software that has become a de facto standard.             *
	 *                                                                       *
	 *    Help yourself get started quickly and support the FreeRTOS         *
	 *    project by purchasing a FreeRTOS tutorial book, reference          *
	 *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
	 *                                                                       *
	 *    Thank you!                                                         *
	 *                                                                       *
	***************************************************************************

	This file is part of the FreeRTOS distribution.

	FreeRTOS is free software; you can redistribute it and/or modify it under
	the terms of the GNU General Public License (version 2) as published by the
	Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

	>>!   NOTE: The modification to the GPL is included to allow you to     !<<
	>>!   distribute a combined work that includes FreeRTOS without being   !<<
	>>!   obliged to provide the source code for proprietary components     !<<
	>>!   outside of the FreeRTOS kernel.                                   !<<

	FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
	WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
	FOR A PARTICULAR PURPOSE.  Full license text is available from the following
	link: http://www.freertos.org/a00114.html

	1 tab == 4 spaces!

	***************************************************************************
	 *                                                                       *
	 *    Having a problem?  Start by reading the FAQ "My application does   *
	 *    not run, what could be wrong?"                                     *
	 *                                                                       *
	 *    http://www.FreeRTOS.org/FAQHelp.html                               *
	 *                                                                       *
	***************************************************************************

	http://www.FreeRTOS.org - Documentation, books, training, latest versions,
	license and Real Time Engineers Ltd. contact details.

	http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
	including FreeRTOS+Trace - an indispensable productivity tool, a DOS
	compatible FAT file system, and our tiny thread aware UDP/IP stack.

	http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
	Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
	licenses offer ticketed support, indemnification and middleware.

	http://www.SafeRTOS.com - High Integrity Systems also provide a safety
	engineered and independently SIL3 certified version for use in safety and
	mission critical applications that require provable dependability.

	1 tab == 4 spaces!
*/

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

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

#define configUSE_PREEMPTION                        1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION     1
#define configCPU_CLOCK_HZ                          ( CONFIG_CPU_CLOCK_HZ )
#define configTIMER_RATE_HZ                         ( ( TickType_t ) configCPU_CLOCK_HZ / 2 )
#ifndef configTICK_RATE_HZ /* allow overide from makefile */
#define configTICK_RATE_HZ                          ( 1000 )
#endif
#define configUSE_16_BIT_TICKS                      0
#define configMAX_PRIORITIES                        ( CONFIG_MAX_PRIORITIES )
#define configMINIMAL_STACK_SIZE                    ( CONFIG_STACK_SIZE/2 )
#define configISR_STACK_SIZE                        ( CONFIG_STACK_SIZE/2 )
#define configTOTAL_HEAP_SIZE						( ( size_t ) CONFIG_HEAP_SIZE/2 )
#define configMAX_TASK_NAME_LEN                     ( 16 )

/* Hook functions */
#define configUSE_IDLE_HOOK                         0
#define configUSE_TICK_HOOK                         0

/* Co routines */
#define configUSE_CO_ROUTINES                       0

/* The interrupt priority of the RTOS kernel */
#define configKERNEL_INTERRUPT_PRIORITY             0x01

/* The maximum priority from which API functions can be called */
#define configMAX_API_CALL_INTERRUPT_PRIORITY       0x03

/* Prevent assert code from being used in assembly files */
#ifndef __ASSEMBLER__
	void vAssertCalled( const char *pcFileName, unsigned long ulLine );
	#define configASSERT( x )						\
		do {										\
			if( ( x ) == 0 )						\
				vAssertCalled( __FILE__, __LINE__ );\
		} while (0)
#endif

/* Optional functions */
#define INCLUDE_vTaskPrioritySet					0
#define INCLUDE_uxTaskPriorityGet					0
#define INCLUDE_vTaskDelayUntil						1
#define INCLUDE_vTaskDelay							1
#define INCLUDE_vTaskDelete							0
#define INCLUDE_vTaskSuspend						0

#if defined(ENABLE_TRACE)
#include "trace.h"
#endif

#endif	/* FREERTOSCONFIG_H */

