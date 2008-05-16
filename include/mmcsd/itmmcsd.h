/********************************************************************************
itmmcsd.h -> driver for the MMC/SD Card controller on Texas Instruments DM275,
DM320 SoC.

August 19, 2005 Nathan J. Crawford.

Copyright 2005 Ingenient Technologies, Inc.

This driver supports basic operations for the TI DM275 MMC/SD controller.

********************************************************************************/

//#ifndef TIDM275_H
//#define TIDM275_H


//#include "../include/ingenient-devices.h"

#define IT_DMA_WRITE	1
#define IT_DMW_READ	0
#define SD_CARD_DETECT_BIT 2 
#define SD_CARD_WP_BIT 1

#define IO_CLK_MMCCLK	0x0482
#define MMC_CLK_ENABLE  (1 << 8)
/* Yes, this should probably go in asm/arch/gio.h */
/*typedef enum {
	gio_output = 0,
	gio_input
	}gio_direction;
*/
/* just to be on the safe side, define some standard data types.  */
/*
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned long
#define s8 signed char
#define s16 signed short
#define s32 signed long
*/
#if 1
#define MMC_INT_DAT3 		(1 << 11)
#define MMC_INT_DAT_RCV_RDY	(1 << 10)
#define MMC_INT_DAT_TRX_RDY	(1 << 9)
#define MMC_INT_RSP_CRC_ERROR	(1 << 7)
#define MMC_INT_READ_CRC_ERROR	(1 << 6)
#define MMC_INT_WRITE_CRC_ERROR	(1 << 5)
#define MMC_INT_RSP_TIMEOUT	(1 << 4)
#define MMC_INT_READ_TIMEOUT	(1 << 3)
#define MMC_INT_RSP_DONE	(1 << 2)
#define MMC_INT_BUSY_DONE	(1 << 1)
#define MMC_INT_DATA_DONE	(1 << 0)

#define MMC_INT_DAT_RCV_RDY_S1 	(1 << 3)
#define MMC_INT_DAT_TRX_RDY_S1	(1 << 2)

/* Response Macros */
#define MMCSD_RSPNONE                   (0x0000)
#define MMCSD_RSP1                      (0x0200)
#define MMCSD_RSP2                      (0x0400)
#define MMCSD_RSP3                      (0x0600)
#define MMCSD_RSP4                       MMCSD_RSP1
#define MMCSD_RSP5                       MMCSD_RSP1
#define MMCSD_RSP6                       MMCSD_RSP1

/*Insert 80 CLK cycles*/

#define MMC_INITCK                      (0x4000)

/*Write enable */

#define MMC_WR_EN                       (0x0800)
	
/*data transfer*/

#define MMCSD_DATA_TRANS		(0x2000)

