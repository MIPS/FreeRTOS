/*
 * A simple demo.
 * Chases a led along the line of leds, speed controlled by some switches
 * Produces a web page of the task states
 * Produces serial output from a simple monitor task   
 */

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "web.h"
#include "mipsFPGA_eth.h"
#include "gpioTask.h"

/* Standard include. */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "lwip/dhcp.h"
#include "lwip/sys.h"

/* Priorities at which the tasks are created. */
#define	zeroTASK_PRIORITY			( tskIDLE_PRIORITY )
#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define	twoTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define	threeTASK_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define NEYX4DDR_UART_CLK 			50000000
#define mainQUEUE_LENGTH			( 8 )


/* MIPSFPGA MEMORY MAP */
#define MIPSFPGA_INTC_BASE		0xB0200000
#define MIPSFPGA_GPIO_BASE		0xB0600000
#define MIPSFPGA_UART_BASE		0xB0401000
#define MIPSFPGA_EHTNET_BASE	0xB0E00000


static MIPSFPGA_INTC_T intc;
static UART16550_T uart;
static MIPSFPGA_ETH_T eth;
static MIPSFPGA_GPIO_T gpio;

static HANDLER_DESC_T ethirq;
static HANDLER_DESC_T uartirq;

typedef struct {
	MIPSFPGA_INTC_T *intc;
	UART16550_T *uart;
	MIPSFPGA_ETH_T *eth;
	FILE *dbgFile;
} MONITOR_INFO_T;

static MONITOR_INFO_T monitor_info;
static WEB_INFO_T web_info;
static struct eth_addr mac_addr[] = { 0x00,0x19,0xF5,0xFF,0xFF,0xF0 };
/*-----------------------------------------------------------*/

/* Tasks */
static void MonitorTask(void *parameters);

/*-----------------------------------------------------------*/

static FILE *dbgFile;

int main(void)
{
	intc.base_addr = MIPSFPGA_INTC_BASE;
	intc_init((INTC_T*)&intc);

	dbgFile = uart16550_fopen(&uart, "w");

	/* create tasks */
	gpio.port_data = (volatile uint32_t*)MIPSFPGA_GPIO_BASE;
	xTaskCreate( gpioTask, ( signed char * ) "gpio Task", 
		configNORMAL_STACK_SIZE, &gpio, oneTASK_PRIORITY, NULL );

	uart.base_addr = MIPSFPGA_UART_BASE;
	uart.regs = (UART16550_REGS_T*)MIPSFPGA_UART_BASE;
	uart.baud_clk = NEYX4DDR_UART_CLK;
	uart.speed = 115200;
	uart.bits = SER_8BITS;
	uart.stop_bits = SER_1STOPBIT;
	uart.parity = SER_NOPARITY;
	uart.intc = (INTC_T*)&intc;
	uart.ReadySemaphore = xSemaphoreCreateBinary( );

	uartirq.int_num=6;
	uartirq.ext_num=0;

	uart16550_init(&uart, &uartirq);

	monitor_info.intc = &intc;
	monitor_info.uart = &uart;
	monitor_info.eth = &eth;
	monitor_info.dbgFile = dbgFile;
	xTaskCreate( MonitorTask, ( signed char * ) "Monitor Task", 
		configNORMAL_STACK_SIZE, &monitor_info, oneTASK_PRIORITY, NULL );


	eth.intc = &intc;
	eth.regs = (uint32_t*)MIPSFPGA_EHTNET_BASE;
	eth.ethaddr = mac_addr;

	ethirq.int_num=6;
	ethirq.ext_num=1;

	mipsFPGA_ethInit(&eth,&ethirq);

	web_info.uart = &uart;
	web_info.netif = &eth.netif;
	web_info.dbgFile = dbgFile;
	sys_thread_new( "Web server", vBasicWEBServer, ( void * ) &web_info, 
		configNORMAL_STACK_SIZE, oneTASK_PRIORITY+1 );

	/* Start the tasks running. */
	vTaskStartScheduler();

	return 0;
}

/*-----------------------------------------------------------*/

void vAssertCalled( const char *pcFileName, unsigned long ulLine )
{
	fprintf( dbgFile, "Guru meditation in %s:%d\n", pcFileName, ulLine);
	for (;;);
}

static void MonitorTask(void *parameters)
{
	MONITOR_INFO_T *monitor_info = (MONITOR_INFO_T *)parameters;
	UART16550_T *uart = monitor_info->uart;
	MIPSFPGA_ETH_T *eth = monitor_info->eth;
	FILE *dbgFile = monitor_info->dbgFile; 
	struct netif *netif = &eth->netif;
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
		fprintf( dbgFile, "\r\n" );

		// block on rx queue
		xQueueReceive( uart->RxQueue, &ch, portMAX_DELAY);
		switch( ch ) {
			case 't' :
				fprintf( dbgFile, 
					"Task          State  Priority  Stack    #\r\n");
				fprintf( dbgFile, 
					"************************************************\r\n" );
				vTaskList( outbuff );
				fprintf( dbgFile, "%s\r\n", outbuff );
			break;
#ifdef DEMO_DEBUG
			case 'c' :
				fprintf( dbgFile, "Interrupt counters:\r\n");
				fprintf( dbgFile, "uart RxInterruptCount   %08x\r\n", 
					uart->RxInterruptCount );
				fprintf( dbgFile, "Ethernet rx count       %08x\r\n\n", 
					eth->rx_count );
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

