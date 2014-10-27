#ifndef __TYPES_H__
#define __TYPES_H__

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

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;
typedef signed char             s8;
typedef short                   s16;
typedef int                     s32;
typedef long long               s64;

#define writel(data,addr) *((u32*)(addr)) = data
#define readl(addr) *(u32*)(addr)


/* GMAC core can compute the checksums in HW. */
enum rx_frame_status {
	good_frame = 0,
	discard_frame = 1,
	csum_none = 2,
};

/* Transmit checksum insertion control */
enum tdes_csum_insertion {
	cic_disabled = 0,			/* Checksum Insertion Control */
	cic_only_ip = 1,			/* Only IP header */
	cic_no_pseudoheader = 2,	/* IP header but pseudoheader
								 * is not calculated */
	cic_full = 3,				/* IP header and pseudoheader */
};

struct dma_desc {
	/* Receive descriptor */
	union {
		struct {
			/* RDES0 */
			u32 payload_csum_error:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 mii_error:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 collision:1;
			u32 ipc_csum_error:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 vlan_tag:1;
			u32 overflow_error:1;
			u32 length_error:1;
			u32 sa_filter_fail:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 da_filter_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved1:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 reserved2:5;
			u32 disable_ic:1;

		} rx;
		struct {
			/* RDES0 */
			u32 rx_mac_addr:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 error_gmii:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 late_collision:1;
			u32 ipc_csum_error:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 vlan_tag:1;
			u32 overflow_error:1;
			u32 length_error:1;
			u32 sa_filter_fail:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 da_filter_fail:1;
			u32 own:1;
			/* RDES1 */
			u32 buffer1_size:13;
			u32 reserved1:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 buffer2_size:13;
			u32 reserved2:2;
			u32 disable_ic:1;
		} erx;          /* -- enhanced -- */
 
		 /* Transmit descriptor */
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 vlan_frame:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 payload_error:1;
			u32 frame_flushed:1;
			u32 jabber_timeout:1;
			u32 error_summary:1;
			u32 ip_header_error:1;
			u32 time_stamp_status:1;
			u32 reserved1:13;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 time_stamp_enable:1;
			u32 disable_padding:1;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 crc_disable:1;
			u32 checksum_insertion:2;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
		} tx;
		struct {
			/* TDES0 */
			u32 deferred:1;
			u32 underflow_error:1;
			u32 excessive_deferral:1;
			u32 collision_count:4;
			u32 vlan_frame:1;
			u32 excessive_collisions:1;
			u32 late_collision:1;
			u32 no_carrier:1;
			u32 loss_carrier:1;
			u32 payload_error:1;
			u32 frame_flushed:1;
			u32 jabber_timeout:1;
			u32 error_summary:1;
			u32 ip_header_error:1;
			u32 time_stamp_status:1;
			u32 reserved1:2;
			u32 second_address_chain:1;
			u32 end_ring:1;
			u32 checksum_insertion:2;
			u32 reserved2:1;
			u32 time_stamp_enable:1;
			u32 disable_padding:1;
			u32 crc_disable:1;
			u32 first_segment:1;
			u32 last_segment:1;
			u32 interrupt:1;
			u32 own:1;
			/* TDES1 */
			u32 buffer1_size:13;
			u32 reserved3:3;
			u32 buffer2_size:13;
			u32 reserved4:3;
        } etx;          /* -- enhanced -- */

		u64 all_flags;
	} des01;
	unsigned int des2;
	unsigned int des3;
};
/* TX DESC */
struct tx_dma_desc {
	struct {
		/* TDES0 */
		u32 deferred:1;
		u32 underflow_error:1;
		u32 excessive_deferral:1;
		u32 collision_count:4;
		u32 vlan_frame:1;
		u32 excessive_collisions:1;
		u32 late_collision:1;
		u32 no_carrier:1;
		u32 loss_carrier:1;
		u32 payload_error:1;
		u32 frame_flushed:1;
		u32 jabber_timeout:1;
		u32 error_summary:1;
		u32 ip_header_error:1;
		u32 time_stamp_status:1;
		u32 reserved1:13;
		u32 own:1;
	} des0;
	struct {
		/* TDES1 */
		u32 buffer1_size:11;
		u32 buffer2_size:11;
		u32 time_stamp_enable:1;
		u32 disable_padding:1;
		u32 second_address_chained:1;
		u32 end_ring:1;
		u32 crc_disable:1;
		u32 checksum_insertion:2;
		u32 first_segment:1;
		u32 last_segment:1;
		u32 interrupt:1;
	} des1;
	unsigned int des2;
	unsigned int des3;
};

