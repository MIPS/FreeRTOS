#ifndef __CI40_NET_H__
#define __CI40_NET_H__

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


#define ETH_INT_NUM		50

#define MAC_ADDR 0x00,0x19,0xF5,0xFF,0xFF,0xF0
#define MAC_ADDR_HI 0x0019
#define MAC_ADDR_LO 0xF5FFFFFE

#define IP_ADDR 192,168,154,21

#define DANUBE_NET_BASE_ADDR 0xB8140000
//#define DANUBE_NET_BASE_ADDR 0x18140000

/* Flow Control defines */
#define FLOW_OFF        0
#define FLOW_RX         1
#define FLOW_TX         2
#define FLOW_AUTO       (FLOW_TX | FLOW_RX)

/* Wake-On-Lan options. */
#define WAKE_PHY                (1 << 0)
#define WAKE_UCAST              (1 << 1)
#define WAKE_MCAST              (1 << 2)
#define WAKE_BCAST              (1 << 3)
#define WAKE_ARP                (1 << 4)
#define WAKE_MAGIC              (1 << 5)
#define WAKE_MAGICSECURE        (1 << 6) /* only meaningful if WAKE_MAGIC */

/* GMAC TX FIFO is 8K, Rx FIFO is 16K */
#define BUF_SIZE_16KiB 16384
#define BUF_SIZE_8KiB 8192
#define BUF_SIZE_4KiB 4096
#define BUF_SIZE_2KiB 2048

 /* *********************************************
    DMA CRS Control and Status Register Mapping
  * *********************************************/
 #define DMA_BUS_MODE            0x00001000      /* Bus Mode */
 #define DMA_XMT_POLL_DEMAND     0x00001004      /* Transmit Poll Demand */
 #define DMA_RCV_POLL_DEMAND     0x00001008      /* Received Poll Demand */
 #define DMA_RCV_BASE_ADDR       0x0000100c      /* Receive List Base */
 #define DMA_TX_BASE_ADDR        0x00001010      /* Transmit List Base */
 #define DMA_STATUS              0x00001014      /* Status Register */
 #define DMA_CONTROL             0x00001018      /* Ctrl (Operational Mode) */
 #define DMA_INTR_ENA            0x0000101c      /* Interrupt Enable */
 #define DMA_MISSED_FRAME_CTR    0x00001020      /* Missed Frame Counter */
 #define DMA_CUR_TX_BUF_ADDR     0x00001050      /* Current Host Tx Buffer */
 #define DMA_CUR_RX_BUF_ADDR     0x00001054      /* Current Host Rx Buffer */



#define GMAC_CONTROL		0x00000000	/* Configuration */
#define GMAC_FRAME_FILTER	0x00000004	/* Frame Filter */
#define GMAC_HASH_HIGH		0x00000008	/* Multicast Hash Table High */
#define GMAC_HASH_LOW		0x0000000c	/* Multicast Hash Table Low */
#define GMAC_MII_ADDR		0x00000010	/* MII Address */
#define GMAC_MII_DATA		0x00000014	/* MII Data */
#define GMAC_FLOW_CTRL		0x00000018	/* Flow Control */
#define GMAC_VLAN_TAG		0x0000001c	/* VLAN Tag */
#define GMAC_VERSION		0x00000020	/* GMAC CORE Version */
#define GMAC_WAKEUP_FILTER	0x00000028	/* Wake-up Frame Filter */

/* ********************************
   DMA Control register defines
 * ********************************/
#define DMA_CONTROL_ST          0x00002000      /* Start/Stop Transmission */
#define DMA_CONTROL_SR          0x00000002      /* Start/Stop Receive */

/* **************************************
   DMA Interrupt Enable register defines
 * **************************************/
/**** NORMAL INTERRUPT ****/
#define DMA_INTR_ENA_NIE 0x00010000     /* Normal Summary */
#define DMA_INTR_ENA_TIE 0x00000001     /* Transmit Interrupt */
#define DMA_INTR_ENA_TUE 0x00000004     /* Transmit Buffer Unavailable */
#define DMA_INTR_ENA_RIE 0x00000040     /* Receive Interrupt */
#define DMA_INTR_ENA_ERE 0x00004000     /* Early Receive */
 
#define DMA_INTR_NORMAL (DMA_INTR_ENA_NIE | DMA_INTR_ENA_RIE | \
                        DMA_INTR_ENA_TIE)

/**** ABNORMAL INTERRUPT ****/
#define DMA_INTR_ENA_AIE 0x00008000     /* Abnormal Summary */
#define DMA_INTR_ENA_FBE 0x00002000     /* Fatal Bus Error */
#define DMA_INTR_ENA_ETE 0x00000400     /* Early Transmit */
#define DMA_INTR_ENA_RWE 0x00000200     /* Receive Watchdog */
#define DMA_INTR_ENA_RSE 0x00000100     /* Receive Stopped */
#define DMA_INTR_ENA_RUE 0x00000080     /* Receive Buffer Unavailable */
#define DMA_INTR_ENA_UNE 0x00000020     /* Tx Underflow */
#define DMA_INTR_ENA_OVE 0x00000010     /* Receive Overflow */
#define DMA_INTR_ENA_TJE 0x00000008     /* Transmit Jabber */
#define DMA_INTR_ENA_TSE 0x00000002     /* Transmit Stopped */

