#include <mips/cpu.h>
#include <regdef.h>
#include "ISR_Support.h"
 	
	.extern uartRxIsr
	.extern EthernetRxRelease
	.extern GraphCalcRelease
	.global vUartRxISR
	.global vEthRxISR
	.global vGraphCalcISR


/******************************************************************/

 	.set		noreorder
	.set 		noat

 	.ent		vUartRxISR

vUartRxISR:
	portSAVE_CONTEXT

	jal		uartRxIsr
	nop

	portRESTORE_CONTEXT

	.end		vUartRxISR


 	.ent		vEthRxISR

vEthRxISR:
	portSAVE_CONTEXT

	jal		EthernetRxRelease
	nop

	portRESTORE_CONTEXT
	.end		vEthRxISR
