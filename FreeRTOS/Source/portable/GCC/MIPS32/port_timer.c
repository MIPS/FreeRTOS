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

/*-----------------------------------------------------------
 * Implementation of functions defined in port_timer.h
 *----------------------------------------------------------*/
#include <mips/cpu.h>
#include <stdint.h>

#include "FreeRTOSConfig.h"
#include "port_timer.h"
#include "int_handler.h"

static void inline prvSetNextTick ( void )
{
TickType_t xJiffies;
TickType_t xJResult;
TickType_t xJTest;
reg_t count;
reg_t compare;

	/* Convert from milliseconds to timer ticks */
	xJiffies = ms * ( ( TickType_t ) configTIMER_RATE_HZ / 1000UL ) * cyclesPerTick;

	/* Check that we have leeway to differentiate miss from wrap */
	xJTest = ( xJiffies & 0xc0000000 );
	xJResult = xJiffies << 2;

	/* Get the last trigger cycle count */
	compare = mips_getcompare();

	/* Make sure we've not missed the deadline */
	for( ;; )
	{
		compare += xJiffies;
		mips_setcompare( compare );
		count = mips_getcount();
		if( compare < count )
		{
			/* When xJTest != 0, there's not enough leeway to differentiate a
			tick miss from a wrap, so we assume we've wrapped. Similarly, when
			xJResult is less than count - compare, we have wrapped. */
			if( ( xJTest ) || ( ( count - compare ) > ( xJResult ) ) )
			{
				break;
			}
			else
			{
				/* We have missed a timer tick */
				continue;
			}
		}
		else
		{
			/* We've not missed a tick, nor have we wrapped */
			break;
		}
	}
}

/* Determine the number of cycles one instruction takes. We do this as it may
be the case that the Count tick rate may not be 1 tick per instruction. */
static inline TickType_t prvCyclesPerTick( void )
{
	uint32_t x, y;

	asm volatile( "mfc0 %0, $9, 0\n\t"
				"mfc0 %1, $9, 0\n\t" : "=r"( x ), "=r"( y ));
	y -= x;

	return (y ? y : 1);
}

/* Clear the timer interrupt */
void portClearTickTimerInterrupt( void )
{
	prvSetNextTick( portTICK_PERIOD_MS );
}

extern void vPortTickInterruptHandler( void );

/* This is defined as weak, as it can (theoretically) be redefined by the
application that we are being built into. */
__attribute__(( weak )) void vApplicationSetupTickTimerInterrupt( void )
{
	/* Install interrupt handler */
	pvPortInstallISR( TIMER_IRQ, vPortTickInterruptHandler );

	/* Set Compare to Count as a starting point */
	mips_setcompare( mips_getcount() );

	/* Set the interrupt to fire 1ms from now */
	prvSetNextTick( portTICK_PERIOD_MS );
}
