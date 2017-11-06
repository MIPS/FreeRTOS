/*
    FreeRTOS V4.6.1 - copyright (C) 2003-2006 Richard Barry.

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License** as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    FreeRTOS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeRTOS; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    A special exception to the GPL can be applied should you wish to distribute
    a combined work that includes FreeRTOS, without being obliged to provide
    the source code for any proprietary components.  See the licensing section
    of http://www.FreeRTOS.org for full details of how and when the exception
    can be applied.

 ***************************************************************************
 ***************************************************************************
 *                                                                         *
 * Get the FreeRTOS eBook!  See http://www.FreeRTOS.org/Documentation      *
 *                                                                         *
 * This is a concise, step by step, 'hands on' guide that describes both   *
 * general multitasking concepts and FreeRTOS specifics. It presents and   *
 * explains numerous examples that are written using the FreeRTOS API.     *
 * Full source code for all the examples is provided in an accompanying    *
 * .zip file.                                                              *
 *                                                                         *
 ***************************************************************************
 ***************************************************************************

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
 */

/*
    Implements a simplistic WEB server.  Every time a connection is made and
    data is received a dynamic page that shows the current TCP/IP statistics
    is generated and returned.  The connection is then closed.

    This file was adapted from a FreeRTOS lwIP slip demo supplied by a third
    party.
 */

/* ------------------------ System includes ------------------------------- */
#include <stdio.h>
#include <string.h>

/* ------------------------ FreeRTOS includes ----------------------------- */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* ------------------------ lwIP includes --------------------------------- */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/udp.h"
#include "lwip/tcp_impl.h"
#include "lwip/dhcp.h"
#include "lwip/timers.h"
//#include "netif/loopif.h"

/* ------------------------ Project includes ------------------------------ */
#include "web.h"
/* ------------------------ Defines --------------------------------------- */
/* The size of the buffer in which the dynamic WEB page is created. */
#define webMAX_PAGE_SIZE        ( 2048 )

/* Standard GET response. */
#define webHTTP_OK  "HTTP/1.0 200 OK\r\nContent-type: text/html\r\n\r\n"

/* The port on which we listen. */
#define webHTTP_PORT            ( 80 )

/* Delay on close error. */
#define webSHORT_DELAY          ( 10 )

#define mainTASK_PRIORITY           ( tskIDLE_PRIORITY + 1 )


/* Format of the dynamic page that is returned on each connection. */
#define webHTML_START \
		"<html>\
		<head>\
		</head>\
		<BODY onLoad=\"window.setTimeout(&quot;location.href='index.html'&quot;,1000)\"bgcolor=\"#CCCCff\">\
		\r\nPage Hits = "

#define webHTML_END \
		"\r\n" \
		"FreeRTOS MCF5235 port (c) 2006 by Christian Walter &lt;wolti@sil.at&gt;\r\n" \
		"Adapted for MIPS (c) 2015 by Imagination Technologies Limited &lt;www.imgtec.com&gt;\r\n" \
		"</pre>\r\n" \
		"</BODY>\r\n" \
		"</html>"

/* ------------------------ Prototypes ------------------------------------ */
static void     vProcessConnection( struct netconn *pxNetCon );

/*------------------------------------------------------------*/

struct ip_addr  xIpAddr, xNetMask, xGateway;


/*
 * Process an incoming connection on port 80.
 *
 * This simply checks to see if the incoming data contains a GET request, and
 * if so sends back a single dynamically created page.  The connection is then
 * closed.  A more complete implementation could create a task for each
 * connection.
 */
