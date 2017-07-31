#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "types.h"
#include "ci40_uart.h"
#include <ISR_Support.h>

#define uartQUEUE_LENGTH					( 1024 )
#define	mainTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

void uartRxIsr(void);
void vUartRxISR(void);
void vUartRxISR2(void);

#define MAX_UARTS	2
CI40_UART_T *uarts[MAX_UARTS];

//u32 interrupt_mask = 0;
//u32 interrupt_ack = 0;

void CI40_uart_init(CI40_UART_T *uart)
{
	u32 value = 0;

	uart->regs->lcr = SER_LCR_DLAB_MASK;
	value = uart->baud_clk / uart->speed;
	uart->regs->rbr_thr_dll = value & 0xFF;
	uart->regs->ier_dlh = value >> 8;
	value = uart->bits | uart->stop_bits << 2 | uart->parity << 3;
	uart->regs->lcr = value;

	uart->regs->mcr = 0x00;

	uart->regs->ier_dlh = SER_IER_ERBFI_MASK;
}

static void send_ch(CI40_UART_T *uart, u8 ch)
{

	while(! (uart->regs->lsr & SER_LSR_THRE_MASK));
	uart->regs->rbr_thr_dll = ch;
}

static u8 read_ch(CI40_UART_T *uart)
{
	// read data will clear interrupt
	return uart->regs->rbr_thr_dll;
}

void uartTask(void *parameters)
{
	u8 ch;
	u32 reg;
	CI40_UART_T *uart = (CI40_UART_T*) parameters;
	u32 int_num = 6;
	u32	ext_int = UART2_INT_NUM;

	uarts[0] = uart;
	CI40_uart_init(uart);
	uart->TxQueue = xQueueCreate( uartQUEUE_LENGTH, sizeof( u8 ) );
	uart->RxQueue = xQueueCreate( uartQUEUE_LENGTH, sizeof( u8 ) );

	pvPortInstallISR( int_num, vUartRxISR );
	vLevelTrigExternalNonEicInterrupt( ext_int, 1 );
	vRouteExternalNonEicInterrupt( ext_int, 1, int_num - 2 );
	
	xSemaphoreGive( uart->ReadySemaphore );
	for ( ;; ) {
		xQueueReceive( uart->TxQueue, &ch, portMAX_DELAY);
		send_ch( uart, ch );
	}
}


void uartTxDirect(CI40_UART_T *uart, u8 *str)
{
	u8 ch;
	while( *str ) {
		ch = *str;
		send_ch(uart, ch);
		str ++;
	}
}

void uartTx(CI40_UART_T *uart, u8 *str)
{
	u8 ch;
	while( *str ) {
		ch = *str;
		xQueueSend( uart->TxQueue, &ch, portMAX_DELAY );
		str ++;
	}
}

u8 uartRx(CI40_UART_T *u)
{
	u8 ch;
	xQueueReceive( u->RxQueue, &ch, portMAX_DELAY);
	return ch;
}

void uartRxIsr(void)
{
	unsigned char ch;
	CI40_UART_T *uart = uarts[0];

#ifdef DEMO_DEBUG
	uart->RxInterruptCount ++;
#endif
	// Reading data will clear interrupt on uart
	ch = read_ch( uart );
	xQueueSend( uart->RxQueue, &ch, portMAX_DELAY);
}

size_t mipsUART_writer(void *cookie, char *data, size_t length)
{
	CI40_UART_T *uart = (CI40_UART_T *) cookie;
	size_t bytes = length;

	while (bytes--)
		send_ch(uart, (int8_t) * data++);
	return length;
}

const cookie_io_functions_t mipsUART_cookie = {
	.write = (cookie_write_function_t *) mipsUART_writer,
};

FILE *mipsUART_fopen(CI40_UART_T * uart, const char *mode)
{
	FILE *file;
	file = fopencookie(uart, mode, mipsUART_cookie);
	setvbuf(file, NULL, _IOLBF, 0);
	return file;
}
