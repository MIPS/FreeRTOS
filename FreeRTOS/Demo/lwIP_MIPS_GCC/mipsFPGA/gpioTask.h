#ifndef __GPIOTASK_H__
#define __GPIOTASK_H__

typedef struct {
	volatile uint32_t *port_data;
} MIPSFPGA_GPIO_T;

void gpioTask(void *parameters);

#endif
