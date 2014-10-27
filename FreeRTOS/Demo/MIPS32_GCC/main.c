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

/*
 * This is a very simple demo that demonstrates task and queue usages only in
 * a simple and minimal FreeRTOS configuration.  Details of other FreeRTOS
 * features (API functions, tracing features, diagnostic hook functions, memory
 * management, etc.) can be found on the FreeRTOS web site
 * (http://www.FreeRTOS.org) and in the FreeRTOS book.
 *
 * Details of this demo (what it does, how it should behave, etc.) can be found
 * in the accompanying PDF application note.
 *
*/

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Standard include. */
#include <stdio.h>

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define	mainQUEUE_SEND_TASK_PRIORITY		( tskIDLE_PRIORITY + 1 )

/* The rate at which data is sent to the queue, specified in milliseconds. */
#define mainQUEUE_SEND_FREQUENCY_MS			( 10 )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 1 )

/*-----------------------------------------------------------*/

/*
 * The tasks as described in the accompanying PDF application note.
 */
static void prvQueueReceiveTask( void *pvParameters );
static void prvQueueSendTask( void *pvParameters );

/*-----------------------------------------------------------*/

/* The queue used by both tasks. */
static xQueueHandle xQueue = NULL;

/* One array position is used for each task created by this demo.  The
variables in this array are set and cleared by the trace macros within
FreeRTOS, and displayed on the logic analyzer window within the Keil IDE -
the result of which being that the logic analyzer shows which task is
running when. */
unsigned long ulTaskNumber[ 3 ];

/*-----------------------------------------------------------*/

int main(void)
{
	/* Create the queue. */
	xQueue = xQueueCreate( mainQUEUE_LENGTH, sizeof( unsigned long ) );

	if( xQueue != NULL )
	{
		/* Start the two tasks as described in the accompanying application
		note. */
		xTaskCreate( prvQueueReceiveTask, ( signed char * ) "Rx", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_RECEIVE_TASK_PRIORITY, NULL );
		xTaskCreate( prvQueueSendTask, ( signed char * ) "TX", configMINIMAL_STACK_SIZE, NULL, mainQUEUE_SEND_TASK_PRIORITY, NULL );

		/* Start the tasks running. */
		vTaskStartScheduler();
	}

	/* If all is well we will never reach here as the scheduler will now be
	running.  If we do reach here then it is likely that there was insufficient
	heap available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask( void *pvParameters )
{
portTickType xNextWakeTime;
const unsigned long ulValueToSend = 100UL;
#if defined(__mips_dsp) || defined(__mips_hard_float)
unsigned int reg;
#endif

	/* Initialise xNextWakeTime - this only needs to be done once. */
	xNextWakeTime = xTaskGetTickCount();

	for( ;; )
	{
#ifdef __mips_dsp
		__asm__ __volatile__ (
				"li		%0, 0x12345678\n\t"
				"mthi   %0, $ac2\n\t"
				"li		%0, 0x9abcdef0\n\t"
				"mtlo   %0, $ac2\n\t"
				: "=r"(reg)
				);
#endif

#ifdef __mips_hard_float
		__asm__ __volatile__ (
				"li		%0, 0x12345678\n\t"
				"mtc1   %0, $f0\n\t"
				"li		%0, 0x9abcdef0\n\t"
				"mtc1   %0, $f2\n\t"
				"li		%0, 0x12345678\n\t"
				"mtc1   %0, $f4\n\t"
				"li		%0, 0x9abcdef0\n\t"
				"mtc1   %0, $f6\n\t"
				"li		%0, 0x12345678\n\t"
				"mtc1   %0, $f8\n\t"
				"li		%0, 0x9abcdef0\n\t"
				"mtc1   %0, $f10\n\t"
				: "=r"(reg)
				);
#endif

		/* Place this task in the blocked state until it is time to run again.
		The block time is specified in ticks, the constant used converts ticks
		to ms.  While in the Blocked state this task will not consume any CPU
		time. */
		vTaskDelayUntil( &xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS );

		/* Send to the queue - causing the queue receive task to unblock and
		print out a message.  0 is used as the block time so the sending
		operation will not block - it shouldn't need to block as the queue
		should always be empty at this point in the code. */
		xQueueSend( xQueue, &ulValueToSend, 0 );

		printf("%s message sent\n", __func__);

#ifdef __mips_hard_float
		unsigned int f0, f1, f2, f3, f4, f5;
		__asm__ __volatile__ (
				"mfc1   %0, $f0\n\t"
				"mfc1   %1, $f2\n\t"
				"mfc1   %2, $f4\n\t"
				"mfc1   %3, $f6\n\t"
				"mfc1   %4, $f8\n\t"
				"mfc1   %5, $f10\n\t"
				: "=r"(f0), "=r"(f1), "=r"(f2), "=r"(f3), "=r"(f4), "=r"(f5)
				);
		printf("*S* f0: %p\tf1: %p\tf2: %p\tf3: %p\tf4: %p\tf5: %p\t\n",
				f0, f1, f2, f3, f4, f5);
		configASSERT( f0 == 0x12345678 &&
					  f1 == 0x9abcdef0 &&
					  f2 == 0x12345678 &&
					  f3 == 0x9abcdef0 &&
					  f4 == 0x12345678 &&
					  f5 == 0x9abcdef0);
#endif

#ifdef __mips_dsp
		unsigned int reghi = 0, reglo = 0;
		__asm__ __volatile__ (
				"mfhi   %0, $ac2\n\t"
				"mflo   %1, $ac2\n\t"
				: "=r"(reghi), "=r"(reglo)
				);

		configASSERT( ( reghi == 0x12345678 && reglo == 0x9abcdef0 ) );
#endif
	}
}
/*-----------------------------------------------------------*/

