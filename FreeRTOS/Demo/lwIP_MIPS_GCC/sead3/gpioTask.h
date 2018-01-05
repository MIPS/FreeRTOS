#ifndef __GPIOTASK_H__
#define __GPIOTASK_H__


typedef struct {
	volatile uint32_t *regs;
} SEAD3_GPIO_T;

void gpioTask(void *parameters);

#endif
