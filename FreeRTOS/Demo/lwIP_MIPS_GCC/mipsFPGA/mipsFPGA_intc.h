#ifndef __MIPSFPGA_INTC_H__
#define __MIPSFPGA_INTC_H__

#include "intc.h"

#define AIX_INTC_ISR	0
#define AIX_INTC_IPR	1
#define AIX_INTC_IER	2
#define AIX_INTC_IAR	3
#define AIX_INTC_SIE	4
#define AIX_INTC_CIE	5
#define AIX_INTC_IVR	6
#define AIX_INTC_MER	7
#define AIX_INTC_ILR	8

#define AIX_INTC_INT0	1
#define AIX_INTC_INT1	2
#define AIX_INTC_INT2	4
#define AIX_INTC_INT3	8
#define AIX_INTC_INT4	16

#define AIX_INTC_ME		1
#define AIX_INTC_HIE	2

#define MAX_HANDLERS	8

typedef struct {
	uint32_t base_addr;
	HANDLER_DESC_T handlers[MAX_HANDLERS];
	HANDLER_DESC_T *handler_pool;
	HANDLER_DESC_T *handler_list;
	uint32_t all_mask;
	uint32_t interrupt_count;
} MIPSFPGA_INTC_T;

#endif
