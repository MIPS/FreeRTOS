/*
	FreeRTOS V8.2 - Copyright (C) 2015 Real Time Engineers Ltd.
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

#include <assert.h>
#include <mips/cpu.h>
#include <stdint.h>
#include <string.h>

#include "int_handler.h"

/* We move away from a pure assembly version of the hardware_init_hook so that
the same functionality can be used regardless of ISA revision*/
void hardware_init_hook(void)
{
	/* Ensure we have a Config3 register - if Vectored interrupts are a feature
	of the core, then it should be available */
	assert( mips32_getconfig2() & CFG2_M );

	/* Initialise the GIC */
	vPortInitGIC();

#ifdef __mips_micromips

	/* Ensure micromips is supported */
	assert( mips32_getconfig3() & CFG3_MCU );
	assert( ( mips32_getconfig3() & CFG3_ISA_MASK ) != CFG3_ISA_MIPS );

	/* set micromips exception entry point */
	mips32_bisconfig3( CFG3_IOE );
#else
	/* Ensure mips32 is supported */
	if ( mips32_getconfig3() & CFG3_MCU ) {
		assert( ( mips32_getconfig3() & CFG3_ISA_MASK ) != CFG3_ISA_UMIPS );

		/* set mips32 exception entry point */
		mips32_bicconfig3( CFG3_IOE );
	}
#endif

	/* Ensure the core has Vectored Interrupts support */
	assert( mips32_getconfig3() & CFG3_VI );

	/* Setup Vectored Interrupts */
	mips_biscr( CR_IV );
	mips32_setintctl( ( mips32_getintctl() & ~INTCTL_VS ) | INTCTL_VS_32 );

#ifdef __mips_dsp
	/* Ensure the core has DSP module */
	assert( mips32_getconfig3() & CFG3_DSPP );

	/* Enable DSP */
	mips_bissr( SR_MX );
	assert( mips_getsr() & SR_MX );
#endif

#ifdef __mips_hard_float
	/* Ensure the core has FPU implemented */
	assert( mips32_getconfig1() & CFG1_FP );
#endif
}

