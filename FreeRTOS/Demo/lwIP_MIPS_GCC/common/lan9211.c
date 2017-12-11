/***(C)2017***************************************************************
*
* Copyright (c) 2017, Imagination Technologies Limited
****(C)2017**************************************************************/


#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/tcpip.h"
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

#include "lan9211.h"
#include "intc.h"


#ifdef LITTLE_ENDIAN
#define cpu_to_le32( value ) (value)
#define le32_to_cpu( value ) (value)
#else
#define cpu_to_le32( value ) ( (                ((uint32_t)value)  << 24) |   \
                               ((0x0000FF00UL & ((uint32_t)value)) <<  8) |   \
                               ((0x00FF0000UL & ((uint32_t)value)) >>  8) |   \
                               (                ((uint32_t)value)  >> 24)   )
#define le32_to_cpu( value ) cpu_to_le32( value )
#endif

/* System Control and Status Registers */
#define SMSC_RXDATA_FIFO	0x00
#define SMSC_TXDATA_FIFO	0x20
#define  TX_CMDA_INT		0x80000000
#define  TX_CMDA_ALIGN4		0x00000000
#define  TX_CMDA_ALIGN16	0x01000000
#define  TX_CMDA_ALIGN32	0x02000000
#define  TX_CMDA_OFFSET		0x001f0000
#define  TX_CMDA_FS			0x00002000
#define  TX_CMDA_LS			0x00001000
#define  TX_CMDA_BSIZE		0x000007ff

#define SMSC_RXSTAT_FIFO	0x40
#define SMSC_RXSTAT_FIFO_PEEK	0x44
#define	 RXSTAT_PKTLEN		0x3FFF0000
#define	 RXSTAT_ES			0x00008000
#define	 RXSTAT_BROADCAST	0x00002000
#define	 RXSTAT_LE			0x00001000
#define	 RXSTAT_RUNT_ERR	0x00000800
#define	 RXSTAT_MULTICAST	0x00000400
#define	 RXSTAT_TOOLONG_ERR	0x00000080
#define	 RXSTAT_COLL_ERR	0x00000040
#define	 RXSTAT_FRAMETYPE	0x00000020
#define	 RXSTAT_WATCHDOG	0x00000010
#define	 RXSTAT_MII			0x00000008
#define	 RXSTAT_DRIBBLE		0x00000004
#define	 RXSTAT_CRC_ERR		0x00000002

#define SMSC_TXSTAT_FIFO		0x48
#define SMSC_TXSTAT_FIFO_PEEK	0x4c
#define  TXSTAT_TAG			0xffff0000
#define  TXSTAT_ES			0x00008000
#define  TXSTAT_LOC			0x00000800
#define  TXSTAT_NOC			0x00000400
#define  TXSTAT_LC			0x00000200
#define  TXSTAT_EC			0x00000100
#define  TXSTAT_CCNT		0x00000078
#define  TXSTAT_ED			0x00000004

#define SMSC_ID_REV			0x50
#define SMSC_IRQ_CFG		0x54
#define  IRQ_CFG_DEAS		0xff000000
#define  IRQ_CFG_DEASCLR	0x00004000
#define  IRQ_CFG_DEASSTS	0x00002000
#define  IRQ_CFG_IRQINT		0x00001000
#define  IRQ_CFG_IRQEN		0x00000100
#define  IRQ_CFG_IRQPOL		0x00000010
#define  IRQ_CFG_IRQTYPE	0x00000001

#define SMSC_INT_STS		0x58
#define  INT_STS_SW			0x80000000
#define  INT_STS_TXSTOP		0x02000000
#define  INT_STS_RXSTOP		0x01000000
#define  INT_STS_RXDFH		0x00800000
#define  INT_STS_TXIOC		0x00200000
#define  INT_STS_RXD		0x00100000
#define  INT_STS_GPT		0x00080000
#define  INT_STS_PHY		0x00040000
#define  INT_STS_PME		0x00020000
#define  INT_STS_TXSO		0x00010000
#define  INT_STS_RWT		0x00008000
#define  INT_STS_RXE		0x00004000
#define  INT_STS_TXE		0x00002000
#define  INT_STS_TDFO		0x00000400
#define  INT_STS_TDFA		0x00000200
#define  INT_STS_TSFF		0x00000100
#define  INT_STS_TSFL		0x00000080
#define  INT_STS_RXDF		0x00000040
#define  INT_STS_RSFF		0x00000010
#define  INT_STS_RSFL		0x00000008
#define  INT_STS_GPIO2		0x00000004
#define  INT_STS_GPIO1		0x00000002
#define  INT_STS_GPIO0		0x00000001

