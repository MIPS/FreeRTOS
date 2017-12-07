/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/


#include "FreeRTOS.h"
#include "int_handler.h"

#include "../common/intc.h"
#include "vi_intc.h"

extern volatile uint32_t *GIC;
extern void vIntcISR1();
extern void vIntcISR2();
extern void vIntcISR3();
extern void vIntcISR4();
extern void vIntcISR5();
extern void vIntcISR6();
extern void vIntcISR7();

static HANDLER_DESC_T *cpu_irqs[8];

void intc_isr1(void)
{
	cpu_irqs[1]->function(cpu_irqs[1]->parameter);
}
void intc_isr2(void)
{
	cpu_irqs[2]->function(cpu_irqs[2]->parameter);
}
void intc_isr3(void)
{
	cpu_irqs[3]->function(cpu_irqs[3]->parameter);
}
void intc_isr4(void)
{
	cpu_irqs[4]->function(cpu_irqs[4]->parameter);
}
void intc_isr5(void)
{
	cpu_irqs[5]->function(cpu_irqs[5]->parameter);
}
void intc_isr6(void)
{
	cpu_irqs[6]->function(cpu_irqs[6]->parameter);
}
void intc_isr7(void)
{
	cpu_irqs[7]->function(cpu_irqs[7]->parameter);
}

void intc_init(INTC_T* ctrlr)
{
	VI_INTC_T* ctrlr_vi = (VI_INTC_T*)ctrlr;
	/*
	 * ICU in M class cores is same as a single core GIC
	 * manually set GIC base as cannot probe from CPO regs on none CPS systems/
	 */
	GIC = (uint32_t *) ctrlr_vi->base_addr;
}

int32_t intc_RegisterHandler(INTC_T* ctrlr, HANDLER_DESC_T* irq)
{
	typedef void(*irqf)(void);
	const irqf irq_lookup[] = {
			(irqf)NULL, /* SW0 is reserved */
			&vIntcISR1,
			&vIntcISR2,
			&vIntcISR3,
			&vIntcISR4,
			&vIntcISR5,
			&vIntcISR6,
			&vIntcISR7
	};

	if (irq->int_num == SW0 || irq->int_num == TIMER_IRQ)
		return -1;

	/* only allow 1 ext irq per CPU irq for now - TODO allow a list of handlers per int_num */
	if(cpu_irqs[irq->int_num] != NULL)
		return -1;

	/* connect up */
	cpu_irqs[irq->int_num] = irq;
	pvPortInstallISR( irq->int_num, irq_lookup[irq->int_num & 0x7]);

	/* route in ICU/GIC -TODO route function should be split out to allow usage without ICU/GIC hardware */
	vLevelTrigExternalNonEicInterrupt( irq->ext_num, 1 );
	vRouteExternalNonEicInterrupt( irq->ext_num, 0, irq->int_num - 2 );

	return 0;
}

void intc_ack(int num)
{
	GIC_SH_WEDGE =  num;
}



