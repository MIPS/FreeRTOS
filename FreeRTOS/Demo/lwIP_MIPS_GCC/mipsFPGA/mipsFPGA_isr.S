#include <mips/cpu.h>
#include <regdef.h>
#include "ISR_Support.h"
 	
	.extern intc_Isr
	.global vIntcISR


/******************************************************************/

 	.set		noreorder
	.set 		noat

 	.ent		vIntcISR
vIntcISR:

	portSAVE_CONTEXT

	jal		intc_Isr
	nop

	portRESTORE_CONTEXT

	.end		vIntcISR