#define SMSC_INT_EN			0x5c
#define SMSC_BYTE_TEST		0x64
#define SMSC_FIFO_INT		0x68
#define SMSC_RX_CFG			0x6c
#define SMSC_TX_CFG			0x70
#define  TX_CFG_TXSDUMP		0x00008000
#define  TX_CFG_TXDDUMP		0x00004000
#define  TX_CFG_TXSAO		0x00000004
#define  TX_CFG_TXON		0x00000002
#define  TX_CFG_STOPTX		0x00000001

#define SMSC_HW_CFG			0x74
#define  HW_CFG_FPORTEND	0x20000000
#define  HW_CFG_FSELEND		0x10000000
#define  HW_CFG_AMDIX		0x01000000
#define  HW_CFG_MBO			0x00100000
#define  HW_CFG_TXFIFOSZ	0x000f0000
#define  HW_CFG_SRSTTO		0x00000002
#define  HW_CFG_SRST		0x00000001

#define SMSC_RX_DP_CTL		0x78
#define SMSC_RX_FIFO_INF	0x7c
#define  RX_FIFO_INF_RXSUSED	0x00ff0000
#define  RX_FIFO_INF_RXDUSED	0x0000ffff
#define SMSC_TX_FIFO_INF	0x80
#define  TX_FIFO_INF_TXSUSED	0x00ff0000
#define  TX_FIFO_INF_TDFREE	0x0000ffff

#define SMSC_PMT_CTRL		0x84
#define  PMT_CTRL_PMMODE	0x00003000
#define  PMT_CTRL_PHYRST	0x00000400
#define  PMT_CTRL_WOLEN		0x00000200
#define  PMT_CTRL_EDEN		0x00000100
#define  PMT_CTRL_PMETYPE	0x00000040
#define  PMT_CTRL_WUPS		0x00000030
#define  PMT_CTRL_PMEIND	0x00000008
#define  PMT_CTRL_PMEPOL	0x00000004
#define  PMT_CTRL_PMEEN		0x00000002
#define  PMT_CTRL_READY		0x00000001

#define SMSC_GPIO_CFG		0x88
#define  GPIO_CFG_LED3		0x40000000
#define  GPIO_CFG_LED2		0x20000000
#define  GPIO_CFG_LED1		0x10000000
#define  GPIO_CFG_INTPOL3	0x04000000
#define  GPIO_CFG_INTPOL2	0x02000000
#define  GPIO_CFG_INTPOL1	0x01000000
#define  GPIO_CFG_BUF3		0x00040000
#define  GPIO_CFG_BUF2		0x00020000
#define  GPIO_CFG_BUF1		0x00010000

#define SMSC_GPT_CFG		0x8c
#define  GPT_CFG_TIMEREN	0x20000000

#define SMSC_GPT_CNT		0x90
#define SMSC_WORD_SWAP		0x98
#define SMSC_FREE_RUN		0x9c
#define SMSC_RX_DROP		0xa0

#define SMSC_MAC_CSR_CMD	0xa4
#define  MAC_CSR_CMD_BUSY	0x80000000
#define  MAC_CSR_CMD_RNW	0x40000000

#define SMSC_MAC_CSR_DATA	0xa8

#define SMSC_AFC_CFG		0xac
#define  AFC_CFG_HI			0x00ff0000
#define  AFC_CFG_LO			0x0000ff00
#define  AFC_CFG_BACKDUR	0x000000f0
#define  AFC_CFG_FCMULTI	0x00000008
#define  AFC_CFG_FCBRD		0x00000004
#define  AFC_CFG_FCADD		0x00000002
#define  AFC_CFG_FCANY		0x00000001

#define SMSC_E2P_CMD		0xb0
#define E2P_CMD_BUSY		(1<<31)
#define E2P_CMD_OP(x)		((x)<<28)
#define E2P_CMD_TIMEO		(1<<9)
#define E2P_CMD_MACLD		(1<<8)
#define E2P_CMD_ADDR(x)		((x)<<0)

