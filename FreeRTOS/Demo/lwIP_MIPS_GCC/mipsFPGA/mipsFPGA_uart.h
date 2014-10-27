#ifndef __MIPSFPGA_UART_H__
#define __MIPSFPGA_UART_H__

#include <stdio.h>
#include "types.h"
#include "mipsFPGA_intc.h"


#define UART1_BASE_ADDR				0xB0401000
#define UART_BASE_ADDR				UART1_BASE_ADDR
#define UART_CLK_FREQ				1843200
#define UART_BAUD_RATE				115200
#define UART_BAUD_DIVISOR			(UART_CLK_FREQ + (UART_BAUD_RATE * 16)/2)) / (UART_BAUD_RATE * 16)

#define UART_REG_DIV_LO_BYTE		0
#define UART_REG_DIV_HI_BYTE		1
#define UART_REG_SETUP				3

#define UART_REG_SETUP_DIV_ACCESS	0x80

#define CR_UART_LSR_RXFER			

//Reg 1: Interrupt Enable
#define SER_IER_EDSSI_MASK      0x00000008  //Modem status
#define SER_IER_ELSI_MASK       0x00000004  //Line status
#define SER_IER_ETBFI_MASK      0x00000002  //TX ready for data
#define SER_IER_ERBFI_MASK      0x00000001  //RX data available

#define SER_IER_EDSSI_SHIFT     3
#define SER_IER_ELSI_SHIFT      2
#define SER_IER_ETBFI_SHIFT     1
#define SER_IER_ERBFI_SHIFT     0


//Reg 2 (R): Interrupt ID register
#define SER_IIR_FEN_MASK        0x000000C0  //Fifos enabled
#define SER_IIR_ID_MASK         0x0000000E  //Interrupt ID
#define SER_IIR_IPEND_MASK      0x00000001  //Interrupt pending

#define SER_IIR_FEN_SHIFT       6
#define SER_IIR_ID_SHIFT        1
#define SER_IIR_IPEND_SHIFT     0

#define SER_IIR_TRGID           6           //Character timeout id
#define SER_IIR_RXERR           3           //Receive error id
#define SER_IIR_RXDATA          2           //Receive data id
#define SER_IIR_TXRDY           1           //Transmit ready id
#define SER_IIR_MOSTS           0           //Modem status id

//Interrupt IDs
#define TX_FIFO_EMPTY           (SER_IIR_TXRDY  << SER_IIR_ID_SHIFT)
#define RX_DATA                 (SER_IIR_RXDATA << SER_IIR_ID_SHIFT)
#define CHAR_TIMEOUT            (SER_IIR_TRGID  << SER_IIR_ID_SHIFT)
#define RECEIVER_LINE_STATUS    (SER_IIR_RXERR  << SER_IIR_ID_SHIFT)
#define MODEM_STATUS_CHANGE     (SER_IIR_MOSTS  << SER_IIR_ID_SHIFT)
#define NONE                    (SER_IIR_IPEND_MASK)

//Reg 3: Line control register
#define SER_LCR_DLAB_MASK       0x00000080  //Divisor latch accessed if = 1
#define SER_LCR_TXBK_MASK       0x00000040  //Tx break while = 1
#define SER_LCR_STPA_MASK       0x00000020  //Sticky parity force = EVPA
#define SER_LCR_EPS_MASK        0x00000010  //Even parity if = 1
#define SER_LCR_PEN_MASK        0x00000008  //Enable parity
#define SER_LCR_STB_MASK        0x00000004  //Stop bits (2 if =1 (1.5 if 5 bits))
#define SER_LCR_WLS_MASK        0x00000003  //Divisor latch accessed if = 1

//Reg 5: Line Status Register
#define SER_LSR_RXER_MASK       0x00000080  //RX error (PE,FE,BI)
#define SER_LSR_TEMT_MASK       0x00000040  //TX underflow
#define SER_LSR_THRE_MASK       0x00000020  //TX ready for new word
#define SER_LSR_BI_MASK         0x00000010  //Break received
#define SER_LSR_FE_MASK         0x00000008  //RX framing error
#define SER_LSR_PE_MASK         0x00000004  //RX parity error
#define SER_LSR_OE_MASK         0x00000002  //RX overflow (new wor discarded)
#define SER_LSR_DR_MASK         0x00000001  //RX data available

//Word format
#define SER_B5S1P0_FMT          0	//5 bits, 1 stop, no parity
#define SER_B6S1P0_FMT          1   //6 bits, 1 stop, no parity
#define SER_B7S1P0_FMT          2   //7 bits, 1 stop, no parity
#define SER_B8S1P0_FMT          3   //8 bits, 1 stop, no parity

//Word format
#define SER_B5S1P0_FMT          0	//5 bits, 1 stop, no parity
#define SER_B6S1P0_FMT          1	//6 bits, 1 stop, no parity
#define SER_B7S1P0_FMT          2	//7 bits, 1 stop, no parity
#define SER_B8S1P0_FMT          3	//8 bits, 1 stop, no parity

#define SER_5BITS		0
#define SER_6BITS       1
#define SER_7BITS       2
#define SER_8BITS       3

#define SER_1STOPBIT		0
#define SER_2STOPBITS		1

#define SER_NOPARITY	0
#define SER_ODDPARITY	1
#define SER_EVENPARITY	3
#define SER_HIPARITY	5
#define SER_LOPARITY	7

typedef struct mipsfpga_uart {
	u32 rbr_thr_dll;
	u32 ier_dlh;
	u32 iid_fcr;
	u32 lcr;
	u32 mcr;
	u32 lsr;
	u32 msr;
} MIPSFPGA_UART_REGS_T;

typedef struct {
	uint32_t base_addr;
	MIPSFPGA_UART_REGS_T *regs;
	MIPSFPGA_INTC_T *intc;
	xQueueHandle TxQueue;
	xQueueHandle RxQueue;
	uint32_t baud_clk;
	uint32_t speed;
	uint32_t bits;
	uint32_t stop_bits;
	uint32_t parity;
	SemaphoreHandle_t ReadySemaphore;
    uint32_t RxInterruptCount;
} MIPSFPGA_UART_T;

void MIPSFPGA_uart_init(MIPSFPGA_UART_T *uart);
void uartTask(void *parameters);
void uartTx(MIPSFPGA_UART_T *u, u8 *str);
void uartTxDirect(MIPSFPGA_UART_T *u, u8 *str);
u8 uartRx(MIPSFPGA_UART_T *u);
FILE *mipsUART_fopen(MIPSFPGA_UART_T *uart, const char *mode);
#endif
