/*
 * Copyright (C) 2006 - 2008 Neuros Technology LLC.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; only support version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
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

/* timer */
#define IO_TIMER2_TIM12     0x01C21C10
#define IO_TIMER2_TIM34     0x01C21C14
#define IO_TIMER2_PRD12     0x01C21C18
#define IO_TIMER2_PRD34     0x01C21C1C
#define IO_TIMER2_TCR         0x01C21C20 
#define IO_TIMER2_TGCR       0x01C21C24
#define IO_TIMER2_WDTCR     0x01C21C28

#endif