#define DMA_INTR_ABNORMAL       (DMA_INTR_ENA_AIE | DMA_INTR_ENA_FBE | \
                                DMA_INTR_ENA_UNE)

/* DMA default interrupt mask */
#define DMA_INTR_DEFAULT_MASK   (DMA_INTR_NORMAL | DMA_INTR_ABNORMAL)

#define SF_DMA_MODE 			1			/* DMA STORE-AND-FORWARD Operation Mode */

#define GMAC_INT_STATUS		0x00000038	/* interrupt status register */

#define GMAC_INT_MASK		0x0000003c	/* interrupt mask register */
/* GMAC HW ADDR regs */
#define GMAC_ADDR_HIGH(reg)		(0x00000040+(reg * 8))
#define GMAC_ADDR_LOW(reg)		(0x00000044+(reg * 8))
#define GMAC_MAX_UNICAST_ADDRESSES	16

#define GMAC_AN_CTRL	0x000000c0	/* AN control */
#define GMAC_AN_CTRL_ANE 0x00001000 /* Auto negotiate */

#define GMAC_AN_STATUS	0x000000c4	/* AN status */
#define GMAC_ANE_ADV	0x000000c8	/* Auto-Neg. Advertisement */
#define GMAC_ANE_LINK	0x000000cc	/* Auto-Neg. link partener ability */
#define GMAC_ANE_EXP	0x000000d0	/* ANE expansion */
#define GMAC_TBI	0x000000d4	/* TBI extend status */
#define GMAC_GMII_STATUS 0x000000d8	/* S/R-GMII status */

/* GMAC Configuration defines */
#define GMAC_CONTROL_TC	0x01000000	/* Transmit Conf. in RGMII/SGMII */
#define GMAC_CONTROL_WD	0x00800000	/* Disable Watchdog on receive */
#define GMAC_CONTROL_JD	0x00400000	/* Jabber disable */
#define GMAC_CONTROL_BE	0x00200000	/* Frame Burst Enable */
#define GMAC_CONTROL_JE	0x00100000	/* Jumbo frame */

#define GMAC_CONTROL_DCRS	0x00010000 /* Disable carrier sense during tx */
#define GMAC_CONTROL_PS		0x00008000 /* Port Select 0:GMI 1:MII */
#define GMAC_CONTROL_FES	0x00004000 /* Speed 0:10 1:100 */
#define GMAC_CONTROL_DO		0x00002000 /* Disable Rx Own */
#define GMAC_CONTROL_LM		0x00001000 /* Loop-back mode */
#define GMAC_CONTROL_DM		0x00000800 /* Duplex Mode */
#define GMAC_CONTROL_IPC	0x00000400 /* Checksum Offload */
#define GMAC_CONTROL_DR		0x00000200 /* Disable Retry */
#define GMAC_CONTROL_LUD	0x00000100 /* Link up/down */
#define GMAC_CONTROL_ACS	0x00000080 /* Automatic Pad Stripping */
#define GMAC_CONTROL_DC		0x00000010 /* Deferral Check */
#define GMAC_CONTROL_TE		0x00000008 /* Transmitter Enable */
#define GMAC_CONTROL_RE		0x00000004 /* Receiver Enable */

#define GMAC_CORE_INIT (GMAC_CONTROL_JD | GMAC_CONTROL_PS | GMAC_CONTROL_ACS | \
			GMAC_CONTROL_IPC | GMAC_CONTROL_JE | GMAC_CONTROL_BE)

/* GMAC Frame Filter defines */
#define GMAC_FRAME_FILTER_PR	0x00000001	/* Promiscuous Mode */
#define GMAC_FRAME_FILTER_HUC	0x00000002	/* Hash Unicast */
#define GMAC_FRAME_FILTER_HMC	0x00000004	/* Hash Multicast */
#define GMAC_FRAME_FILTER_DAIF	0x00000008	/* DA Inverse Filtering */
#define GMAC_FRAME_FILTER_PM	0x00000010	/* Pass all multicast */
#define GMAC_FRAME_FILTER_DBF	0x00000020	/* Disable Broadcast frames */
#define GMAC_FRAME_FILTER_SAIF	0x00000100	/* Inverse Filtering */
#define GMAC_FRAME_FILTER_SAF	0x00000200	/* Source Address Filter */
#define GMAC_FRAME_FILTER_HPF	0x00000400	/* Hash or perfect Filter */
#define GMAC_FRAME_FILTER_RA	0x80000000	/* Receive all mode */
/* GMII ADDR  defines */
#define GMAC_MII_ADDR_WRITE	0x00000002	/* MII Write */
#define GMAC_MII_ADDR_BUSY	0x00000001	/* MII Busy */
/* GMAC FLOW CTRL defines */
#define GMAC_FLOW_CTRL_PT_MASK	0xffff0000	/* Pause Time Mask */
#define GMAC_FLOW_CTRL_PT_SHIFT	16
#define GMAC_FLOW_CTRL_RFE	0x00000004	/* Rx Flow Control Enable */
#define GMAC_FLOW_CTRL_TFE	0x00000002	/* Tx Flow Control Enable */
#define GMAC_FLOW_CTRL_FCB_BPA	0x00000001	/* Flow Control Busy ... */

