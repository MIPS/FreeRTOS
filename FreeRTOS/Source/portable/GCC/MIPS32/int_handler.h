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

#ifndef INT_HANDLER_H
#define	INT_HANDLER_H

#ifdef	__cplusplus
extern "C"
{
#endif

#include "portmacro.h"

/* Nicer names for the interrupt bits */
#define SW0                 0x00
#define SW1                 0x01
#define HW0                 0x02
#define HW1                 0x03
#define HW2                 0x04
#define HW3                 0x05
#define HW4                 0x06
#define HW5                 0x07

/* GIC Base definitions */
#define GCR_BASE GCR[0x8/sizeof(uint32_t)]
#define GCR_GIC_BASE GCR[0x80/sizeof(uint32_t)]
#define GIC_EN	1
#define GCR_GIC_STATUS GCR[0xd0/sizeof(uint32_t)]

#define GIC_SH_CONFIG		GIC[0]
#define NUMINTERRUPTS_MSK	0x007f0000
#define NUMINTERRUPTS_SHF	16
#define PVPES_MSK 			0x000001ff
#define PVPES_SHF			0
#define GIC_SH_POL31_0		GIC[0x100/sizeof(uint32_t)]
#define GIC_SH_TRIG31_0		GIC[0x180/sizeof(uint32_t)]
#define GIC_SH_DUAL31_0		GIC[0x200/sizeof(uint32_t)]
#define GIC_SH_WEDGE		GIC[0x280/sizeof(uint32_t)]
#define GIC_SH_RMASK31_0	GIC[0x300/sizeof(uint32_t)]
#define GIC_SH_SMASK31_0	GIC[0x380/sizeof(uint32_t)]
#define GIC_SH_PEND31_0		GIC[0x480/sizeof(uint32_t)]
#define GIC_SH_MAP0_PIN		GIC[0x500/sizeof(uint32_t)]
#define GIC_SH_MAP0_CORE	GIC[0x2000/sizeof(uint32_t)]
#define GIC_CORE_SPACER		(0x20/sizeof(uint32_t))

#define	GIC_COREi_CTL		GIC[0x8000/sizeof(uint32_t)]
#define GIC_COREi_PEND		GIC[0x8004/sizeof(uint32_t)]
#define	GIC_COREi_MASK		GIC[0x8008/sizeof(uint32_t)]
#define	GIC_COREi_RMASK		GIC[0x800C/sizeof(uint32_t)]
#define GIC_COREi_SMASK		GIC[0x8010/sizeof(uint32_t)]
#define GIC_COREi_TIMER_MAP	GIC[0x8048/sizeof(uint32_t)]
#define GIC_COREi_SWInt0_MAP	GIC[0x8054/sizeof(uint32_t)]
#define GIC_COREi_IDENT		GIC[0x8088/sizeof(uint32_t)]
#define GIC_COREi_EICVEC0	GIC[0x8800/sizeof(uint32_t)]

#define SWINT0_MASK	0x10
#define SWINT1_MASK	0x20
#define TIMER_MASK	0x04

#define SWINT_ROUTABLE	0x08
#define TIMER_ROUTABLE	0x02
#define EIC_MODE	0x01

#define TIMER_PEND 0x4
#define SWINT0_PEND 0x10

#define MAP_TO_PIN	0x80000000

#define INT_TO_MAP_LOCAL(X) (EIC ? (X) : ((X) - 2))

/* Where timer irq is wired as reported by hw */
#define TIMER_IRQ		((mips32_getintctl() & INTCTL_IPTI) >> INTCTL_IPTI_SHIFT)
#define SR_TIMER_IRQ	(SR_IM0 << TIMER_IRQ)

/* GIC Location */
extern volatile uint32_t *GCR;
extern volatile uint32_t *GIC;
extern uint32_t EIC;

/* Initialise the GIC */
void vPortInitGIC( void );

/* Interrupt manipulation */
extern void pvPortInstallISR( uint32_t, void ( * )( void ) );

/*
 * The software interrupt handler that performs the yield.
 */
extern void vPortYieldISR( void );

extern UBaseType_t uxPortSetInterruptMaskFromISR( void );
extern void vPortClearInterruptMaskFromISR( UBaseType_t );
extern void vApplicationSetupSoftwareInterrupt( void );
extern void vLevelTrigExternalNonEicInterrupt( uint32_t ext_int, uint32_t pol);
extern void vRouteExternalNonEicInterrupt( uint32_t ext_int, uint32_t vpe, uint32_t vpe_int);

#ifdef __mips_dsp
#define SR_DSP  SR_MX
#else
#define SR_DSP  0
#endif

#define portINITIAL_SR      ( SR_SINT0 | SR_DSP | SR_EXL | SR_IE )
#define portCLEAR_PENDING_INTERRUPTS()      mips_bicsr( SR_IMASK )

#define portSET_INTERRUPT_MASK_FROM_ISR()   uxPortSetInterruptMaskFromISR()
#define portCLEAR_INTERRUPT_MASK_FROM_ISR( uxSavedStatusRegister ) \
	vPortClearInterruptMaskFromISR( uxSavedStatusRegister )


#ifdef	__cplusplus
}
#endif

#endif	/* INT_HANDLER_H */