#define OP_READ			0
#define OP_EWDS			1
#define OP_EWEN			2
#define OP_WRITE		3
#define OP_WRAL			4
#define OP_ERASE		5
#define OP_ERAL			6
#define OP_RELOAD		7

#define SMSC_E2P_DATA		0xb4

/* MAC control and Status Registers */
/* Accessed indirectly via SMSC_MAC_CSR_CMD/SMSC_MAC_CSR_DATA */
#define MAC_CR			1
#define  MAC_CR_RXALL		0x80000000
#define  MAC_CR_TXEN		0x00000008
#define  MAC_CR_RXEN		0x00000004
#define MAC_CR_HBDIS
#define MAC_ADDRH		2
#define MAC_ADDRL		3
#define MAC_HASHH		4
#define MAC_HASHL		5
#define MAC_MII_ACC		6
#define MAC_MII_DATA	7
#define MAC_FLOW		8
#define  MAC_FLOW_FCPT		0xffff0000
#define  MAC_FLOW_FCPASS	0x00000004
#define  MAC_FLOW_FCEN		0x00000002
#define  MAC_FLOW_FCBSY		0x00000001
#define MAC_VLAN1		9
#define MAC_VLAN2		10
#define MAC_WUFF		11
#define MAC_WUCSR		11
#define MAC_COE_CR		12

/* Phy Control and Staus Registers */
/* Accessed indirectly via MAC_CSR_MII_ACC/MAC_CSR_MII_DATA */
#define MII_BCR			0
#define  BCR_RESET		0x8000
#define  BCR_AUTON		0x1000
#define  BCR_RSTNEG		0x0100

#define MII_BSR			1
#define  BSR_LS			0x0004

#define MII_ID1			2
#define MII_ID2			3
#define MII_ANAR		4
#define  ANAR_
#define MII_ANLPAR		5
#define MII_ANER		6
#define MII_MCSR		17
#define MII_SMACR		18
#define MII_SCSR		27
#define MII_ISR			29
#define MII_IMACR		30
#define MII_PCSR		31

/* MAC_MII_ACC */
#define MII_ACC_WNR		0x00000002
#define MII_ACC_BUSY	0x00000001


static uint32_t RR(LAN9211_T * lan9211, uint32_t r)
{
	return lan9211->regs[r / 4];
}

static void RW(LAN9211_T * lan9211, uint32_t r, uint32_t v)
{
	lan9211->regs[r / 4] = v;
}

/* MAC access */
static uint32_t MACR(LAN9211_T * lan9211, uint32_t mr)
{
	for (; RR(lan9211, SMSC_MAC_CSR_CMD) & MAC_CSR_CMD_BUSY;) ;
	RW(lan9211, SMSC_MAC_CSR_CMD, MAC_CSR_CMD_BUSY | MAC_CSR_CMD_RNW | mr);
	for (; RR(lan9211, SMSC_MAC_CSR_CMD) & MAC_CSR_CMD_BUSY;) ;
	return RR(lan9211, SMSC_MAC_CSR_DATA);
}

static void MACW(LAN9211_T * lan9211, uint32_t mr, uint32_t v)
{
	for (; RR(lan9211, SMSC_MAC_CSR_CMD) & MAC_CSR_CMD_BUSY;) ;
	RW(lan9211, SMSC_MAC_CSR_DATA, v);
	RW(lan9211, SMSC_MAC_CSR_CMD, MAC_CSR_CMD_BUSY | mr);
	/* It should be safe to return without waiting for completion */
	for (; RR(lan9211, SMSC_MAC_CSR_CMD) & MAC_CSR_CMD_BUSY;) ;
}

/* MII access */
static uint32_t MIIR(LAN9211_T * lan9211, uint32_t miir)
{
	for (; MACR(lan9211, MAC_MII_ACC) & MII_ACC_BUSY;) ;
	MACW(lan9211, MAC_MII_ACC, ((1 << 11) | (miir << 6) | MII_ACC_BUSY));
	for (; MACR(lan9211, MAC_MII_ACC) & MII_ACC_BUSY;) ;
	return MACR(lan9211, MAC_MII_DATA);
}

