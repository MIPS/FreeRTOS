#ifndef __MIPSFPGAETHIF_H__
#define __MIPSFPGAETHIF_H__

#include "types.h"
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

#define MIPSFPGA_NET_BASE_ADDR 0xB0E00000

typedef struct mipsFPGAethif {
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  uint32_t *regs;
  uint32_t rx_count;
  MIPSFPGA_INTC_T *intc;
  struct netif *netif;
  SemaphoreHandle_t rx_Semaphore;
}MIPSFPGA_ETH_T;

void mipsFPGA_ethInit(struct netif *netif, MIPSFPGA_ETH_T * ethif);

#endif
