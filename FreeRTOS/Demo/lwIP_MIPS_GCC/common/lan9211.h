#ifndef __LAN9211_H__
#define __LAN9211_H__

#include "lwip/netif.h"

#include "intc.h"

typedef struct LAN9211_tag {
	volatile uint32_t*	regs;
	uint8_t backupMac[6];
	struct netif netif;
	struct eth_addr *ethaddr;
	INTC_T *intc;
	uint32_t rx_count;
}LAN9211_T;

void LAN9211_ethInit(LAN9211_T *lan9211, HANDLER_DESC_T* irq);

#endif