static void
vProcessConnection( struct netconn *pxNetCon )
{
	static char cDynamicPage[webMAX_PAGE_SIZE], cPageHits[11];
	struct netbuf  *pxRxBuffer;
	char       *pcRxString;
	unsigned short usLength;
	static unsigned long ulPageHits = 0;
	err_t error;

	mips_flush_dcache( );
	/* We expect to immediately get data. */
	error = netconn_recv( pxNetCon, &pxRxBuffer );

	if( pxRxBuffer != NULL )
	{
		/* Where is the data? */
		netbuf_data( pxRxBuffer, ( void * )&pcRxString, &usLength );

		/* Is this a GET?  We don't handle anything else. */
		if( !strncmp( pcRxString, "GET", 3 ) )
		{
			pcRxString = cDynamicPage;

			/* Update the hit count. */
			ulPageHits++;
			sprintf( cPageHits, "%lu", ulPageHits );

			/* Write out the HTTP OK header. */
			netconn_write( pxNetCon, webHTTP_OK, ( u16_t ) strlen( webHTTP_OK ),
					NETCONN_COPY );

			/* Generate the dynamic page...

				... First the page header. */
			strcpy( cDynamicPage, webHTML_START );
			/* ... Then the hit count... */
			strcat( cDynamicPage, cPageHits );
			strcat( cDynamicPage,
					"<p><pre>Task          State  Priority  Stack #<br>");
			strcat( cDynamicPage,
					"************************************************<br>" );
			/* ... Then the list of tasks and their status... */
			vTaskList( cDynamicPage + strlen( cDynamicPage ) );
			/* ... Finally the page footer. */
			strcat( cDynamicPage, webHTML_END );

			/* Write out the dynamically generated page. */
			netconn_write( pxNetCon, cDynamicPage, ( u16_t )
					strlen( cDynamicPage ), NETCONN_COPY );
		}

		netbuf_delete( pxRxBuffer );
	}

	netconn_close( pxNetCon );
}

void vBasicWEBServer( void *pvParameters )
{
	WEB_INFO_T *web_info = (WEB_INFO_T*) pvParameters;
	UART16550_T *uart = web_info->uart;
	struct netconn *pxHTTPListener, *pxNewConnection;
	err_t error;
	FILE *dbgFile = web_info->dbgFile;

	/* Initialize lwIP and its interface layer. */
	tcpip_init( NULL, NULL );

	/* Create a new tcp connection handle */
	pxHTTPListener = netconn_new( NETCONN_TCP );
	netconn_bind( pxHTTPListener, NULL, webHTTP_PORT );
	netconn_listen( pxHTTPListener );

	/* Specify host name, and start DHCP */
	netif_set_hostname(web_info->netif, "mips_websever_demo");
	dhcp_start( web_info->netif );

	while ( web_info->netif->ip_addr.addr == 0L ) {
		uint32_t mscnt = 0;
		sys_msleep(DHCP_FINE_TIMER_MSECS);
		dhcp_fine_tmr();
		mscnt += DHCP_FINE_TIMER_MSECS;
		if (mscnt >= DHCP_COARSE_TIMER_SECS*1000) {
			dhcp_coarse_tmr();
			mscnt = 0;
		}
		vTaskDelay( DHCP_FINE_TIMER_MSECS );
	}

	/* Display acquired IP address */
	{
		static uint8_t ip[4];
		ip[0] = web_info->netif->ip_addr.addr & 0xFF;
		ip[1] = (web_info->netif->ip_addr.addr >> 8) & 0xFF;
		ip[2] = (web_info->netif->ip_addr.addr >> 16) & 0xFF;
		ip[3] = (web_info->netif->ip_addr.addr >> 24) & 0xFF;
		fprintf(dbgFile,"IP:%d.%d.%d.%d\r\n",ip[0],ip[1],ip[2],ip[3]);
	}

	/* Loop forever */
	for( ;; )
	{
		/* Wait for connection. */
		error = netconn_accept( pxHTTPListener, &pxNewConnection );
		mips_flush_dcache( );

		if( error == ERR_OK )
		{
			/* Service connection. */
			vProcessConnection( pxNewConnection );
			while( netconn_delete( pxNewConnection ) != ERR_OK )
			{
				vTaskDelay( webSHORT_DELAY );
			}
		}
	}
}