static unsigned long ulRcv;

static void prvQueueReceiveTask( void *pvParameters )
{
unsigned long ulReceivedValue;
#if defined(__mips_dsp) || defined(__mips_hard_float)
unsigned int reg;
#endif

	for( ;; )
	{

#ifdef __mips_dsp
		__asm__ __volatile__ (
				"li		%0, 0x87654321\n\t"
				"mthi   %0, $ac2\n\t"
				"li		%0, 0x0fedcba9\n\t"
				"mtlo   %0, $ac2\n\t"
				: "=r"(reg)
				);
#endif

#ifdef __mips_hard_float
		__asm__ __volatile__ (
				"li		%0, 0x87654321\n\t"
				"mtc1   %0, $f0\n\t"
				"li		%0, 0x0fedcba9\n\t"
				"mtc1   %0, $f2\n\t"
				"li		%0, 0x87654321\n\t"
				"mtc1   %0, $f4\n\t"
				"li		%0, 0x0fedcba9\n\t"
				"mtc1   %0, $f6\n\t"
				"li		%0, 0x87654321\n\t"
				"mtc1   %0, $f8\n\t"
				"li		%0, 0x0fedcba9\n\t"
				"mtc1   %0, $f10\n\t"
				: "=r"(reg)
				);
#endif

		/* Wait until something arrives in the queue - this task will block
		indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
		FreeRTOSConfig.h. */
		xQueueReceive( xQueue, &ulReceivedValue, portMAX_DELAY );

		printf("%s message received\n", __func__);

#ifdef __mips_hard_float
		unsigned int f0, f1, f2, f3, f4, f5;
		__asm__ __volatile__ (
				"mfc1   %0, $f0\n\t"
				"mfc1   %1, $f2\n\t"
				"mfc1   %2, $f4\n\t"
				"mfc1   %3, $f6\n\t"
				"mfc1   %4, $f8\n\t"
				"mfc1   %5, $f10\n\t"
				: "=r"(f0), "=r"(f1), "=r"(f2), "=r"(f3), "=r"(f4), "=r"(f5)
				);
		printf("*R* f0: %p\tf1: %p\tf2: %p\tf3: %p\tf4: %p\tf5: %p\t\n",
				f0, f1, f2, f3, f4, f5);
		configASSERT( f0 == 0x87654321 &&
					  f1 == 0x0fedcba9 &&
					  f2 == 0x87654321 &&
					  f3 == 0x0fedcba9 &&
					  f4 == 0x87654321 &&
					  f5 == 0x0fedcba9);
#endif

#ifdef __mips_dsp
		unsigned int reghi = 0, reglo = 0;
		__asm__ __volatile__ (
				"mfhi   %0, $ac2\n\t"
				"mflo   %1, $ac2\n\t"
				: "=r"(reghi), "=r"(reglo)
				);

		configASSERT( ( reghi == 0x87654321 && reglo == 0x0fedcba9) );
#endif
		/*  To get here something must have been received from the queue, but
		is it the expected value?  If it is, print out a pass message, if no,
		print out a fail message. */
		if( ulReceivedValue == 100UL )
		{
			ulRcv++;
		}
		else
		{
			ulRcv = 0;
		}
	}
}
/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFileName, unsigned long ulLine )
{
	printf("Guru meditation in %s:%d\n", pcFileName, ulLine);
	for (;;);
}
