/*
 * Copyright (C) 2007 Sergey Kubushyn <ksi@koi8.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef __CONFIG_H
#define __CONFIG_H
#include <asm/sizes.h>

/*=======*/
/* Board */
/*=======*/
#define NTOSD_644XA
#define CFG_NAND_LARGEPAGE
#define CFG_USE_NAND
/*===================*/
/* SoC Configuration */
/*===================*/
#define CONFIG_ARM926EJS			/* arm926ejs CPU core */
#define CONFIG_SYS_CLK_FREQ	297000000	/* Arm Clock frequency */
#define CFG_TIMERBASE		0x01c21400	/* use timer 0 */
#define CFG_HZ_CLOCK		27000000	/* Timer Input clock freq */
#define CFG_HZ			1000
/*=============*/
/* Memory Info */
/*=============*/
#define CFG_MALLOC_LEN		(0x10000 + 512*1024)	/* malloc() len */
#define CFG_GBL_DATA_SIZE	128		/* reserved for initial data */
#define CFG_MEMTEST_START	0x80000000	/* memtest start address */
#define CFG_MEMTEST_END		0x81000000	/* 16MB RAM test */
#define CONFIG_NR_DRAM_BANKS	1		/* we have 1 bank of DRAM */
#define CONFIG_STACKSIZE	(256*1024)	/* regular stack */
#define PHYS_SDRAM_1		0x80000000	/* DDR Start */
#define PHYS_SDRAM_1_SIZE	0x10000000	/* DDR size 128MB */
#define DDR_8BANKS				/* 4-bank DDR2 (128MB) */
/*====================*/
/* Serial Driver info */
/*====================*/
#define CFG_NS16550
#define CFG_NS16550_SERIAL
#define CFG_NS16550_REG_SIZE	4		/* NS16550 register size */
#define CFG_NS16550_COM1	0x01c20000	/* Base address of UART0 */
#define CFG_NS16550_CLK		27000000	/* Input clock to NS16550 */
#define CONFIG_CONS_INDEX	1		/* use UART0 for console */
#define CONFIG_BAUDRATE		115200		/* Default baud rate */
#define CFG_BAUDRATE_TABLE	{ 9600, 19200, 38400, 57600, 115200 }
/*===================*/
/* I2C Configuration */
/*===================*/
#define CONFIG_HARD_I2C
#define CONFIG_DRIVER_DAVINCI_I2C
#define CFG_I2C_SPEED		80000	/* 100Kbps won't work, silicon bug */
#define CFG_I2C_SLAVE		10	/* Bogus, master-only in U-Boot */
/*==================================*/
/* Network & Ethernet Configuration */
/*==================================*/
#define CONFIG_DRIVER_TI_EMAC
#define CONFIG_MII
#define CONFIG_BOOTP_DEFAULT
#define CONFIG_BOOTP_DNS
#define CONFIG_BOOTP_DNS2
#define CONFIG_BOOTP_SEND_HOSTNAME
#define CONFIG_NET_RETRY_COUNT	10
//#define CONFIG_OVERWRITE_ETHADDR_ONCE
/*=====================*/
/* Flash & Environment */
/*=====================*/
#undef CFG_ENV_IS_IN_FLASH
#define CFG_NO_FLASH
#define CFG_ENV_IS_IN_NAND		/* U-Boot env in NAND Flash  */
#ifdef CFG_NAND_SMALLPAGE
#define CFG_ENV_SECT_SIZE	512	/* Env sector Size */
#define CFG_ENV_SIZE		SZ_16K
#else
#define CFG_ENV_SECT_SIZE       2048
#define CFG_ENV_SIZE            SZ_128K
#endif
#define CONFIG_ENV_OVERWRITE
#define CONFIG_SKIP_LOWLEVEL_INIT	/* U-Boot is loaded by a bootloader */
#define CONFIG_SKIP_RELOCATE_UBOOT	/* to a proper address, init done */
#define CFG_NAND_BASE		0x02000000
#define CFG_NAND_HW_ECC
#define CONFIG_MTD_NAND_VERIFY_WRITE
#define CFG_MAX_NAND_DEVICE	1	/* Max number of NAND devices */
#define NAND_MAX_CHIPS		1
#define CFG_ENV_OFFSET		0x0	/* Block 0--not used by bootcode */
/*=====================*/
/* Board related stuff */
/*=====================*/
#define CONFIG_RTC_DS1307		/* RTC chip on SCHMOOGIE */
#define CFG_I2C_RTC_ADDR	0x6f	/* RTC chip I2C address */
#define CONFIG_HAS_UID
#define CONFIG_UID_DS28CM00		/* Unique ID on SCHMOOGIE */
#define CFG_UID_ADDR		0x50	/* UID chip I2C address */
/*==============================*/
/* U-Boot general configuration */
/*==============================*/
#define BOARD_LATE_INIT
#define CONFIG_MMC 
#define CONFIG_DOS_PARTITION
#undef 	CONFIG_USE_IRQ			/* No IRQ/FIQ in U-Boot */
#define CONFIG_MISC_INIT_R
#define CONFIG_BOOTDELAY	3
#define CONFIG_BOOTFILE		"/images/uImage"/* Boot file name */
#define CFG_PROMPT		"U-Boot > "	/* Monitor Command Prompt */
#define CFG_CBSIZE		1024		/* Console I/O Buffer Size  */
#define CFG_PBSIZE		(CFG_CBSIZE+sizeof(CFG_PROMPT)+16)	/* Print buffer sz */
#define CFG_MAXARGS		16		/* max number of command args */
#define CFG_BARGSIZE		CFG_CBSIZE	/* Boot Argument Buffer Size */
#define CFG_LOAD_ADDR		0x82000000	/* default Linux kernel load address */
#define CONFIG_LOADADDR		CFG_LOAD_ADDR
#define CONFIG_VERSION_VARIABLE
#define CONFIG_AUTO_COMPLETE		/* Won't work with hush so far, may be later */
#define CFG_PROMPT_HUSH_PS2	"> "
#define CONFIG_CMDLINE_EDITING
#define CFG_LONGHELP
#define CONFIG_CRC32_VERIFY
#define CONFIG_MX_CYCLIC

