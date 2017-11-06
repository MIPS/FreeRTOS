/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/


#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include <lwip/stats.h>
#include "netif/etharp.h"

#include "intc.h"
#include "ci40_eth.h"

#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

/* Forward declarations. */
static void  ci40_eth_input(struct netif *netif);
static err_t ci40_eth_output(struct netif *netif, struct pbuf *p,
		struct ip_addr *ipaddr);

static void low_level_init(struct netif *netif)
{

	/* set MAC hardware address length */
	netif->hwaddr_len = 6;

	/* set MAC hardware address */
	netif->hwaddr[0] = 0x00;
	netif->hwaddr[1] = 0x19;
	netif->hwaddr[2] = 0xF5;
	netif->hwaddr[3] = 0xFF;
	netif->hwaddr[4] = 0xFF;
	netif->hwaddr[5] = 0xF0;

	/* maximum transfer unit */
	netif->mtu = 1500;

	/* broadcast capability */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

	/* initialise interface. */
	ci40_net_init( netif->hwaddr );
}

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	struct pbuf *q;

#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

	for(q = p; q != NULL; q = q->next) {
		ci40_net_send(q->payload, q->len);
	}

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

#if LINK_STATS
	lwip_stats.link.xmit++;
#endif /* LINK_STATS */

	return ERR_OK;
}

static struct pbuf *low_level_input(struct netif *netif)
{
	struct pbuf *p, *q;
	uint16_t len;

	len = ci40_net_len();
	if (len == 0) return NULL;

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE;						/* allow room for Ethernet padding */
#endif

	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	if (p != NULL) {

#if ETH_PAD_SIZE
		pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

		/* We iterate over the pbuf chain until we have read the entire
		 * packet into the pbuf. */
		for(q = p; q != NULL; q = q->next) {
			ci40_net_read(q->payload, q->len);
		}

#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif

#if LINK_STATS
		lwip_stats.link.recv++;
#endif /* LINK_STATS */
	} else {
		//drop packet();
#if LINK_STATS
		lwip_stats.link.memerr++;
		lwip_stats.link.drop++;
#endif /* LINK_STATS */
	}

	return p;
}

static err_t ci40_eth_output(struct netif *netif, struct pbuf *p,
		struct ip_addr *ipaddr)
{
	return etharp_output(netif, p, ipaddr);
}

static void ci40_eth_input(struct netif *netif)
{
	struct eth_hdr *ethhdr;
	struct pbuf *p;

	/* move received packet into a new pbuf */
	p = low_level_input(netif);
	/* no packet could be read, silently ignore this */
	if (p == NULL) return;
	/* points to packet payload, which starts with an Ethernet header */
	ethhdr = p->payload;

	switch (htons(ethhdr->type)) {
	/* IP or ARP packet? */
	case ETHTYPE_IP:
	case ETHTYPE_ARP:
#if PPPOE_SUPPORT
		/* PPPoE packet? */
	case ETHTYPE_PPPOEDISC:
	case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
		/* full packet send to tcpip_thread to process */
		if (netif->input(p, netif)!=ERR_OK)
		{
			LWIP_DEBUGF(NETIF_DEBUG, ("ci40_eth_input: IP input error\n"));
			pbuf_free(p);
			p = NULL;
		}
		break;

	default:
		pbuf_free(p);
		p = NULL;
		break;
	}
}

static void ci40_arp_timer(void *arg)
{
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, ci40_arp_timer, NULL);
}

static err_t ci40_init(struct netif *netif)
{
	netif->name[0] = 'e';
	netif->name[1] = '0';
	netif->output = ci40_eth_output;
	netif->linkoutput = low_level_output;
	low_level_init(netif);
	etharp_init();
	sys_timeout(ARP_TMR_INTERVAL, ci40_arp_timer, NULL);
	return ERR_OK;
}

#ifdef DEMO_DEBUG
uint32_t EthRxTaskCount = 0;
uint32_t EthernetRxReleaseCount = 0;
#endif

static void ci40_eth_rx_task( void *pvParameters )
{
	CI40_ETH_T *ci40_eth = (CI40_ETH_T *) pvParameters;
	struct netif *ci40_if = &ci40_eth->netif;

	for( ;; ) {
		if (ci40_net_len( )) {
			ci40_eth_input(ci40_if);
		} else  {
			xSemaphoreTake( ci40_eth->rx_semaphore, portMAX_DELAY );
#ifdef DEMO_DEBUG
			EthRxTaskCount ++;
#endif
		}
	}
}

static void ci40_eth_rx_isr(void* param)
{
	CI40_ETH_T *ci40_eth = (CI40_ETH_T *) param;
	BaseType_t xEthHigherPriorityTaskWoken;
#ifdef DEMO_DEBUG
	EthernetRxReleaseCount ++;
#endif
	ci40_gmac_dma_clear_int();
	xEthHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR( ci40_eth->rx_semaphore, &xEthHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xEthHigherPriorityTaskWoken );
}

void ci40_eth_init(CI40_ETH_T *ci40_eth, HANDLER_DESC_T* irq)
{
	struct ip_addr  xIpAddr, xNetMask, xGateway;
	struct netif *ci40_if = &ci40_eth->netif;

	/* Create and configure the EMAC interface. */
	IP4_ADDR( &xIpAddr,0,0,0,0 );
	IP4_ADDR( &xNetMask,0,0,0,0 );
	IP4_ADDR( &xGateway,0,0,0,0 );
	netif_add( ci40_if, &xIpAddr, &xNetMask, &xGateway, ci40_eth, ci40_init, tcpip_input );

	/* make it the default interface */
	netif_set_default( ci40_if );

	ci40_eth->rx_semaphore = xSemaphoreCreateBinary( );

	irq->function = ci40_eth_rx_isr;
	irq->parameter = ci40_eth;
	intc_RegisterHandler(ci40_eth->intc, irq);


	xTaskCreate( ci40_eth_rx_task, "Eth RX", configNORMAL_STACK_SIZE, ci40_eth, oneTASK_PRIORITY, NULL );
}
