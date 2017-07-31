#ifndef __CI40_ETH_H__
#define __CI40_ETH_H__

void EthRxTask( void *pvParameters );
void CI40_ethInit(struct netif *ci40_if);

#endif
