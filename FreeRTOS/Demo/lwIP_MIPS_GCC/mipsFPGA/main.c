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
#include "mipsFPGA_uart.h"
#include "web.h"
#include "mipsFPGA_eth.h"
#include "gpioTask.h"

/* Standard include. */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "lwip/dhcp.h"

/* Priorities at which the tasks are created. */
#define	zeroTASK_PRIORITY			( tskIDLE_PRIORITY )
#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define	twoTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define	threeTASK_PRIORITY			( tskIDLE_PRIORITY + 3 )
#define NEYX4DDR_BAUD_CLK 50000000 / 16
#define mainQUEUE_LENGTH					( 8 )

static MIPSFPGA_INTC_T intc;
static MIPSFPGA_UART_T uart;
static MIPSFPGA_ETH_T eth;
static MIPSFPGA_GPIO_T gpio;

typedef struct {
	MIPSFPGA_INTC_T *intc;
	MIPSFPGA_UART_T *uart;
	MIPSFPGA_ETH_T *eth;
	FILE *dbgFile;
} MONITOR_INFO_T;
MONITOR_INFO_T monitor_info;
WEB_INFO_T web_info;
struct eth_addr mac_addr[] = { 0x00,0x19,0xF5,0xFF,0xFF,0xF0 };
/*-----------------------------------------------------------*/

/* Tasks */
static void MonitorTask(void *parameters);

/*-----------------------------------------------------------*/

/* Semaphores */
extern SemaphoreHandle_t UartReadySemaphore;

/*-----------------------------------------------------------*/

FILE *dbgFile;

int main(void)
{
	intc.base_addr = MIPSFPGA_INTC_ADDR;
	intc_Init( &intc );

	dbgFile = mipsUART_fopen(&uart, "w");

	/* create tasks */
	gpio.port_data = (uint32_t*)GPIO_BASE_ADDR;
	xTaskCreate( gpioTask, ( signed char * ) "gpio Task", 
		configNORMAL_STACK_SIZE, &gpio, oneTASK_PRIORITY, NULL );

	uart.base_addr = UART_BASE_ADDR;
	uart.regs = (MIPSFPGA_UART_REGS_T*)UART_BASE_ADDR;
	uart.baud_clk = NEYX4DDR_BAUD_CLK;
	uart.speed = 9600;
	uart.bits = SER_8BITS;
	uart.stop_bits = SER_1STOPBIT;
	uart.parity = SER_NOPARITY;
	uart.intc = &intc;
	uart.ReadySemaphore = xSemaphoreCreateBinary( );
	xTaskCreate( uartTask, ( signed char * ) "uart Task", 
		configNORMAL_STACK_SIZE, &uart, oneTASK_PRIORITY, NULL );

    monitor_info.intc = &intc;
	monitor_info.uart = &uart;
	monitor_info.eth = &eth;
	monitor_info.dbgFile = dbgFile;
	xTaskCreate( MonitorTask, ( signed char * ) "Monitor Task", 
		configNORMAL_STACK_SIZE, &monitor_info, oneTASK_PRIORITY, NULL );

	eth.intc = &intc;
	eth.regs = (uint32_t*)MIPSFPGA_NET_BASE_ADDR;
	eth.ethaddr = mac_addr;
	web_info.uart = &uart;
	web_info.eth = &eth;
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
	MIPSFPGA_UART_T *uart = monitor_info->uart;
	MIPSFPGA_ETH_T *eth = monitor_info->eth;
	FILE *dbgFile = monitor_info->dbgFile; 
	struct netif *netif = eth->netif;
	static u8 outbuff[1024];
	u8 ch;
	u8 ip[4] = { 0, 0, 0, 0 };
	u8 *dhcp_states[] = {
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

