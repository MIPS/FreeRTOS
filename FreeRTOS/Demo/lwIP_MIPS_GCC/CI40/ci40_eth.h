#ifndef __CI40_ETH_H__
#define __CI40_ETH_H__

typedef struct {
	struct netif netif;
	SemaphoreHandle_t rx_semaphore;
	INTC_T *intc;
}CI40_ETH_T;

void ci40_eth_init(CI40_ETH_T *ci40_eth, HANDLER_DESC_T* irq);

#endif
