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
#include "mipsFPGA_eth.h"
#include "mipsFPGA_intc.h"

#define ETH_TX_DATA		(0x0000 / 4)
#define ETH_RX_DATA		(0x1000 / 4)
#define MDIOADDR		(0x07E4 / 4)
#define MDIOWR			(0x07E8 / 4)
#define MDIORD			(0x07EC / 4)
#define MDIOCTRL		(0x07F0 / 4)
#define TX_PING_LEN		(0x07F4 / 4)
#define	GIE				(0x07F8 / 4)
#define TX_PING_CNTL	(0x07FC / 4)
#define TX_PONG_LEN		(0x0FF4 / 4)
#define TX_PONG_CNTL	(0x0FFC / 4)
#define RX_PING_CNTL	(0x17FC / 4)
#define RX_PONG_CNTL	(0x1FFC / 4)

#define GIE_ENABLE		0x80000000
#define RX_INT_ENABLE	0x00000008
#define TX_ENABLE		0x00000001
#define TX_BUSY			0x00000001
#define RX_READY		0x00000001
#define MAC_SET			0x00000003


/* Define those to better describe your network interface. */
#define IFNAME0 'e'
#define IFNAME1 'n'

/* Forward declarations. */
static void  mipsFPGA_eth_input(struct netif *netif);
static err_t mipsFPGA_eth_output(struct netif *netif, struct pbuf *p,
             struct ip_addr *ipaddr);
void mipsFPGA_eth_Rx(void *parameter);

static void low_level_init(struct netif *netif)
{
	MIPSFPGA_ETH_T *eth = netif->state;
	HANDLER_DESC_T handler_info;

	/* set MAC hardware address length */
	netif->hwaddr_len = 6;

	/* set MAC hardware address */
	netif->hwaddr[0] = eth->ethaddr->addr[0];
	netif->hwaddr[1] = eth->ethaddr->addr[1];
	netif->hwaddr[2] = eth->ethaddr->addr[2];
	netif->hwaddr[3] = eth->ethaddr->addr[3];
	netif->hwaddr[4] = eth->ethaddr->addr[4];
	netif->hwaddr[5] = eth->ethaddr->addr[5];

	/* maximum transfer unit */
	netif->mtu = 1500;
  
	/* broadcast capability */
	netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;
 
	/* Do whatever else is needed to initialize interface. */  
	memcpy((void*)&eth->regs[ETH_TX_DATA],netif->hwaddr,6);
	eth->regs[TX_PING_CNTL] |= MAC_SET;

	handler_info.mask = AIX_INTC_INT1;
	handler_info.function = mipsFPGA_eth_Rx;
	handler_info.parameter = netif;
	intc_RegisterHandler(eth->intc, &handler_info);
	eth->regs[RX_PING_CNTL] &= ~RX_READY;
	eth->regs[RX_PING_CNTL] = RX_INT_ENABLE;
	eth->regs[GIE] |= GIE_ENABLE; 
}

/*
 * low_level_output():
 *
 * Should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 */

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
	MIPSFPGA_ETH_T *eth = netif->state;
	struct pbuf *q;
	unsigned char *ptr;
	int len = 0;

	//initiate transfer();
	while (eth->regs[TX_PING_CNTL] & TX_BUSY)
		;
  
#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE);			/* drop the padding word */
#endif

	ptr = (u8*)&eth->regs[ETH_TX_DATA];
	for(q = p; q != NULL; q = q->next) {
		/* Send the data from the pbuf to the interface, one pbuf at a
			time. The size of the data in each pbuf is kept in the ->len
			variable. */
		//send data from(q->payload, q->len);
		memcpy(ptr,q->payload,q->len);
		ptr += q->len;
		len += q->len;
	}
	eth->regs[TX_PING_LEN] = len;

	//signal that packet should be sent();
	eth->regs[TX_PING_CNTL] |= TX_ENABLE; 

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

