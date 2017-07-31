#ifndef __GPIOTASK_H__
#define __GPIOTASK_H__

#define GPIO_BASE_ADDR 0xB0600000

typedef struct {
	uint32_t *port_data;
} MIPSFPGA_GPIO_T;

void gpioTask(void *parameters);

#endif