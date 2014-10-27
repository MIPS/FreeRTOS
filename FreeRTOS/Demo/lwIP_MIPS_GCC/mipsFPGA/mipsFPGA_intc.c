#include "mipsFPGA_intc.h"

void vIntcISR(void);
static MIPSFPGA_INTC_T *global_intc; 

void intc_Isr(void)
{
	u32 volatile *fpga_intc = (u32*)global_intc->base_addr;
	MIPSFPGA_INTC_HANDLER_T *handler;
	u32 interrupt_bits;
	
	global_intc->interrupt_count++;
	interrupt_bits = fpga_intc[AIX_INTC_ISR] & global_intc->all_mask;
	while (interrupt_bits) {
		handler = global_intc->handler_list;
		while (handler) {
			if (interrupt_bits & handler->mask) {
				handler->function(handler->parameter);
				fpga_intc[AIX_INTC_IAR] |= handler->mask;
			}
			handler = handler->next;
		}
		interrupt_bits = fpga_intc[AIX_INTC_ISR] & global_intc->all_mask;
	}
}

static void intc_CreatePool(MIPSFPGA_INTC_T * intc)
{
	u32 index;

	for (index = 0; index < MAX_HANDLERS; index++) {
		intc->handlers[index].next = intc->handler_pool;
		intc->handler_pool = &intc->handlers[index];
	}
}

static MIPSFPGA_INTC_HANDLER_T *intc_TakeItem(MIPSFPGA_INTC_T * intc)
{
	MIPSFPGA_INTC_HANDLER_T *handler;

	handler = intc->handler_pool;
	intc->handler_pool = intc->handler_pool->next;

	return handler;
}

static void intc_AddItemToList(MIPSFPGA_INTC_T * intc,
			       MIPSFPGA_INTC_HANDLER_T * handler)
{
	handler->next = intc->handler_list;
	intc->handler_list = handler;
}

static void intc_InstallIsr(MIPSFPGA_INTC_T * intc)
{
	u32 int_num = 6;
	u32 *fpga_intc = (u32*)intc->base_addr;

	fpga_intc[AIX_INTC_IAR] =  0x1F; 
	fpga_intc[AIX_INTC_IER] =  0x00; 
	pvPortInstallISR( int_num, vIntcISR);
	global_intc = intc;
	fpga_intc[AIX_INTC_MER] = AIX_INTC_ME | AIX_INTC_HIE; 
}

void intc_Init(MIPSFPGA_INTC_T * intc)
{
	u32 *fpga_intc = (u32 *) intc->base_addr;

	intc_CreatePool(intc);
	fpga_intc[AIX_INTC_IAR] = 0x1F;
	fpga_intc[AIX_INTC_IER] = 0x00;
	intc_InstallIsr(intc);
	fpga_intc[AIX_INTC_MER] = AIX_INTC_ME | AIX_INTC_HIE;
	intc->all_mask = 0;
}

int intc_RegisterHandler(MIPSFPGA_INTC_T * intc, HANDLER_DESC_T * desc)
{
	MIPSFPGA_INTC_HANDLER_T *handler;
	u32 *fpga_intc = (u32 *) intc->base_addr;

	handler = intc_TakeItem(intc);
	if (!handler)
		return -1;	//FAIL
	handler->mask = desc->mask;
	handler->function = desc->function;
	handler->parameter = desc->parameter;
	intc_AddItemToList(intc, handler);
	fpga_intc[AIX_INTC_IER] |= desc->mask;

	intc->all_mask |= desc->mask;

	return 0;		//SUCCESS
}


