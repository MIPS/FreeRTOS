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

/* Critical section management, and task utilities.*/
#ifndef CRIT_TASK_H
#define	CRIT_TASK_H

#ifdef	__cplusplus
extern "C"
{
#endif

#define portIPL_SHIFT               10
#define portALL_IPL_BITS			( 0x3FUL << portIPL_SHIFT )

/* This clears the IPL bits, then sets them to
configMAX_SYSCALL_INTERRUPT_PRIORITY.  An extra check is performed if
configASSERT() is defined to ensure an assertion handler does not inadvertently
attempt to lower the IPL when the call to assert was triggered because the IPL
value was found to be above configMAX_SYSCALL_INTERRUPT_PRIORITY when an ISR
safe FreeRTOS API function was executed.  ISR safe FreeRTOS API functions are
those that end in FromISR.  FreeRTOS maintains a separate interrupt API to
ensure API function and interrupt entry is as fast and as simple as possible. */
#ifdef configASSERT
	#define portDISABLE_EIC_INTERRUPTS()										\
	{																			\
	uint32_t ulStatus;															\
																				\
		/* Mask interrupts at and below the kernel interrupt priority. */		\
		ulStatus = mips_getsr();												\
																				\
		/* Is the current IPL below configMAX_SYSCALL_INTERRUPT_PRIORITY? */	\
		if( ( ( ulStatus & portALL_IPL_BITS ) >> portIPL_SHIFT ) < configMAX_API_CALL_INTERRUPT_PRIORITY ) \
		{																		\
			ulStatus &= ~portALL_IPL_BITS;										\
			mips_setsr( ( ulStatus | ( configMAX_API_CALL_INTERRUPT_PRIORITY << portIPL_SHIFT ) ) ); \
		}																		\
	}
#else /* configASSERT */
	#define portDISABLE_EIC_INTERRUPTS()									\
	{																		\
	uint32_t ulStatus;														\
																			\
		/* Mask interrupts at and below the kernel interrupt priority. */	\
		ulStatus = mips_getsr();											\
		ulStatus &= ~portALL_IPL_BITS;										\
		mips_setsr( ( ulStatus | ( configMAX_API_CALL_INTERRUPT_PRIORITY << portIPL_SHIFT ) ) ); \
	}
#endif /* configASSERT */

#define portENABLE_EIC_INTERRUPTS()										\
{																		\
uint32_t ulStatus;														\
																		\
	/* Unmask all interrupts. */										\
	ulStatus = mips_getsr();											\
	ulStatus |= portALL_IPL_BITS;										\
	mips_setsr( ulStatus );												\
}

#ifndef _mips_intenable
/* MIPS32r2 atomic interrupt enable */
#define _mips_intenable() __extension__({ \
    unsigned int __v; \
    __asm__ __volatile__ ("ei %0; ehb" : "=d" (__v)); \
    __v; \
})
#endif

#define portDISABLE_INTERRUPTS( )	_mips_intdisable( );
#define portENABLE_INTERRUPTS( ) 	_mips_intenable( ); 


extern void vTaskEnterCritical( void );
extern void vTaskExitCritical( void );
#define portCRITICAL_NESTING_IN_TCB 1
#define portENTER_CRITICAL()        vTaskEnterCritical()
#define portEXIT_CRITICAL()         vTaskExitCritical()

#ifndef configUSE_PORT_OPTIMISED_TASK_SELECTION
	#define configUSE_PORT_OPTIMISED_TASK_SELECTION 1
#endif

#if configUSE_PORT_OPTIMISED_TASK_SELECTION == 1

	/* Check the configuration. */
	#if( configMAX_PRIORITIES > 32 )
		#error configUSE_PORT_OPTIMISED_TASK_SELECTION can only be set to 1 when configMAX_PRIORITIES is less than or equal to 32.  It is very rare that a system requires more than 10 to 15 difference priorities as tasks that share a priority will time slice.
	#endif

	/* Store/clear the ready priorities in a bit map. */
	#define portRECORD_READY_PRIORITY( uxPriority, uxReadyPriorities )      \
		( uxReadyPriorities ) |= ( 1UL << ( uxPriority ) )

	#define portRESET_READY_PRIORITY( uxPriority, uxReadyPriorities )       \
		( uxReadyPriorities ) &= ~( 1UL << ( uxPriority ) )

	/*-----------------------------------------------------------*/

	#define portGET_HIGHEST_PRIORITY( uxTopPriority, uxReadyPriorities )    \
		uxTopPriority = ( 31 - mips_clz( ( uxReadyPriorities ) ) )

#endif

/* Task utilities. */

/* To yield, we fire off a software interrupt, that will be handled by the
relevant interrupt handler. */
#define portYIELD()             mips_biscr( SR_SINT0 );

extern volatile UBaseType_t uxInterruptNesting;
#define portASSERT_IF_IN_ISR()  configASSERT( uxInterruptNesting == 0 )

#define portNOP()               __asm volatile ( "ssnop" )
/*-----------------------------------------------------------*/

#ifdef	__cplusplus
}
#endif

#endif	/* CRIT_SECT_H */

