/*
 * A simple demo.
*/

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "ci40_uart.h"
#include "web.h"
#include "ci40_net.h"
#include "ci40_eth.h"

/* lwIP includes. */
#include "lwip/dhcp.h"

/* Standard include. */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

/* Priorities at which the tasks are created. */
#define	zeroTASK_PRIORITY			( tskIDLE_PRIORITY )
#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )
#define	twoTASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define	threeTASK_PRIORITY			( tskIDLE_PRIORITY + 3 )

/* The number of items the queue can hold.  This is 1 as the receive task
will remove items as they are added, meaning the send task should always find
the queue empty. */
#define mainQUEUE_LENGTH					( 8 )

/*-----------------------------------------------------------*/

/* Tasks */
void vuIP_TASK( void *pvParameters );
static void MonitorTask(void *parameters);

/*-----------------------------------------------------------*/

/* Type definitions */
typedef struct {
	CI40_UART_T *uart;
	struct netif *ci40_if;
	FILE *dbgFile;
} MONITOR_INFO_T;

/* Variables */
struct netif ci40_if;
CI40_UART_T uart;
FILE *dbgFile;
MONITOR_INFO_T monitor_info;
WEB_INFO_T web_info;
/*-----------------------------------------------------------*/

int main(void)
{
	dbgFile = mipsUART_fopen(&uart, "w");

	uart.base_addr = UART_BASE_ADDR;
	uart.regs = (CI40_UART_REGS_T*)UART_BASE_ADDR;
	uart.baud_clk = UART_CLK_FREQ / 16;
	uart.speed = 115200;
	uart.bits = SER_8BITS;
	uart.stop_bits = SER_1STOPBIT;
	uart.parity = SER_NOPARITY;
	uart.ReadySemaphore = xSemaphoreCreateBinary( );
	CI40_uart_init( &uart );
	vlwIPInit();

	/* create tasks */
	xTaskCreate( uartTask, ( signed char * ) "uart Task", configNORMAL_STACK_SIZE, &uart, oneTASK_PRIORITY, NULL );
	
	CI40_ethInit(&ci40_if);

	monitor_info.uart = &uart;
	monitor_info.dbgFile = dbgFile;
	monitor_info.ci40_if = &ci40_if;
	xTaskCreate( MonitorTask, ( signed char * ) "Monitor Task", configNORMAL_STACK_SIZE, &monitor_info, oneTASK_PRIORITY, NULL );

	web_info.uart = &uart;
	web_info.dbgFile = dbgFile;
	web_info.ci40_if = &ci40_if;
	sys_thread_new( "Web server", vBasicWEBServer, &web_info, configNORMAL_STACK_SIZE, oneTASK_PRIORITY+1 );

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
	CI40_UART_T *uart = monitor_info->uart;
	FILE *dbgFile = monitor_info->dbgFile; 
	struct netif *netif = monitor_info->ci40_if;
#ifdef DEMO_DEBUG
	extern u32 EthernetRxReleaseCount;
	extern u32 EthRxTaskCount;
#endif
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