static void MIIW(LAN9211_T * lan9211, uint32_t miir, uint32_t v)
{
	for (; MACR(lan9211, MAC_MII_ACC) & MII_ACC_BUSY;) ;
	MACW(lan9211, MAC_MII_DATA, v);
	MACW(lan9211, MAC_MII_ACC,
	     ((1 << 11) | (miir << 6) | MII_ACC_WNR | MII_ACC_BUSY));
	/* It should be safe to return without waiting for completion */
	for (; MACR(lan9211, MAC_MII_ACC) & MII_ACC_BUSY;) ;
}

static err_t LAN9211_txFrame(struct netif *netif, struct pbuf *p)
{
	LAN9211_T *lan9211 = netif->state;
	struct pbuf *q;
	uint32_t *data;
	uint32_t i, len, aligned;

	/* Command hw to send q->tot_len byte frame */
	RW(lan9211, SMSC_TXDATA_FIFO,
	   cpu_to_le32(TX_CMDA_ALIGN4 | TX_CMDA_FS | TX_CMDA_LS | p->tot_len));
	RW(lan9211, SMSC_TXDATA_FIFO, cpu_to_le32(p->tot_len));

	len = (p->tot_len + 3) / 4;	/* Length in whole words */
#if ETH_PAD_SIZE
	pbuf_header(p, -ETH_PAD_SIZE);
#endif
	for (q = p; q != NULL; q = q->next) {
		data = (uint32_t *) q->payload;
		for (i = 0; i < q->len; i += 4) {
			memcpy(&aligned, data, sizeof(aligned));
			RW(lan9211, SMSC_TXDATA_FIFO, cpu_to_le32(aligned));
			len--;
			data++;
		}
	}
	while (len--)
		RW(lan9211, SMSC_TXDATA_FIFO, 0);

#if ETH_PAD_SIZE
	pbuf_header(p, ETH_PAD_SIZE);
#endif
	while ((RR(lan9211, SMSC_TX_FIFO_INF) & TX_FIFO_INF_TXSUSED) == 0) ;
	if (RR(lan9211, SMSC_TXSTAT_FIFO) & TXSTAT_ES)
		return ERR_IF;

	LINK_STATS_INC(link.xmit);

	return ERR_OK;
}

static struct pbuf *LAN9211_rxFrame(struct netif *netif)
{
	LAN9211_T *lan9211 = netif->state;
	struct pbuf *p, *q;
	uint16_t len;
	uint32_t *data;
	uint32_t rxstat, i, aligned;

	if ((RR(lan9211, SMSC_RX_FIFO_INF) & RX_FIFO_INF_RXSUSED) == 0)
		return NULL;

	rxstat = RR(lan9211, SMSC_RXSTAT_FIFO);
	if (rxstat & (RXSTAT_ES | RXSTAT_LE | RXSTAT_WATCHDOG | RXSTAT_MII)) {
		return NULL;
	}

	len = (rxstat & RXSTAT_PKTLEN) >> 16;

#if ETH_PAD_SIZE
	len += ETH_PAD_SIZE;
#endif

	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

	len = (len + 3) / 4;	/* Length in whole words */

	if (p != NULL) {

#if ETH_PAD_SIZE
		pbuf_header(p, -ETH_PAD_SIZE);
#endif

		for (q = p; q != NULL; q = q->next) {
			data = (uint32_t *) q->payload;
			for (i = 0; i < q->len; i += 4) {
				aligned =
				    le32_to_cpu(RR(lan9211, SMSC_RXDATA_FIFO));
				memcpy(data, &aligned, sizeof(aligned));
				len--;
				data++;
			}
		}

#if ETH_PAD_SIZE
		pbuf_header(p, ETH_PAD_SIZE);
#endif

		LINK_STATS_INC(link.recv);
	} else {
		LINK_STATS_INC(link.memerr);
		LINK_STATS_INC(link.drop);
	}

	while (len--)
		RR(lan9211, SMSC_RXDATA_FIFO);

	return p;
}

