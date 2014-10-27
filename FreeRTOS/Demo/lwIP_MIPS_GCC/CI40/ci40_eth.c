/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */

#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/tcpip.h"
#include <lwip/stats.h>
#include "netif/etharp.h"

/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

#define	oneTASK_PRIORITY			( tskIDLE_PRIORITY + 1 )

SemaphoreHandle_t EthernetRxSemaphore;

/* Forward declarations. */
static void  ci40_eth_input(struct netif *netif);
static err_t ci40_eth_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);

static void
low_level_init(struct netif *netif)
{
  
  /* set MAC hardware address length */
  netif->hwaddr_len = 6;

  /* set MAC hardware address */
  // 0x00,0x19,0xF5,0xFF,0xFF,0xF0
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x19;
  netif->hwaddr[2] = 0xF5;
  netif->hwaddr[3] = 0xFF;
  netif->hwaddr[4] = 0xFF;
  netif->hwaddr[5] = 0xF0;

  /* maximum transfer unit */
  netif->mtu = 1500;
  
  /* broadcast capability */
//  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
 
  /* Do whatever else is needed to initialize interface. */  
  ci40_net_init( netif->hwaddr );
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
  struct pbuf *q;

  //initiate transfer();
  
#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

  for(q = p; q != NULL; q = q->next) {
    /* Send the data from the pbuf to the interface, one pbuf at a
       time. The size of the data in each pbuf is kept in the ->len
       variable. */
    //send data from(q->payload, q->len);
	ci40_net_send(q->payload, q->len);
  }

  //signal that packet should be sent();

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE);			/* reclaim the padding word */
#endif
  
#if LINK_STATS
  lwip_stats.link.xmit++;
#endif /* LINK_STATS */      

  return ERR_OK;
}

/*
 * low_level_input():
 *
 * Should allocate a pbuf and transfer the bytes of the incoming
 * packet from the interface into the pbuf.
 *
 */

static struct pbuf *
low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  u16_t len;

  /* Obtain the size of the packet and put it into the "len"
     variable. */
  len = ci40_net_len();
  if (len == 0) return NULL;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE;						/* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  
  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable. */
      //read data into(q->payload, q->len);
      ci40_net_read(q->payload, q->len);
    }
    //acknowledge that packet has been read();

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

/*
 * ethernetif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t
ci40_eth_output(struct netif *netif, struct pbuf *p,
      struct ip_addr *ipaddr)
{
  
 /* resolve hardware address, then send (or queue) packet */
  return etharp_output(netif, p, ipaddr);
 
}


/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
static void
ci40_eth_input(struct netif *netif)
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
     { LWIP_DEBUGF(NETIF_DEBUG, ("ci40_eth_input: IP input error\n"));
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

static void
arp_timer(void *arg)
{
  etharp_tmr();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

/*
 * ethernetif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t
ci40_eth_init(struct netif *netif)
{
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  netif->output = ci40_eth_output;
  netif->linkoutput = low_level_output;
  low_level_init(netif);
  etharp_init();
  sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
  return ERR_OK;
}

void ci40_net_input(struct netif *netif)
{
	ci40_eth_input(netif);
}

#ifdef DEMO_DEBUG
uint32_t EthRxTaskCount = 0;
uint32_t EthernetRxReleaseCount = 0;
#endif

void EthRxTask( void *pvParameters )
{
	struct netif *ci40_if = (struct netif*) pvParameters;

	for( ;; ) {
		if (ci40_net_len( )) {
			ci40_net_input(ci40_if);
		} else  {
			xSemaphoreTake( EthernetRxSemaphore, portMAX_DELAY );
#ifdef DEMO_DEBUG
			EthRxTaskCount ++;
#endif
		}
	}
}


void EthernetRxRelease(void)
{
	static BaseType_t xEthHigherPriorityTaskWoken;
#ifdef DEMO_DEBUG
	EthernetRxReleaseCount ++;
#endif
	ci40_gmac_dma_clear_int();
	xEthHigherPriorityTaskWoken = pdFALSE;
	xSemaphoreGiveFromISR( EthernetRxSemaphore, &xEthHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xEthHigherPriorityTaskWoken );
}


void CI40_ethInit(struct netif *ci40_if)
{
	struct ip_addr  xIpAddr, xNetMask, xGateway;

    /* Create and configure the EMAC interface. */
    IP4_ADDR( &xIpAddr,0,0,0,0 );
    IP4_ADDR( &xNetMask,0,0,0,0 );
    IP4_ADDR( &xGateway,0,0,0,0 );
    netif_add( ci40_if, &xIpAddr, &xNetMask, &xGateway, NULL, ci40_eth_init, tcpip_input );

    /* make it the default interface */
    netif_set_default( ci40_if );

	EthernetRxSemaphore = xSemaphoreCreateBinary( );

	xTaskCreate( EthRxTask, "Eth RX", configNORMAL_STACK_SIZE, ci40_if, oneTASK_PRIORITY, NULL );
}
