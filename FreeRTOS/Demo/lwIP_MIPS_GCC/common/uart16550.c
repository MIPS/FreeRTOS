/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/

#define _GNU_SOURCE /* for fopen cookie */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "uart16550.h"

#define uartQUEUE_LENGTH		( 1024 )
#define	mainTASK_PRIORITY		( tskIDLE_PRIORITY + 1 )


static void uart16550_rx_isr(void *parameter);

void uart16550_init(UART16550_T *uart, HANDLER_DESC_T* handler_info)
{
	uint32_t value = 0;

	uart->regs->lcr = SER_LCR_DLAB_MASK;
	value = uart->baud_clk / (uart->speed * 16);
	uart->regs->rbr_thr_dll = value & 0xFF;
	uart->regs->ier_dlh = value >> 8;
	value = uart->bits | uart->stop_bits << 2 | uart->parity << 3;
	uart->regs->lcr = value;

	uart->regs->mcr = 0x00;

	uart->regs->ier_dlh = SER_IER_ERBFI_MASK;

	uart->TxQueue = xQueueCreate( uartQUEUE_LENGTH, sizeof( uint8_t ) );
	uart->RxQueue = xQueueCreate( uartQUEUE_LENGTH, sizeof( uint8_t ) );

	handler_info->function = uart16550_rx_isr;
	handler_info->parameter = uart;

	intc_RegisterHandler(uart->intc, handler_info);

	xTaskCreate( uart16550_task, ( signed char * ) "uart Task",
			configNORMAL_STACK_SIZE, uart, tskIDLE_PRIORITY + 1, NULL );
}

static void send_ch(UART16550_T *uart, uint8_t ch)
{

	while(! (uart->regs->lsr & SER_LSR_THRE_MASK));
	uart->regs->rbr_thr_dll = ch;
}

static uint8_t read_ch(UART16550_T *uart)
{
	// read data will clear interrupt
	return uart->regs->rbr_thr_dll;
}

void uart16550_task(void *parameters)
{
	uint8_t ch;
	uint32_t reg;
	UART16550_T *uart = (UART16550_T*) parameters;
	
	xSemaphoreGive( uart->ReadySemaphore );
	for ( ;; ) {
		

#ifdef UART_IRQS_BROKEN_SO_POLL
		if(xQueueReceive( uart->TxQueue, &ch, 1))
			send_ch( uart, ch );

		if (uart->regs->lsr &  SER_LSR_DR_MASK) {
			ch = read_ch( uart );
			xQueueSend( uart->RxQueue, &ch, portMAX_DELAY);
		}
#else
		xQueueReceive( uart->TxQueue, &ch, portMAX_DELAY);
		send_ch( uart, ch );
#endif
	}
}


void uart16550_txdirect(UART16550_T *uart, uint8_t *str)
{
	uint8_t ch;
	while( *str ) {
		ch = *str;
		send_ch(uart, ch);
		str ++;
	}
}

void uart16550_tx(UART16550_T *uart, uint8_t *str)
{
	uint8_t ch;
	while( *str ) {
		ch = *str;
		xQueueSend( uart->TxQueue, &ch, portMAX_DELAY );
		str ++;
	}
}

uint8_t uar16550_rx(UART16550_T *u)
{
	uint8_t ch;
	xQueueReceive( u->RxQueue, &ch, portMAX_DELAY);
	return ch;
}

static void uart16550_rx_isr(void *parameter)
{
	unsigned char ch;
	UART16550_T *uart = (UART16550_T*) parameter;

#ifdef DEMO_DEBUG
	uart->RxInterruptCount ++;
#endif
	// Reading data will clear interrupt on uart
	if (uart->regs->lsr &  SER_LSR_DR_MASK) {
		ch = read_ch( uart );
		xQueueSend( uart->RxQueue, &ch, portMAX_DELAY);
	}
}

size_t uart16550_writer(void *cookie, char *data, size_t length)
{
	UART16550_T *uart = (UART16550_T *) cookie;
	size_t bytes = length;

	while (bytes--)
		send_ch(uart, (int8_t) * data++);
	return length;
}

const cookie_io_functions_t uart16550_cookie = {
	.write = (cookie_write_function_t *) uart16550_writer,
};

FILE *uart16550_fopen(UART16550_T * uart, const char *mode)
{
	FILE *file;
	file = fopencookie(uart, mode, uart16550_cookie);
	setvbuf(file, NULL, _IOLBF, 0);
	return file;
}