static void LAN9211_input(LAN9211_T *lan9211)
{
	struct pbuf *p;
	struct eth_hdr *ethhdr;
	struct netif *netif = &lan9211->netif;

	for (;;) {
		p = LAN9211_rxFrame(netif);
		if (p == NULL) {
			return;
		}
		ethhdr = (struct eth_hdr *)p->payload;

		switch (htons(ethhdr->type)) {
		case ETHTYPE_ARP:
		case ETHTYPE_IP:
#if PPPOE_SUPPORT
		case ETHTYPE_PPPOE:
		case ETHTYPE_PPPOEDISC:

#endif				/* PPPOE_SUPPORT */
			if (netif->input(p, netif) == ERR_OK)
				continue;
			LWIP_DEBUGF(NETIF_DEBUG,
				("LAN9211_input: packet input error\n"));
			break;
		default:
			LWIP_DEBUGF(NETIF_DEBUG,
				("LAN9211_input: unknown packet type\n"));
		}
		pbuf_free(p);
	}
}

static err_t LAN9211_init(struct netif *netif)
{
	LAN9211_T *lan9211 = (LAN9211_T *) netif->state;
	uint32_t ml, mh, tries, status;

	LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
	netif->hostname = "lwip";
#endif				/* LWIP_NETIF_HOSTNAME */

	/* Brazenly assume 100baseT */
	NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100 * 1024 * 1024);

	netif->name[0] = 'e';
	netif->name[1] = 'n';
	netif->output = etharp_output;
	netif->linkoutput = LAN9211_txFrame;
	netif->mtu = 1500;
	netif->flags =
	    NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	netif->hwaddr_len = ETHARP_HWADDR_LEN;

	if ((RR(lan9211, SMSC_PMT_CTRL) & PMT_CTRL_READY) == 0) {
		RW(lan9211, SMSC_BYTE_TEST, 0);
		for (tries = 10; tries > 0; --tries) {
			if (RR(lan9211, SMSC_PMT_CTRL) & PMT_CTRL_READY)
				break;
			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
		if (tries <= 0)
			return ERR_IF;
	}

	/* Full reset */
	RW(lan9211, SMSC_HW_CFG, HW_CFG_SRST);
	for (tries = 10; tries > 0; --tries) {
		status = RR(lan9211, SMSC_HW_CFG);
		if (status & HW_CFG_SRSTTO)
			return ERR_IF;	/* Phy initialisation failed */
		if ((status & HW_CFG_SRST) == 0)
			break;
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (tries <= 0)
		return ERR_IF;

	/* Wait for EEPROM accesses to complete */
	for (tries = 50; tries > 0; --tries) {
		if ((RR(lan9211, SMSC_E2P_CMD) & E2P_CMD_BUSY) == 0)
			break;
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
	if (tries <= 0)
		return ERR_IF;

	/* Handset MAC address if it has not been read from EEROM */
	if ((RR(lan9211, SMSC_E2P_CMD) & E2P_CMD_MACLD) == 0) {
		MACW(lan9211, MAC_ADDRH,
		     (lan9211->backupMac[5] << 8) | lan9211->backupMac[4]);
		MACW(lan9211, MAC_ADDRL,
		     (lan9211->
		      backupMac[3] << 24) | (lan9211->backupMac[2] << 16) |
		     (lan9211->backupMac[1] << 8) | lan9211->backupMac[0]);
	}

	/* Back propagate MAC */
	mh = MACR(lan9211, MAC_ADDRH);
	ml = MACR(lan9211, MAC_ADDRL);
	netif->hwaddr[5] = _mips32r2_ext(mh, 8, 8);
	netif->hwaddr[4] = _mips32r2_ext(mh, 0, 8);
	netif->hwaddr[3] = _mips32r2_ext(ml, 24, 8);
	netif->hwaddr[2] = _mips32r2_ext(ml, 16, 8);
	netif->hwaddr[1] = _mips32r2_ext(ml, 8, 8);
	netif->hwaddr[0] = _mips32r2_ext(ml, 0, 8);

	/* disable and clear interrupts */
	RW(lan9211, SMSC_INT_EN, 0);
	RW(lan9211, SMSC_INT_STS, ~0);
	RW(lan9211, SMSC_IRQ_CFG, 0x22000101);

	/* Flow control and buffering */
#if BYTE_ORDER == BIG_ENDIAN
	/* Byteswap word accesses to FIFO data */
	RW(lan9211, SMSC_HW_CFG,
	   HW_CFG_FPORTEND | HW_CFG_FSELEND | HW_CFG_SRST);
#else
	RW(lan9211, SMSC_HW_CFG, HW_CFG_MBO | (4 << 16));	/* TXFIFO 4KB */
#endif
	MACW(lan9211, MAC_FLOW, (0xffff << 16) | MAC_FLOW_FCEN);
	RW(lan9211, SMSC_AFC_CFG,
	   (80 << 16) | (40 << 8) | (0x7 << 4) | AFC_CFG_FCANY);

	/* Enable GPIO pins as LED outputs */
	RW(lan9211, SMSC_GPIO_CFG,
	   (GPIO_CFG_LED3 | GPIO_CFG_LED2 | GPIO_CFG_LED1 | GPIO_CFG_BUF3 |
	    GPIO_CFG_BUF2 | GPIO_CFG_BUF1));

	/* Configure phy */
	RW(lan9211, SMSC_PMT_CTRL,
	   RR(lan9211, SMSC_PMT_CTRL) | PMT_CTRL_PHYRST);
	/* Wait for EEPROM accesses to complete */
	for (tries = 20; tries > 0; --tries) {
		if ((RR(lan9211, SMSC_PMT_CTRL) & PMT_CTRL_PHYRST) == 0)
			break;
		vTaskDelay(10 / portTICK_PERIOD_MS);
	}
	if (tries <= 0)
		return ERR_IF;

	MIIW(lan9211, MII_BCR, BCR_RESET);

	vTaskDelay(1000 / portTICK_PERIOD_MS);

	MIIW(lan9211, MII_ANAR, 0x01e1);

	MIIW(lan9211, MII_BCR, BCR_AUTON | BCR_RSTNEG);

	/* Enable TX */
	RW(lan9211, SMSC_GPT_CFG, GPT_CFG_TIMEREN | 10000);
	RW(lan9211, SMSC_TX_CFG, TX_CFG_TXON);

	/* no padding to start of packets */
	RW(lan9211, SMSC_RX_CFG, 0);

	MACW(lan9211, MAC_CR, MAC_CR_RXALL | MAC_CR_TXEN | MAC_CR_RXEN);

	/* Interrupt configuration */
	RW(lan9211, SMSC_FIFO_INT, 0);
	RW(lan9211, SMSC_INT_EN, INT_STS_PHY | INT_STS_RSFL);
	RW(lan9211, SMSC_INT_STS, ~0);

	return ERR_OK;
}

volatile int unexpected=0;
static void LAN9211_isr(void* param)
{

	LAN9211_T *lan9211 = (LAN9211_T *) param;
	uint32_t status = RR(lan9211, SMSC_INT_STS), miistatus;
	RW(lan9211, SMSC_INT_EN, 0);
	RW(lan9211, SMSC_INT_STS, status);
	RW(lan9211, SMSC_INT_EN, INT_STS_PHY | INT_STS_RSFL);
	if (status & INT_STS_PHY) {
		MIIR(lan9211, MII_BSR);	/* Clear sticky */
		miistatus = MIIR(lan9211, MII_BSR);
		if (miistatus & BSR_LS)
			netif_set_link_up(&lan9211->netif);
		else
			netif_set_link_down(&lan9211->netif);
	}
	if (status & INT_STS_RSFL) {
		LAN9211_input(lan9211);
		lan9211->rx_count++;
	}
	else
	{
		unexpected++;
	}
}

void LAN9211_ethInit(LAN9211_T * lan9211, HANDLER_DESC_T* irq)
{
	HANDLER_DESC_T handler_info;
	static struct ip_addr addr, mask, gw;
	IP4_ADDR(&addr, 0, 0, 0, 0);
	IP4_ADDR(&mask, 0, 0, 0, 0);
	IP4_ADDR(&gw, 0, 0, 0, 0);
	netif_add(&lan9211->netif, &addr, &mask, &gw, lan9211, LAN9211_init,
		  tcpip_input);
	/* make it the default interface */
	netif_set_default(&lan9211->netif);

	irq->function = LAN9211_isr;
	irq->parameter = lan9211;
	intc_RegisterHandler(lan9211->intc, irq);
}

