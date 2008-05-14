
#ifndef __ASM_ARCH_IO_REGISTERS1_H__
#define __ASM_ARCH_IO_REGISTERS1_H__

/* MMC/SD Controller */
#define IO_PTSTAT                      0x01C41128
#define IO_PTCMD                       0x01C41120
#define IO_MDCTL15                   0x01C41A3C  
#define IO_MMC_CONTROL            0x1E10000
#define IO_MMC_MEM_CLK_CONTROL    0x1E10004
#define IO_MMC_STATUS0            0x01E10008
#define IO_MMC_STATUS1            0x01E1000C
#define IO_MMC_INT_ENABLE         0x01E10010
#define IO_MMC_RESPONSE_TIMEOUT   0x01E10014
#define IO_MMC_READ_TIMEOUT       0x01E10018
#define IO_MMC_BLOCK_LENGTH       0x01E1001C
#define IO_MMC_NR_BLOCKS          0x01E10020
#define IO_MMC_NR_BLOCKS_COUNT    0x01E10024
#define IO_MMC_RX_DATA            0x01E10028
#define IO_MMC_TX_DATA            0x01E1002C
#define IO_MMC_COMMAND            0x01E10030
#define IO_MMC_ARG                0x01E10034
#define IO_MMC_RESPONSE01         0x01E10038
#define IO_MMC_RESPONSE23         0x01E1003C
#define IO_MMC_RESPONSE45         0x01E10040
#define IO_MMC_RESPONSE67         0x01E10044
#define IO_MMC_SPI_DATA           0x01E10048
#define IO_MMC_COMMAND_INDEX      0x01E1004C

#endif