#define CONFIG_HOSTNAME		neuros-osd
#define CONFIG_IPADDR		192.168.1.100
#define CONFIG_SERVERIP		192.168.1.1
#define CONFIG_GATEWAYIP	192.168.1.1
#define CONFIG_NETMASK		255.255.255.0
#define CONFIG_ETHADDR		6E:65:75:72:6F:73
/*===================*/
/* Linux Information */
/*===================*/
#define LINUX_BOOT_PARAM_ADDR	0x80000100
#define CONFIG_CMDLINE_TAG
#define CONFIG_SETUP_MEMORY_TAGS
#define CONFIG_BOOTARGS		""
/*=================*/
/* U-Boot commands */
/*=================*/
#include <config_cmd_default.h>
#define CONFIG_CMD_ASKENV
#define CONFIG_CMD_DHCP
#define CONFIG_CMD_DIAG
#define CONFIG_CMD_I2C
#define CONFIG_CMD_MII
#define CONFIG_CMD_PING
#define CONFIG_CMD_SAVES
#define CONFIG_CMD_DATE
#define CONFIG_CMD_NAND
#define CONFIG_CMD_MMCSD		/* neuros ntosd davinci MMC/SD support			*/
#undef CONFIG_CMD_EEPROM
#undef CONFIG_CMD_BDI
#undef CONFIG_CMD_FPGA
#undef CONFIG_CMD_SETGETDCR
#undef CONFIG_CMD_FLASH
#undef CONFIG_CMD_IMLS
#define CONFIG_CMD_FAT		/* FAT support			*/
/*=======================*/
/* KGDB support (if any) */
/*=======================*/
#ifdef CONFIG_CMD_KGDB
#define CONFIG_KGDB_BAUDRATE	115200	/* speed to run kgdb serial port */
#define CONFIG_KGDB_SER_INDEX	1	/* which serial port to use */
#endif

/*=======================*/
/*   JFFS2  Support      */
/*=======================*/
#define CONFIG_CMD_JFFS2
#undef CONFIG_JFFS2_CMDLINE
#define CONFIG_JFFS2_NAND 1
#define CONFIG_JFFS2_DEV "nand0"
#define CONFIG_JFFS2_PART_SIZE   0x500000
#define CONFIG_JFFS2_PART_OFFSET 0x1C0000

/*=======================*/
/*   YAFFS2  Support     */
/*=======================*/
#define CFG_NAND_YAFFS_WRITE
#define CFG_NAND_YAFFS_NEW_OOB_LAYOUT

/*=======================*/
/*  UPGRADE IMAGE  Support      */
/*=======================*/
#define CONFIG_CMD_UPDATE_UBOOT
#define CONFIG_UPGRADE_IMAGE
#if defined(CONFIG_UPGRADE_IMAGE)
#define SAFE_SIZE (224*1024*1024)
#define CFG_PACKAGE_ADDR  (PHYS_SDRAM_1 + (32*1024*1024))
#define CFG_FLAG_ADDR     (CFG_PACKAGE_ADDR - 4)
#define NAND_PAGE_SIZE 2048
#define NAND_BLOCK_SIZE (128*1024)
#endif
#define UBL_NAND_START_PAGE	64
#define UBL_NAND_END_PAGE	383

#endif /* __CONFIG_H */
