/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/

#include <mips/cpu.h>
#include <regdef.h>
#include "ISR_Support.h"
 	
	.extern intc_isr1
	.extern intc_isr2
	.extern intc_isr3
	.extern intc_isr4
	.extern intc_isr5
	.extern intc_isr6
	.extern intc_isr7
	.global vIntcISR1
	.global vIntcISR2
	.global vIntcISR3
	.global vIntcISR4
	.global vIntcISR5
	.global vIntcISR6
	.global vIntcISR7
/******************************************************************/

	.set 		noat

 	.ent		vIntcISR1
vIntcISR1:

	portSAVE_CONTEXT

	jal		intc_isr1

	portRESTORE_CONTEXT

	.end		vIntcISR1

 	.ent		vIntcISR2
vIntcISR2:

	portSAVE_CONTEXT

	jal		intc_isr2

	portRESTORE_CONTEXT

	.end		vIntcISR2

	.ent		vIntcISR3
vIntcISR3:

	portSAVE_CONTEXT

	jal		intc_isr3

	portRESTORE_CONTEXT

	.end		vIntcISR3

 	.ent		vIntcISR4
vIntcISR4:

	portSAVE_CONTEXT

	jal		intc_isr4

	portRESTORE_CONTEXT

	.end		vIntcISR4

 	.ent		vIntcISR5
vIntcISR5:

	portSAVE_CONTEXT

	jal		intc_isr5

	portRESTORE_CONTEXT

	.end		vIntcISR5

 	.ent		vIntcISR6
vIntcISR6:

	portSAVE_CONTEXT

	jal		intc_isr6

	portRESTORE_CONTEXT

	.end		vIntcISR6

 	.ent		vIntcISR7
vIntcISR7:

	portSAVE_CONTEXT

	jal		intc_isr7

	portRESTORE_CONTEXT

	.end		vIntcISR7