/* Command Macros */
#define MMCSD_CMD0                      (0x0000)
#define MMCSD_CMD1                      (0x0001)
#define MMCSD_CMD2                      (0x0002)
#define MMCSD_CMD3                      (0x0003)
#define MMCSD_CMD4                      (0x0004)
#define MMCSD_CMD5                      (0x0005)
#define MMCSD_CMD6                      (0x0006)
#define MMCSD_CMD7                      (0x0007)
#define MMCSD_CMD8                      (0x0008)
#define MMCSD_CMD9                      (0x0009)
#define MMCSD_CMD10                     (0x000A)
#define MMCSD_CMD11                     (0x000B)
#define MMCSD_CMD12                     (0x000C)
#define MMCSD_CMD13                     (0x000D)
#define MMCSD_CMD14                     (0x000E)
#define MMCSD_CMD15                     (0x000F)
#define MMCSD_CMD16                     (0x0010)
#define MMCSD_CMD17                     (0x0011)
#define MMCSD_CMD18                     (0x0012)
#define MMCSD_CMD19                     (0x0013)
#define MMCSD_CMD20                     (0x0014)
#define MMCSD_CMD21                     (0x0015)
#define MMCSD_CMD22                     (0x0016)
#define MMCSD_CMD23                     (0x0017)
#define MMCSD_CMD24                     (0x0018)
#define MMCSD_CMD25                     (0x0019)
#define MMCSD_CMD26                     (0x001A)
#define MMCSD_CMD27                     (0x001B)
#define MMCSD_CMD28                     (0x001C)
#define MMCSD_CMD29                     (0x001D)
#define MMCSD_CMD30                     (0x001E)
#define MMCSD_CMD31                     (0x001F)
#define MMCSD_CMD32                     (0x0020)
#define MMCSD_CMD33                     (0x0021)
#define MMCSD_CMD34                     (0x0022)
#define MMCSD_CMD35                     (0x0023)
#define MMCSD_CMD36                     (0x0024)
#define MMCSD_CMD37                     (0x0025)
#define MMCSD_CMD38                     (0x0026)
#define MMCSD_CMD39                     (0x0027)
#define MMCSD_CMD40                     (0x0028)
#define MMCSD_CMD41                     (0x0029)
#define MMCSD_CMD42                     (0x002A)
#define MMCSD_CMD43                     (0x002B)
#define MMCSD_CMD44                     (0x002C)
#define MMCSD_CMD45                     (0x002D)
#define MMCSD_CMD46                     (0x002E)
#define MMCSD_CMD47                     (0x002F)
#define MMCSD_CMD48                     (0x0030)
#define MMCSD_CMD49                     (0x0031)
#define MMCSD_CMD50                     (0x0032)
#define MMCSD_CMD51                     (0x0033)
#define MMCSD_CMD52                     (0x0034)
#define MMCSD_CMD53                     (0x0035)
#define MMCSD_CMD54                     (0x0036)
#define MMCSD_CMD55                     (0x0037)
#define MMCSD_CMD56                     (0x0038)
#define MMCSD_CMD57                     (0x0039)
#define MMCSD_CMD58                     (0x003A)
#define MMCSD_CMD59                     (0x003B)
#define MMCSD_CMD60                     (0x003C)
#define MMCSD_CMD61                     (0x003D)
#define MMCSD_CMD62                     (0x003E)
#define MMCSD_CMD63                     (0x003F)
#define MMCSD_CMD64                     (0x0040)

/*OCR request voltage*/
	
#define OCR_REQUEST 			(0x00100000)

/*Busy expected*/

#define MMCSD_BSYEXP			(0x0100)

/* Commands and their responses */
/* MMC and SD */
#define MMCSD_GO_IDLE_STATE                   (MMCSD_CMD0 | MMCSD_RSPNONE)
#define MMCSD_ALL_SEND_CID                    (MMCSD_CMD2 | MMCSD_RSP2 )
#define MMCSD_ALL_SEND_CID_INITCK             (MMCSD_CMD2 | MMCSD_RSP2 | MMC_INITCK)
#define MMCSD_SEND_RELATIVE_ADDRESS	      (MMCSD_CMD3 | MMCSD_RSP6)
#define SD_SEND_OP_COND			      (MMCSD_CMD41 | MMCSD_RSP3)
#define SD_SEND_SD_STATUS		      (MMCSD_CMD41 | MMCSD_RSP1)
#define SD_SEND_SCR                           (MMCSD_CMD51 | MMCSD_RSP1)
#define MMCSD_SET_DSR                         (MMCSD_CMD4 | MMCSD_RSPNONE)
#define MMCSD_SELECT_CARD                     (MMCSD_CMD7 | MMCSD_RSP1)
#define MMCSD_DESELECT_CARD                   (MMCSD_CMD7 )
#define MMCSD_SEND_CSD                        (MMCSD_CMD9 | MMCSD_RSP2)
#define MMCSD_SEND_CID                        (MMCSD_CMD10| MMCSD_RSP2)
#define MMCSD_SEND_STATUS                     (MMCSD_CMD13 | MMCSD_RSP1)
#define MMCSD_GO_INACTIVE_STATE               (MMCSD_CMD15 | MMCSD_RSPNONE)
#define MMCSD_APP_CMD                         (MMCSD_CMD55 | MMCSD_RSP1 )
#define MMCSD_STOP_TRANSMISSION               (MMCSD_CMD12 | MMCSD_RSP1 | MMCSD_BSYEXP)
#define MMCSD_READ_MULTIPLE_BLOCK             (MMCSD_CMD18 | MMCSD_RSP1)
#define MMCSD_WRITE_MULTIPLE_BLOCK            (MMCSD_CMD25 | MMCSD_RSP1 ) /*| MMCSD_BSYEXP)*/

