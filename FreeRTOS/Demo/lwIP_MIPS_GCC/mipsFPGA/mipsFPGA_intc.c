#include "mipsFPGA_intc.h"

extern void vIntcISR(void);
static MIPSFPGA_INTC_T *global_intc; 

void intc_Isr(void)
{
	uint32_t volatile *regs = (uint32_t*)global_intc->base_addr;
	HANDLER_DESC_T *handler;
	uint32_t interrupt_bits;
	
	global_intc->interrupt_count++;
	interrupt_bits = regs[AIX_INTC_ISR] & global_intc->all_mask;
	while (interrupt_bits) {
		handler = global_intc->handler_list;
		while (handler) {
			if (interrupt_bits & handler->mask) {
				handler->function(handler->parameter);
				regs[AIX_INTC_IAR] |= handler->mask;
			}
			handler = handler->next;
		}
		interrupt_bits = regs[AIX_INTC_ISR] & global_intc->all_mask;
	}
}

static void intc_CreatePool(MIPSFPGA_INTC_T * intc)
{
	uint32_t index;

	for (index = 0; index < MAX_HANDLERS; index++) {
		intc->handlers[index].next = intc->handler_pool;
		intc->handler_pool = &intc->handlers[index];
	}
}

static HANDLER_DESC_T *intc_TakeItem(MIPSFPGA_INTC_T * intc)
{
	HANDLER_DESC_T *handler;

	handler = intc->handler_pool;
	intc->handler_pool = intc->handler_pool->next;

	return handler;
}

static void intc_AddItemToList(MIPSFPGA_INTC_T * intc,
		HANDLER_DESC_T * handler)
{
	handler->next = intc->handler_list;
	intc->handler_list = handler;
}

static void intc_InstallIsr(MIPSFPGA_INTC_T * intc)
{
	uint32_t int_num = 6;
	volatile uint32_t *regs  = (uint32_t*)intc->base_addr;

	regs[AIX_INTC_IAR] =  0x1F;
	regs[AIX_INTC_IER] =  0x00;
	pvPortInstallISR( int_num, vIntcISR);
	global_intc = intc;
	regs[AIX_INTC_MER] = AIX_INTC_ME | AIX_INTC_HIE;
}

void intc_init(INTC_T * intc)
{
	MIPSFPGA_INTC_T* m_intc = (MIPSFPGA_INTC_T *)intc;
	volatile uint32_t *regs = (volatile uint32_t *) m_intc->base_addr;

	intc_CreatePool(m_intc);
	regs[AIX_INTC_IAR] = 0x1F;
	regs[AIX_INTC_IER] = 0x00;
	intc_InstallIsr(m_intc);
	regs[AIX_INTC_MER] = AIX_INTC_ME | AIX_INTC_HIE;
	m_intc->all_mask = 0;
}

int32_t intc_RegisterHandler(INTC_T *intc, HANDLER_DESC_T *desc)
{
	MIPSFPGA_INTC_T *m_intc = (MIPSFPGA_INTC_T *)intc;
	HANDLER_DESC_T *handler;
	volatile uint32_t *regs = (volatile uint32_t *) m_intc->base_addr;

	desc->mask = 0x1 << desc->ext_num;

	handler = intc_TakeItem(m_intc);
	if (!handler)
		return -1;
	handler->mask = desc->mask;
	handler->function = desc->function;
	handler->parameter = desc->parameter;
	intc_AddItemToList(m_intc, handler);
	regs[AIX_INTC_IER] |= desc->mask;

	m_intc->all_mask |= desc->mask;

	return 0;
}

void intc_ack(int num)
{
	//done in irq handler.
}

