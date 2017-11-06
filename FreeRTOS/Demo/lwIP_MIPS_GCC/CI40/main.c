/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/

/*
 * A simple demo.
 * Produces a web page of the task states
 * Produces serial output from a simple monitor task
 */
/* Standard include. */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* lwIP includes. */
#include "lwip/dhcp.h"
#include "../common/uart16550.h"
#include "../common/lan9211.h"
#include "../common/intc.h"
#include "../common/vi_intc.h"
#include "web.h"
#include "ci40_net.h"
#include "ci40_eth.h"


/* Priorities at which the tasks are created. */
#define	zeroTASK_PRIORITY			( tskIDLE_PRIORITY )
#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define	twoTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define	threeTASK_PRIORITY			( tskIDLE_PRIORITY + 3 )

/* Clocks */
#define CI40_UART_CLK_FREQ	1843200


/* Ci40 MEMORY MAP */
#define CI40_UART1_BASE		0xB8101400
#define CI40_UART2_BASE		0xB8101500
#define CI40_EHTNET_BASE	0xB8140000

/*-----------------------------------------------------------*/

/* Tasks */
static void MonitorTask(void *parameters);

/*-----------------------------------------------------------*/

/* Type definitions */
typedef struct {
	UART16550_T *uart;
	CI40_ETH_T *ci40_eth;
	FILE *dbgFile;
} MONITOR_INFO_T;

/* Variables */
static CI40_ETH_T ci40_eth;
static UART16550_T uart;
static FILE *dbgFile;
static MONITOR_INFO_T monitor_info;
static WEB_INFO_T web_info;
static VI_INTC_T intc;

static HANDLER_DESC_T ethirq;
static HANDLER_DESC_T uartirq;


/*-----------------------------------------------------------*/

int main(void)
{
	dbgFile = uart16550_fopen(&uart, "w");

	uart.base_addr = CI40_UART2_BASE;
	uart.regs = (UART16550_REGS_T*)CI40_UART2_BASE;
	uart.baud_clk = CI40_UART_CLK_FREQ;
	uart.speed = 115200;
	uart.bits = SER_8BITS;
	uart.stop_bits = SER_1STOPBIT;
	uart.parity = SER_NOPARITY;
	uart.ReadySemaphore = xSemaphoreCreateBinary( );
	uart.intc = (INTC_T*)&intc;

	uartirq.int_num=4;
	uartirq.ext_num=25; /* uart 1 = IRQ24, uart 2 = IRQ25 */

	uart16550_init(&uart, &uartirq);

	ethirq.int_num=5;
	ethirq.ext_num=50;
	
	ci40_eth.intc = (INTC_T*)&intc;

	ci40_eth_init(&ci40_eth,&ethirq);

	monitor_info.uart = &uart;
	monitor_info.dbgFile = dbgFile;
	monitor_info.ci40_eth = &ci40_eth;
	xTaskCreate( MonitorTask, ( signed char * ) "Monitor Task",
		configNORMAL_STACK_SIZE, &monitor_info, oneTASK_PRIORITY, NULL );

	web_info.uart = &uart;
	web_info.dbgFile = dbgFile;
	web_info.netif = &ci40_eth.netif;
	sys_thread_new( "Web server", vBasicWEBServer, &web_info,
		configNORMAL_STACK_SIZE, oneTASK_PRIORITY+1 );

	/* Start the tasks running. */
	vTaskStartScheduler();

	return 0;
}
/*-----------------------------------------------------------*/


void vAssertCalled( const char *pcFileName, unsigned long ulLine )
{
	printf("Guru meditation in %s:%d\n", pcFileName, ulLine);
	for (;;);
}

