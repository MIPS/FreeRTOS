/***(C)2017***************************************************************
*
* Copyright (c) 2017, MIPS Tech, LLC.
****(C)2017**************************************************************/

#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "gpioTask.h"

const char message[] = "Hello from FreeRTOS on SEAD3  ";

/* Display Commands */
#define DISPLAY_CLEAR	0x1 /* Clear the display */
#define DISPLAY_HOME	0x2 /* Set cursor at home position */
#define DISPLAY_LINE2	0x4 /* Move to line 2 */

enum lcd_write_type {
	LCD_INST,
	LCD_DATA,
};

typedef struct lcd_regs {
	volatile uint32_t inst;
	volatile uint32_t _pad0;
	volatile uint32_t data;
	volatile uint32_t _pad1;
	volatile uint32_t cpld_status;
	volatile uint32_t _pad2;
	volatile uint32_t cpld_data;
} LCD_REG_T;

static volatile LCD_REG_T *regs;


static void cpld_wait(void)
{
	uint8_t status;

	do {
		status = regs->cpld_status;
	} while (status & 0x1);
}

static void lcd_write(enum lcd_write_type type, uint8_t val)
{
	uint8_t status;

	cpld_wait();

	do {
		status = regs->inst;
		cpld_wait();
		status = regs->cpld_data;
	} while (status & 0x80);

	switch (type) {
	case LCD_INST:
		regs->inst = val;
		break;

	case LCD_DATA:
		regs->data = val;
		break;
	}
}

static void display_set(int cmd)
{
	unsigned addr = 0;

	if (cmd & DISPLAY_CLEAR)
		lcd_write(LCD_INST, 0x01);

	if (cmd & DISPLAY_HOME)
		lcd_write(LCD_INST, 0x02);

	if (cmd & DISPLAY_LINE2)
		addr += 0x40;

	lcd_write(LCD_INST, 0x80 | addr);
}

static int display_putc(char c)
{
	lcd_write(LCD_DATA, c);
	return c;
}

static void print24(const char* str,unsigned int count)
{
	unsigned int i;
	unsigned int len = strlen(str);

	for(i=0; i<24; i++)
	{
		int index = (count + i) % len;
		display_putc(str[index]);
	}
}

void gpioTask(void *parameters)
{
	SEAD3_GPIO_T *gpio = (SEAD3_GPIO_T*)parameters;
	TickType_t xDelay = 250 / portTICK_PERIOD_MS;
	unsigned int count = 0;

	regs =(volatile LCD_REG_T *)gpio->regs;

	display_set(DISPLAY_CLEAR);
	display_set(DISPLAY_HOME);

	for ( ;; ) {
		display_set(DISPLAY_CLEAR);
		display_set(DISPLAY_HOME);

		print24(message,count);

		display_putc(message[count]);

		count = (count + 1) % (sizeof(message) -1);

		vTaskDelay( xDelay );

		xDelay = 500 / portTICK_PERIOD_MS;
	}
}