static struct pbuf *low_level_input(struct netif *netif)
{
	MIPSFPGA_ETH_T *eth = netif->state;
	struct pbuf *p, *q;
	u16_t len;
	unsigned char *ptr;

	/* Obtain the size of the packet and put it into the "len"
     variable. */
	if (eth->regs[RX_PING_CNTL] & RX_READY) {
		len = htons(eth->regs[ETH_RX_DATA + 3] & 0xFFFF);
		if (len > 0x600)
			len = 0x500;
	} else {
		len = 0;
	}
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

		ptr = (u8*)&eth->regs[ETH_RX_DATA]; 
		/* We iterate over the pbuf chain until we have read the entire
			* packet into the pbuf. */
		for(q = p; q != NULL; q = q->next) {
			/* Read enough bytes to fill this pbuf in the chain. The
			* available data in the pbuf is given by the q->len
			* variable. */
			//read data into(q->payload, q->len);
			memcpy(q->payload,ptr,q->len);
			ptr += q->len;
		}
		//acknowledge that packet has been read();
		eth->regs[RX_PING_CNTL] &= ~RX_READY;

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
 * mipsFPGAethif_output():
 *
 * This function is called by the TCP/IP stack when an IP packet
 * should be sent. It calls the function called low_level_output() to
 * do the actual transmission of the packet.
 *
 */

static err_t mipsFPGA_eth_output(struct netif *netif, struct pbuf *p,
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
 * @param netif the lwip network interface structure for this mipsFPGAethif
 */
static void mipsFPGA_eth_input(struct netif *netif)
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
			if (netif->input(p, netif)!=ERR_OK) { 
				LWIP_DEBUGF(NETIF_DEBUG, ("mipsFPGAethif_input: IP input error\n"));
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

static void arp_timer(void *arg)
{
	etharp_tmr();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
}

void mipsFPGAethRxTask(void *parameters)
{
	struct netif *netif = (struct netif *)parameters;
	MIPSFPGA_ETH_T *eth = netif->state;
	for (;;) {
		xSemaphoreTake( eth->rx_Semaphore, 0 );
		mipsFPGA_eth_input(netif);
		eth->regs[RX_PING_CNTL] &= ~RX_READY;
	}
}

/*
 * mipsFPGAethif_init():
 *
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 */

err_t mipsFPGA_eth_init(struct netif *netif)
{
	netif->name[0] = IFNAME0;
	netif->name[1] = IFNAME1;
	netif->output = mipsFPGA_eth_output;
	netif->linkoutput = low_level_output;
	low_level_init(netif);
	etharp_init();
	sys_timeout(ARP_TMR_INTERVAL, arp_timer, NULL);
	return ERR_OK;
}

void mipsFPGA_eth_Rx(void *parameter)
{
	BaseType_t xHigherPriorityTaskWoken;
	struct netif *netif = (struct netif*)parameter;
	MIPSFPGA_ETH_T *eth = (MIPSFPGA_ETH_T*) netif->state;
	eth->rx_count++;
    xHigherPriorityTaskWoken = pdFALSE;
    xSemaphoreGiveFromISR( eth->rx_Semaphore, &xHigherPriorityTaskWoken );
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void mipsFPGA_ethInit(struct netif *netif, MIPSFPGA_ETH_T * eth)
{
	static struct ip_addr addr, mask, gw;
	IP4_ADDR(&addr, 0, 0, 0, 0);
	IP4_ADDR(&mask, 0, 0, 0, 0);
	IP4_ADDR(&gw, 0, 0, 0, 0);
	netif_add(netif, &addr, &mask, &gw, eth, mipsFPGA_eth_init,
		  tcpip_input);
	/* make it the default interface */
	netif_set_default(netif);

	eth->rx_Semaphore = xSemaphoreCreateBinary( );

	xTaskCreate( mipsFPGAethRxTask, ( signed char * ) "Eth Rx Task", configNORMAL_STACK_SIZE, netif, tskIDLE_PRIORITY + 1, NULL );
}
