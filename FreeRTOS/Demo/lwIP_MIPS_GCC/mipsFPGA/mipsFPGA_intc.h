#ifndef __MIPSFPGA_INTC_H__
#define __MIPSFPGA_INTC_H__

#include "types.h"

#define MIPSFPGA_INTC_ADDR	0xB0200000

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
	u32 mask;
	void (*function)(void*);
	void *parameter;
}HANDLER_DESC_T;

typedef struct handler_struct {
	s32 mask;
	void (*function) (void *);
	void *parameter;
	struct handler_struct *next;
} MIPSFPGA_INTC_HANDLER_T;


typedef struct {
	u32 base_addr;
	MIPSFPGA_INTC_HANDLER_T handlers[MAX_HANDLERS];
	MIPSFPGA_INTC_HANDLER_T *handler_pool;
	MIPSFPGA_INTC_HANDLER_T*handler_list;
	u32 all_mask;
    u32 interrupt_count;
} MIPSFPGA_INTC_T;

void intc_Init(MIPSFPGA_INTC_T *intc);
s32 intc_RegisterHandler(MIPSFPGA_INTC_T *intc, HANDLER_DESC_T *desc);


#endif