/*--- DMA BLOCK defines ---*/
/* DMA Bus Mode register defines */
#define DMA_BUS_MODE_SFT_RESET	0x00000001	/* Software Reset */
#define DMA_BUS_MODE_DA		0x00000002	/* Arbitration scheme */
#define DMA_BUS_MODE_DSL_MASK	0x0000007c	/* Descriptor Skip Length */
#define DMA_BUS_MODE_DSL_SHIFT	2	/*   (in DWORDS)      */
/* Programmable burst length (passed thorugh platform)*/
#define DMA_BUS_MODE_PBL_MASK	0x00003f00	/* Programmable Burst Len */
#define DMA_BUS_MODE_PBL_SHIFT	8
#define DMA_BUS_MODE_ATDS	0x80
#define DMA_BUS_MODE_FB		0x00010000	/* Fixed burst */
#define DMA_BUS_MODE_RPBL_MASK	0x003e0000	/* Rx-Programmable Burst Len */
#define DMA_BUS_MODE_RPBL_SHIFT	17
#define DMA_BUS_MODE_USP	0x00800000
#define DMA_BUS_MODE_4PBL	0x01000000
#define DMA_BUS_MODE_AAL	0x02000000

/* DMA CRS Control and Status Register Mapping */
#define DMA_HOST_TX_DESC	  0x00001048	/* Current Host Tx descriptor */
#define DMA_HOST_RX_DESC	  0x0000104c	/* Current Host Rx descriptor */
/*  DMA Bus Mode register defines */
#define DMA_BUS_PR_RATIO_MASK	  0x0000c000	/* Rx/Tx priority ratio */
#define DMA_BUS_PR_RATIO_SHIFT	  14
#define DMA_BUS_FB	  	  0x00010000	/* Fixed Burst */

/* DMA operation mode defines (start/stop tx/rx are placed in common header)*/
#define DMA_CONTROL_DT		0x04000000 /* Disable Drop TCP/IP csum error */
#define DMA_CONTROL_RSF		0x02000000 /* Receive Store and Forward */
#define DMA_CONTROL_DFF		0x01000000 /* Disable flushing */

#define DMA_CONTROL_TSF		0x00200000 /* Transmit  Store and Forward */
#define DMA_CONTROL_FTF		0x00100000 /* Flush transmit FIFO */

enum ttc_control {
	DMA_CONTROL_TTC_64 = 0x00000000,
	DMA_CONTROL_TTC_128 = 0x00004000,
	DMA_CONTROL_TTC_192 = 0x00008000,
	DMA_CONTROL_TTC_256 = 0x0000c000,
	DMA_CONTROL_TTC_40 = 0x00010000,
	DMA_CONTROL_TTC_32 = 0x00014000,
	DMA_CONTROL_TTC_24 = 0x00018000,
	DMA_CONTROL_TTC_16 = 0x0001c000,
};
#define DMA_CONTROL_TC_TX_MASK	0xfffe3fff

#define DMA_CONTROL_EFC		0x00000100
#define DMA_CONTROL_FEF		0x00000080
#define DMA_CONTROL_FUF		0x00000040

enum rtc_control {
	DMA_CONTROL_RTC_64 = 0x00000000,
	DMA_CONTROL_RTC_32 = 0x00000008,
	DMA_CONTROL_RTC_96 = 0x00000010,
	DMA_CONTROL_RTC_128 = 0x00000018,
};
#define DMA_CONTROL_TC_RX_MASK	0xffffffe7

#define DMA_CONTROL_OSF	0x00000004	/* Operate on second frame */

/* MMC registers offset */
#define GMAC_MMC_CTRL      0x100
#define GMAC_MMC_RX_INTR   0x104
#define GMAC_MMC_TX_INTR   0x108
#define GMAC_MMC_RX_CSUM_OFFLOAD   0x208


void ci40_net_init(u8 *mac_addr);
void ci40_net_send(s8 *data, s32 len);
s32 ci40_net_len(void);
void ci40_net_dump(void);
void ci40_net_read(s8 *data, s32 len);
s32 ci40_gmac_dma_init(u32 ioaddr, s32 pbl, u32 dma_tx, u32 dma_rx);
void ci40_gmac_core_init(u32 ioaddr);
void ci40_gmac_set_umac_addr(u32 ioaddr, u8 *addr,	u32 reg_n);
void ci40_gmac_dma_operation_mode(u32 ioaddr, s32 txmode, s32 rxmode);
void ci40_gmac_dma_clear_int(void);
#endif
