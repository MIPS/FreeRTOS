#ifndef __MIPSFPGAETHIF_H__
#define __MIPSFPGAETHIF_H__

/* ------------------------ lwIP includes --------------------------------- */
#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "lwip/stats.h"
#include "lwip/debug.h"
#include "netif/etharp.h"
#include "mipsFPGA_intc.h"


#define MAC_ADDR 0x00,0x19,0xF5,0xFF,0xFF,0xF0
#define MAC_ADDR_HI 0x0019
#define MAC_ADDR_LO 0xF5FFFFFE

typedef struct mipsFPGAethif {
	struct eth_addr *ethaddr;
	/* Add whatever per-interface state that is needed here. */
	uint32_t *regs;
	uint32_t rx_count;
	MIPSFPGA_INTC_T *intc;
	struct netif netif;
	SemaphoreHandle_t rx_Semaphore;
}MIPSFPGA_ETH_T;

void mipsFPGA_ethInit(MIPSFPGA_ETH_T * ethif, HANDLER_DESC_T* irq);

#endif