/* RX DESC */
struct rx_dma_desc {
	struct {
			/* RDES0 */
			u32 payload_csum_error:1;
			u32 crc_error:1;
			u32 dribbling:1;
			u32 mii_error:1;
			u32 receive_watchdog:1;
			u32 frame_type:1;
			u32 collision:1;
			u32 ipc_csum_error:1;
			u32 last_descriptor:1;
			u32 first_descriptor:1;
			u32 vlan_tag:1;
			u32 overflow_error:1;
			u32 length_error:1;
			u32 sa_filter_fail:1;
			u32 descriptor_error:1;
			u32 error_summary:1;
			u32 frame_length:14;
			u32 da_filter_fail:1;
			u32 own:1;
	} des0;
	struct {
			/* RDES1 */
			u32 buffer1_size:11;
			u32 buffer2_size:11;
			u32 reserved1:2;
			u32 second_address_chained:1;
			u32 end_ring:1;
			u32 reserved2:5;
			u32 disable_ic:1;
	} des1;
	unsigned int des2;
	unsigned int des3;
};


struct stmmac_extra_stats {
	/* Transmit errors */
	unsigned long tx_underflow;
	unsigned long tx_carrier;
	unsigned long tx_losscarrier;
	unsigned long tx_heartbeat;
	unsigned long tx_deferred;
	unsigned long tx_vlan;
	unsigned long tx_jabber;
	unsigned long tx_frame_flushed;
	unsigned long tx_payload_error;
	unsigned long tx_ip_header_error;
	/* Receive errors */
	unsigned long rx_desc;
	unsigned long rx_partial;
	unsigned long rx_runt;
	unsigned long rx_toolong;
	unsigned long rx_collision;
	unsigned long rx_crc;
	unsigned long rx_lenght;
	unsigned long rx_mii;
	unsigned long rx_multicast;
	unsigned long rx_gmac_overflow;
	unsigned long rx_watchdog;
	unsigned long da_rx_filter_fail;
	unsigned long sa_rx_filter_fail;
	unsigned long rx_missed_cntr;
	unsigned long rx_overflow_cntr;
	unsigned long rx_vlan;
	/* Tx/Rx IRQ errors */
	unsigned long tx_undeflow_irq;
	unsigned long tx_process_stopped_irq;
	unsigned long tx_jabber_irq;
	unsigned long rx_overflow_irq;
	unsigned long rx_buf_unav_irq;
	unsigned long rx_process_stopped_irq;
	unsigned long rx_watchdog_irq;
	unsigned long tx_early_irq;
	unsigned long fatal_bus_error_irq;
	/* Extra info */
	unsigned long threshold;
	unsigned long tx_pkt_n;
	unsigned long rx_pkt_n;
	unsigned long poll_n;
	unsigned long sched_timer_n;
	unsigned long normal_irq_n;
};

struct net_device_stats {
	u32   rx_packets;
	u32   tx_packets;
	u32   rx_bytes;
	u32   tx_bytes;
	u32   rx_errors;
	u32   tx_errors;
	u32   rx_dropped;
	u32   tx_dropped;
	u32   multicast;
	u32   collisions;
	u32   rx_length_errors;
	u32   rx_over_errors;
	u32   rx_crc_errors;
	u32   rx_frame_errors;
	u32   rx_fifo_errors;
	u32   rx_missed_errors;
	u32   tx_aborted_errors;
	u32   tx_carrier_errors;
	u32   tx_fifo_errors;
	u32   tx_heartbeat_errors;
	u32   tx_window_errors;
	u32   rx_compressed;
	u32   tx_compressed;
};

static inline void stmmac_set_mac_addr(unsigned long ioaddr, u8 addr[6],
	unsigned int high, unsigned int low)
{
	unsigned long data;
 
	data = (addr[5] << 8) | addr[4];
	writel(data, ioaddr + high);
	data = (addr[3] << 24) | (addr[2] << 16) | (addr[1] << 8) | addr[0];
	writel(data, ioaddr + low);

	return;
}
 
static inline void stmmac_get_mac_addr(unsigned long ioaddr,
	unsigned char *addr, unsigned int high,
	unsigned int low)
{
	unsigned int hi_addr, lo_addr;
 
	/* Read the MAC address from the hardware */
	hi_addr = readl(ioaddr + high);
	lo_addr = readl(ioaddr + low);
 
	/* Extract the MAC address from the high and low words */
	addr[0] = lo_addr & 0xff;
	addr[1] = (lo_addr >> 8) & 0xff;
	addr[2] = (lo_addr >> 16) & 0xff;
	addr[3] = (lo_addr >> 24) & 0xff;
	addr[4] = hi_addr & 0xff;
	addr[5] = (hi_addr >> 8) & 0xff;
 
	return;
}

#endif
