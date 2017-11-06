#include "FreeRTOS.h"
#include "task.h"
#include "gpioTask.h"


void gpioTask(void *parameters)
{
	MIPSFPGA_GPIO_T *gpio = (MIPSFPGA_GPIO_T*)parameters; 
	int dir = 1, data = 0x0100;
	TickType_t xDelay = 512 / portTICK_PERIOD_MS;
	unsigned int switches,bit;
		
	for ( ;; ) {
		*gpio->port_data = data;
		if (dir == 1) {
		    data <<= 1;
		    if (data == 0x8000)
		        dir = 0;
		} else {
				data >>= 1;
		    if (data == 0x0100)
		        dir = 1;
		}
		vTaskDelay( xDelay );
		switches = *gpio->port_data & 0x0F;
		bit = 1;
		while (switches) {
			bit <<= 1;
			switches >>= 1;
		}
		xDelay = 512 / portTICK_PERIOD_MS / bit;
	}
}