/* Common to SPI & MMC */
#define MMCSD_SET_BLOCKLEN                    (MMCSD_CMD16 | MMCSD_RSP1 )
#define MMCSD_PROGRAM_CSD                     (MMCSD_CMD27 | MMCSD_RSP1 | MMCSD_BSYEXP) /* MMC-bsy, SPI-bsy optional */
#define MMCSD_SET_WRITE_PROT                  (MMCSD_CMD28 | MMCSD_RSP1 | MMCSD_BSYEXP)
#define MMCSD_CLR_WRITE_PROT                  (MMCSD_CMD29 | MMCSD_RSP1 | MMCSD_BSYEXP)
#define MMCSD_SEND_WRITE_PROT                 (MMCSD_CMD30 | MMCSD_RSP1)
#define MMCSD_READ_SINGLE_BLOCK               (MMCSD_CMD17 | MMCSD_RSP1 )
#define MMCSD_WRITE_BLOCK                     (MMCSD_CMD24 | MMCSD_RSP1 )/*| MMC_BSYEXP)*/

/* Commands and their responses */
/* MMC */
#if 0
#define MMC_SEND_OP_COND                    (MMCSD_CMD1 | MMCSD_RSP3 )
#define MMC_SET_RELATIVE_ADDR               (MMCSD_CMD3 | MMCSD_RSP1)
#define MMC_READ_DAT_UNTIL_STOP             (MMCSD_CMD11 | MMCSD_RSP1)
#define MMC_WRITE_DAT_UNTIL_STOP            (MMCSD_CMD20 | MMCSD_RSP1 | MMCSD_BSYEXP)
#define MMC_PROGRAM_CID                     (MMCSD_CMD26 | MMCSD_RSP1 | MMCSD_BSYEXP)   /* Programmed only once by Manufacturer. Hence not implemented */
#define MMC_FAST_IO                         (MMCSD_CMD39 | MMCSD_RSP4 )
#define MMC_GO_IRQ_STATE                    (MMCSD_CMD40 | MMCSD_RSP5 )
#define MMC_LOCK_UNLOCK                     (MMCSD_CMD42 | MMCSD_RSP1 | MMCSD_BSYEXP)
#define MMC_GEN_CMD                         (MMCSD_CMD56 | MMCSD_RSP1 | MMCSD_BSYEXP)
#endif

/* SPI */
#define MMC_SPI_GO_IDLE_STATE               (MMCSD_CMD0 | MMCSD_RSP1)
#define MMC_SPI_SEND_OP_COND                (MMCSD_CMD1 | MMCSD_RSP1)
#define MMC_SPI_SEND_CSD                    (MMCSD_CMD9 | MMCSD_RSP1)
#define MMC_SPI_SEND_CID                    (MMCSD_CMD10| MMCSD_RSP1)
#define MMC_SPI_SEND_STATUS                 (MMCSD_CMD13 | MMCSD_RSP2)
#define MMC_CRC_ON_OFF                      (MMCSD_CMD59 | MMCSD_RSP1)

/* Common to SPI & MMC */
#define MMC_SET_BLOCKCOUNT                  (MMCSD_CMD23 | MMCSD_RSP1 )
#define MMC_TAG_SECTOR_START                (MMCSD_CMD32 | MMCSD_RSP1)
#define MMC_TAG_SECTOR_END                  (MMCSD_CMD33 | MMCSD_RSP1)
#define MMC_UNTAG_SECTOR                    (MMCSD_CMD34 | MMCSD_RSP1)
#define MMC_TAG_ERASE_GROUP_START           (MMCSD_CMD35 | MMCSD_RSP1)
#define MMC_TAG_ERASE_GROUP_END             (MMCSD_CMD36 | MMCSD_RSP1)
#define MMC_UNTAG_ERASE_GROUP               (MMCSD_CMD37 | MMCSD_RSP1)
//#define MMC_ERASE                           (MMCSD_CMD38 | MMCSD_RSP1 | MMCSD_BSYEXP)

#endif

//endif
