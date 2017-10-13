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

#ifndef ISR_SUPPORT_H
#define	ISR_SUPPORT_H

/* This file is only used by the port_asm.S file, and so is pretty much a
no-op if (accidentally) included within any other non-assembly files */
#if defined(__ASSEMBLER__)

#include <mips/hal.h>

#include "FreeRTOSConfig.h"
#include "ctx.S"


/* Save this processor context. This will happen when a timer tick happens.
As we are using Count/Compare as our timer, this fires on Status(HW5). */
.macro  portSAVE_CONTEXT

	/* Make room for the context. First save the current status so it can be
	manipulated, and the cause and EPC registers so their original values are
	captured. */
	addiu       sp, sp, -CTX_SIZE
	sw          k1, CTX_K1(sp)

	/* k1 is used as the frame pointer. */
	addu         k1, zero, sp

	/* Save the context into the space just created. */
	_gpctx_save

	/* Save the stack pointer. */
	la          s6, uxSavedTaskStackPointer
	sw          k1, (s6)
	lw          k1, CTX_K1(sp)

	.endm

/******************************************************************/

/* Restore the processor context. */
.macro  portRESTORE_CONTEXT

	la          s6, uxSavedTaskStackPointer
	lw          a0, (s6)

	/* Restore the context. */
	_gpctx_load

	la          sp, uxSavedTaskStackPointer
	lw          sp, (sp)


	addiu       sp, sp, CTX_SIZE

	eret
	nop
	
	.endm

/******************************************************************/

	/* Yield context save */
	.macro portYIELD_SAVE

	/* Make room for the context. First save the current status so it can be
	manipulated. */
	addiu       sp, sp, -CTX_SIZE
	sw          k1, CTX_K1(sp)

	/* k0cd is used as the frame pointer. */
	addu         k1, zero, sp

	/* Save the context into the space just created. */
	_gpctx_save


	/* Save the stack pointer to the task. */
	la			s7, pxCurrentTCB
	lw			s7, (s7)
	sw			k1, (s7)

	mfc0		s6, C0_CAUSE
	ins			s6, zero, 8, 1
	mtc0		s6, C0_CAUSE
	ehb

	.endm

/******************************************************************/

	.macro portYIELD_RESTORE

	/* Set the context restore register from the TCB. */
	la			s0, pxCurrentTCB
	lw			s0, (s0)
	lw			a0, (s0)

	_gpctx_load

	/* Restore the stack pointer from the TCB. */
	la			sp, pxCurrentTCB
	lw			sp, (sp)
	lw			sp, (sp)

	/* Remove stack frame. */
	addiu		sp, sp, CTX_SIZE

	eret
	nop

	.endm

#endif /* #if defined(__ASSEMBLER__) */

#endif	/* ISR_SUPPORT_H */