static void MonitorTask(void *parameters)
{
	MONITOR_INFO_T *monitor_info = (MONITOR_INFO_T *)parameters;
	UART16550_T *uart = monitor_info->uart;
	FILE *dbgFile = monitor_info->dbgFile; 
	struct netif *netif = &monitor_info->ci40_eth->netif;
#ifdef DEMO_DEBUG
	extern uint32_t EthernetRxReleaseCount;
	extern uint32_t EthRxTaskCount;
#endif
	static uint8_t outbuff[1024];
	uint8_t ch;
	uint8_t ip[4] = { 0, 0, 0, 0 };
	uint8_t *dhcp_states[] = {
		"DHCP OFF", "DHCP REQUESTING", "DHCP INIT", "DHCP REBOOTING", 
		"DHCP REBINDING", "DHCP RENEWING", "DHCP SELECTING", "DHCP INFORMING",
		"DHCP CHECKING", "DHCP PERMANENT", "DHCP BOUND", "DHCP RELEASING",
		"DHCP BACKING OFF" };

	xSemaphoreTake( uart->ReadySemaphore, portMAX_DELAY );

	for ( ;; ) {
		fprintf( dbgFile, "\r\nMonitor Task\r\n" );
		fprintf( dbgFile, "<t>     Task data\r\n" );
	#ifdef DEMO_DEBUG
		fprintf( dbgFile, "<c>     Interrupt Counts\r\n" );
	#endif
		fprintf( dbgFile, "<n>     Network Addresses\r\n" );
		fprintf( dbgFile, "<d>     DHCP state\r\n" );
		fprintf( dbgFile, "\r\n");

		// block on rx queue
		xQueueReceive( uart->RxQueue, &ch, portMAX_DELAY);
		switch( ch ) {
			case 't' :
				strcpy( outbuff, "Task          State  Priority  Stack    #\r\n");
				strcat( outbuff, "************************************************\r\n" );
				vTaskList( outbuff + strlen( outbuff ) );
				strcat( outbuff, "\r\n" );
				fprintf( dbgFile, outbuff );
			break;
#ifdef DEMO_DEBUG
			case 'c' :
				strcpy( outbuff, "Interrupt counters:\r\n");
				sprintf( outbuff + strlen(outbuff), "uartRxInterruptCount   %08x\r\n", uart->RxInterruptCount );
				sprintf( outbuff + strlen(outbuff), "EthernetRxReleaseCount %08x\r\n", EthernetRxReleaseCount );
				sprintf( outbuff + strlen(outbuff), "EthRxTaskCount         %08x\r\n", EthRxTaskCount );
				strcat( outbuff, "\r\n" );
				fprintf( dbgFile, outbuff );
			break;
#endif
			case 'n' : // display ip address
				ip[0] = netif->ip_addr.addr & 0xFF;
				ip[1] = (netif->ip_addr.addr >> 8) & 0xFF;
				ip[2] = (netif->ip_addr.addr >> 16) & 0xFF;
				ip[3] = (netif->ip_addr.addr >> 24) & 0xFF;
				fprintf( dbgFile, " IP ADDR:%d.%d.%d.%d\r\n",
					ip[0],ip[1],ip[2],ip[3] );
				// display network mask
				ip[0] = netif->netmask.addr & 0xFF;
				ip[1] = (netif->netmask.addr >> 8) & 0xFF;
				ip[2] = (netif->netmask.addr >> 16) & 0xFF;
				ip[3] = (netif->netmask.addr >> 24) & 0xFF;
				fprintf( dbgFile, "NET MASK:%d.%d.%d.%d\r\n",
					ip[0],ip[1],ip[2],ip[3] );
				// display gateway
				ip[0] = netif->gw.addr & 0xFF;
				ip[1] = (netif->gw.addr >> 8) & 0xFF;
				ip[2] = (netif->gw.addr >> 16) & 0xFF;
				ip[3] = (netif->gw.addr >> 24) & 0xFF;
				fprintf( dbgFile, " GATEWAY:%d.%d.%d.%d\r\n",
					ip[0],ip[1],ip[2],ip[3] );
			break;
			case 'd' :
				fprintf( dbgFile, "  DHCP State:%s\r\n",
					dhcp_states[netif->dhcp->state]);
				fprintf( dbgFile, "DHCP retires:%d\r\n", netif->dhcp->tries );
			break;
		}
	}
}

void stackOverflowReport( char *pcTaskName )
{
	fprintf( dbgFile, "\r\nSTACK OVERFLOW %s\r\n", pcTaskName );
}

