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
#include <mips/cpu.h>

/* Set segment mapping to reinstate legacy mapping which is what FreeRTOS expects */
void __attribute__((section(".bootrom"))) set_segment_legacy(void)
{
	unsigned long temp;

	/*
	 * segment   |      PA      |      VA      |  SIZE  |  MODE
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    5      |  0x00000000  |   TLB mapped |   1G   |  User Mapped Cached
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    4      |  0x40000000  |   TLB mapped |   1G   |  User Mapped Cached
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    3      |  0x00000000  |  0x80000000  |  0.5G  |  Kernel Unampped Cached (KSEG0)
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    2      |  0x00000000  |  0xA0000000  |  0.5G  |  Kernel Unampped Uncached (KSEG1)
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    1      |  0x20000000  |  0xC0000000  |  0.5G  |  Kernel Unampped Cached
	 *-----------+--------------+--------------+--------+----------------------------------
	 *    0      |  0x40000000  |  0xE0000000  |  0.5G  |  Kernel Unampped Cached
	 *-----------+--------------+--------------+--------+----------------------------------
	 */
	temp = 0x40038003;
	_mips_mtc0(C0_SEGCTL0, temp);
	temp = 0x00030002;
	_mips_mtc0(C0_SEGCTL1, temp);
	temp = 0x004b804b;
	_mips_mtc0(C0_SEGCTL2, temp);
}

/*
 * This function is called early on after hard reset. You can use it to perform
 * any additional hardware initialisation.
 */
void __attribute__((section(".bootrom"))) __boot_init_hook(void)
{
	if (mips32_getconfig4() & CFG4_M)
		if (mips32_getconfig5() & (CFG5_K | CFG5_EVA))
			set_segment_legacy();
}

#ifdef TWO_STAGE_BOOT
/*
 * When building two stage boot, you need to fill in your code here to load
 * FreeRTOS and jump into it
 */
int main(void)
{
	while (1);

	return 0;
}
#endif
