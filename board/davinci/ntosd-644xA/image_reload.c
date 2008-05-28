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

#include <common.h>
#include <command.h>
#include <fat.h>
#include <asm/byteorder.h>
#include <asm/sizes.h>
#include <nand.h>

#if defined(CONFIG_UPGRADE_IMAGE)

#define NAMELEN   32
#define VERLEN    20  /* version No. len, same as the package tool*/
#define DESCLEN   256 

#define RETRYTIMES  15
#define TIMEDELAY1   1000000 /* 1s */ 
#define TIMEDELAY2   3000000 /* 3s */

/* see include/image.h, here add three for firmware upgrade */
#define IH_TYPE_UBOOT           9
#define IH_TYPE_ROOTFS          10

#define NEUROS_DESC "Neuros Official UPK"
#define PACKAGE_ID "neuros-osd2.0"

/* the messages start position on the screen */
/*
static point_t msg1_pos  = {140, 200};
static point_t msg2_pos1 = {155, 220};
static point_t msg2_pos2 = {155, 260};
static point_t msg3_pos1 = {10,  140};
static point_t msg3_pos2 = {10,  180};
static point_t msg3_pos3 = {10,  220};
static point_t msg4_pos1 = {195, 220};
static point_t msg4_pos2 = {195, 260};
static point_t msg5_pos1 = {160, 220};
static point_t msg5_pos2 = {160, 260};
static point_t msg6_pos1 = {180, 220};
static point_t msg6_pos2 = {180, 260};
*/

typedef enum 
{
    CARD_NULL,
    CARD_CF,
    CARD_MMCSD,
    CARD_MS,
}CARD_TYPE;

enum
{
    CARD_SD, CARD_MMC
};

extern block_dev_desc_t *get_dev(char * ifname, int dev);
extern long file_fat_read(const char *filename, void *buffer, unsigned long maxsize);
extern int fat_register_device(block_dev_desc_t *dev_desc, int part_no);
extern int do_nand(cmd_tbl_t * cmdtp, int flag, int argc, char *argv[]);

typedef struct u_boot_desc
{
    u32 magic_number;
    u32 load_entry;
    u32 page_count;
    u32 start_block;
    u32 start_page;
    u32 load_addr;
}u_boot_desc_t;

typedef struct packet_header
{
    uint32_t    p_headsize;       /* package header size         */
    uint32_t    p_reserve;        /* Bit[3]:1 OSD2.0 upk */
    uint32_t    p_headcrc;        /* package header crc checksum */
    uint32_t    p_datasize;       /* package data size           */
    uint32_t    p_datacrc;        /* package data crc checksum   */
    uint8_t     p_name[NAMELEN];  /* package name                */
    uint8_t     p_vuboot[VERLEN]; /* version of uboot which depend on */
    uint8_t     p_vkernel[VERLEN];/* version of kernel which depend on*/
    uint8_t     p_vrootfs[VERLEN];/* version of rootfs which depend on*/
    uint32_t    p_imagenum;       /* num of the images in package*/
                                  /* follow is image info */
}package_header_t;

typedef struct image_info
{
    uint32_t    i_type;           /* image type, uboot?        */
    uint32_t    i_imagesize;      /* size of image             */
    uint32_t    i_startaddr_p;    /* start address in packeage */
    uint32_t    i_startaddr_f;    /* start address in flash    */
    uint32_t    i_endaddr_f;      /* end address in flash      */
    uint8_t     i_name[NAMELEN];  /* image name                */
    uint8_t     i_version[VERLEN];/* image version             */
}image_info_t;

typedef struct version_struct
{
    uint8_t  upk_desc[DESCLEN];
    uint8_t  pack_id[NAMELEN];
    uint8_t  os_ver [VERLEN];
    uint8_t  app_ver[VERLEN];
}pack_info;

static char package_name[] = "/newpackage/r3.upk";
static ulong loader_addr;

static ulong package_len;

static package_header_t p_head;
static image_info_t     i_info;
static image_header_t   i_head;
static pack_info        pack_i;

static char percent_num=0; /* 1 ~ 100 */
static char script_per=0, uboot_per=0, kernel_per=0, rootfs_per=0;

extern nand_info_t nand_info[];       /* info for NAND chips */

static void print_phdr(void)
{
    package_header_t *phdr= &p_head;

    if ( phdr->p_headsize !=  (sizeof(package_header_t)+ phdr->p_imagenum * sizeof(image_info_t)) )
        return;

    printf("phdr->p_headsize: %d\n", phdr->p_headsize);
    printf("phdr->p_reserve: %x\n",  phdr->p_reserve);
    printf("phdr->p_headcrc: %x\n",  phdr->p_headcrc);
    printf("phdr->p_datasize: %d\n", phdr->p_datasize);
    printf("phdr->p_datacrc: %x\n",  phdr->p_datacrc);
    printf("phdr->p_name: %s\n",     phdr->p_name);
    printf("phdr->p_vuboot: %s\n",   phdr->p_vuboot);
    printf("phdr->p_vkernel: %s\n",  phdr->p_vkernel);
    printf("phdr->p_vrootfs: %s\n",  phdr->p_vrootfs);
    printf("phdr->p_imagenum: %x\n", phdr->p_imagenum);
}

static void print_image_info(void)
{  
    int i;
    package_header_t *phdr = &p_head;
    image_info_t     *iif  = &i_info;

    for (i=0; i < phdr->p_imagenum; i++)
    {
        memmove(iif, (char *)(loader_addr + sizeof(package_header_t)+ i*sizeof(image_info_t)), sizeof(image_info_t));
        if ( (iif->i_imagesize > SAFE_SIZE) || (strlen(iif->i_name) > NAMELEN) 
             || (strlen(iif->i_version) > VERLEN) )
            return;

        printf("iif->i_type: %x\n",        iif->i_type);
        printf("iif->i_imagesize: %d\n",   iif->i_imagesize);
        printf("iif->i_startaddr_p: %x\n", iif->i_startaddr_p);
        printf("iif->i_startaddr_f: %x\n", iif->i_startaddr_f);
        printf("iif->i_endaddr_f: %x\n",   iif->i_endaddr_f);
        printf("iif->i_name: %s\n",        iif->i_name);
        printf("iif->i_version: %s\n",     iif->i_version);
    }
}

static void print_pack_info(pack_info pack_i)
{
    if ( (strlen(pack_i.upk_desc) > DESCLEN) || (strlen(pack_i.pack_id) > NAMELEN) || 
         (strlen(pack_i.os_ver)  > VERLEN)   || (strlen(pack_i.app_ver) > VERLEN) )
        return;

    printf("pack_i->upk_desc: %s\n", pack_i.upk_desc);
    printf("pack_i->pack_id: %s\n",  pack_i.pack_id);
    printf("pack_i->os_ver : %s\n",  pack_i.os_ver);
    printf("pack_i->app_ver: %s\n",  pack_i.app_ver);
}


#if defined(CONFIG_CMD_MMCSD) || defined(CONFIG_CMD_MS)
static int read_from(char *ifname)
{
    CARD_TYPE type = CARD_NULL;

#if defined(CONFIG_CMD_MMCSD)
    if (strcmp(ifname, "mmc")==0)
    {
        extern int do_mmc_sd(void);

        if (do_mmc_sd() != 0)
        {
            //printf("mmc or sd card initial error\n");
            return(-1);
        }
        type = CARD_MMCSD;
    }
    else
#endif
#if defined(CONFIG_CMD_MS)
        if (strcmp(ifname, "ms")==0)
    {
        extern int do_ms(void);
        if (do_ms() != 0)  return(-1);
        type = CARD_MS;
    }
    else
#endif
    {
        printf("Error interface!!\n");
        return(-1);
    }

    if (fat_register_device(get_dev(ifname, 0), 1) != 0)
    {
        //printf ("** Unable to use %s %d:%d for finding package **\n", ifname, 0, 1);
        return(-1);
    }

    //clearscreen();
    //textout(msg1_pos.x, msg1_pos.y, "loading update package... \n", COLOR_YELLOW, COLOR_BLACK);  
    package_len = file_fat_read (package_name, (uchar *)CFG_PACKAGE_ADDR, SAFE_SIZE);
    //printf("read file size: %x\n", package_len);
    if (package_len == -1)
    {
        //printf("can't read file %s from %s !!\n", package_name, ifname);
        //gio_set_bitclr(GIO_LED_RED);
        //gio_set_bitclr(GIO_LED_GREEN);
        return(-1);
    }

    return 0;
}

static int copy_package_to_ram(void)
{
    if (read_from("mmc") != 0)
    {
        if (read_from("ms") != 0)
            return(-1);
    }
    return(0);
}

#endif

static int get_UPK_info(void)
{
    package_header_t *phdr = &p_head;
    pack_info *pinfo = &pack_i;

    printf("read UPK infomation from the UPK in SRAM\n");
    if (package_len < SAFE_SIZE)
    {
        memcpy( pinfo, (uchar *)(CFG_PACKAGE_ADDR), sizeof(pack_info) );
        loader_addr = CFG_PACKAGE_ADDR+sizeof(pack_info);
        memcpy(phdr, (uchar *)loader_addr, sizeof(package_header_t)); 
    }
    else
    {
        printf("The package_len can't be larger than 250MB\n");
        return(-1);
    }
    print_phdr();
    print_image_info();
    print_pack_info(pack_i);

    return 0;
}

static int isupkvalid(void)
{
    pack_info *pinfo = &pack_i;

    if (strcmp(pinfo->pack_id, PACKAGE_ID))
        return(-1);

    if (strncmp(pinfo->upk_desc, NEUROS_DESC, strlen(NEUROS_DESC)))
    {
        //clearscreen();
        printf("WARNING: You are updating to a package that is not officially released by Neuros... \n");
        /*textout(msg3_pos1.x, msg3_pos1.y, "WARNING:You are updating to a package\n", COLOR_YELLOW, COLOR_BLACK); 
        textout(msg3_pos2.x, msg3_pos2.y, "        that is not officially released\n", COLOR_YELLOW, COLOR_BLACK);
        textout(msg3_pos3.x, msg3_pos3.y, "        by Neuros ...\n", COLOR_YELLOW, COLOR_BLACK);
        udelay(TIMEDELAY2);
        clearscreen();
        show_basic_gui();
        */
    }

    return 0;
}

static int check_image(void)
{
    uint len, checksum, tempchecksum;
    int i;

    package_header_t *phdr = &p_head;
    image_info_t     *iif  = &i_info;
    image_header_t   *ihdr = &i_head;
    package_header_t *temp_phdr;

    printf("checking upgrade package...\n");

    if (package_len < (sizeof(package_header_t) + sizeof(image_info_t)))
    {
        //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg4_pos2.x, msg4_pos2.y, "Invalid package!\n", COLOR_YELLOW, COLOR_BLACK);
        return(-1);
    }
    if (get_UPK_info())
    {
        //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg4_pos2.x, msg4_pos2.y, "Invalid package!\n", COLOR_YELLOW, COLOR_BLACK);
        printf("Invalid package!\n");
        udelay(TIMEDELAY2);
        return(-1);
    }

    if (isupkvalid())  return(-1);

    temp_phdr = (package_header_t *)loader_addr;

    if (phdr->p_headsize != (sizeof(package_header_t) + phdr->p_imagenum*sizeof(image_info_t)))
    {
        //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg4_pos2.x, msg4_pos2.y, "Package error!\n", COLOR_YELLOW, COLOR_BLACK);
        printf("bad package head size\n");
        return(-1);
    }

    checksum = phdr->p_headcrc;
    len = phdr->p_headsize;
    temp_phdr->p_headcrc = 0;

    if (crc32(0, (uchar *)loader_addr, len) != checksum)
    {
        //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg4_pos2.x, msg4_pos2.y, "Package error!\n", COLOR_YELLOW, COLOR_BLACK);
        printf ("bad package header checksum != %x \n", checksum);
        return(-1);
    }
    else
    {
        temp_phdr->p_headcrc = checksum;
    }

    /* judge if the package match the board */
    if (!(phdr->p_reserve & 0x08))
    {
        //textout(msg5_pos1.x, msg5_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg5_pos2.x, msg5_pos2.y, "Package don't match!\n", COLOR_YELLOW, COLOR_BLACK);
        printf("\npackage do not match the board!\n\n");
        return(-1);
    }

    checksum = phdr->p_datacrc;
    len = phdr->p_datasize;
    tempchecksum = crc32(0,(uchar *)(loader_addr+phdr->p_headsize),len);
    if (tempchecksum != checksum)
    {
        //textout(msg4_pos1.x, msg4_pos1.y, "Sorry\n", COLOR_YELLOW, COLOR_BLACK);
        //textout(msg4_pos2.x, msg4_pos2.y, "Package error!\n", COLOR_YELLOW, COLOR_BLACK);
        printf ("bad package data checksum != %x \n", checksum);
        udelay(TIMEDELAY1);
        return(-1);
    }
    else
    {
        phdr->p_datacrc = checksum;
    }

    for (i=0; i < phdr->p_imagenum; i++)
    {
        memmove(iif, (uchar *)(loader_addr + sizeof(package_header_t))+ i*sizeof(image_info_t), sizeof(image_info_t));

        if ( (iif->i_type != IH_TYPE_SCRIPT) && (iif->i_imagesize > (iif->i_endaddr_f - iif->i_startaddr_f + 1)) )
        {
            //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
            //textout(msg4_pos2.x, msg4_pos2.y, "Package too big!\n", COLOR_YELLOW, COLOR_BLACK);
            printf("%s image size is too big!\n", iif->i_name);
            udelay(TIMEDELAY1);
            return(-1);
        }
        if (iif->i_type == IH_TYPE_SCRIPT)
        {
            memmove(ihdr, (uchar *)(loader_addr+iif->i_startaddr_p), sizeof(image_header_t));
            checksum = ntohl(ihdr->ih_hcrc);
            ihdr->ih_hcrc = 0;
            if (crc32(0, (uchar *)ihdr, sizeof(image_header_t)) != checksum)
            {
                //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
                //textout(msg4_pos2.x, msg4_pos2.y, "Package error!\n", COLOR_YELLOW, COLOR_BLACK);
                printf ("bad image header checksum != %x\n", checksum);
                udelay(TIMEDELAY1);
                return(-1);
            }
            else
            {
                ihdr->ih_hcrc = htonl(checksum);
            }
            checksum = ntohl(ihdr->ih_dcrc);
            len = ntohl(ihdr->ih_size);
            if (crc32(0, (uchar *)(loader_addr+iif->i_startaddr_p+sizeof(image_header_t)),len) != checksum)
            {
                //textout(msg4_pos1.x, msg4_pos1.y, "Sorry,\n", COLOR_YELLOW, COLOR_BLACK);
                //textout(msg4_pos2.x, msg4_pos2.y, "Package error!\n", COLOR_YELLOW, COLOR_BLACK);
                printf ("bad image data checksum != %x\n", checksum);
                udelay(TIMEDELAY1);
                return(-1);
            }
            else
            {
                ihdr->ih_dcrc = htonl(checksum);
            }
        }
    }

    printf("image correct\n");
    return(0);
}

static void calculate_percent(void)
{
    int i;
    package_header_t *phdr = &p_head;
    image_info_t     *iif  = &i_info;


    memmove(phdr, (char *)loader_addr, sizeof(package_header_t)); 
    for (i=0; i < phdr->p_imagenum; i++)
    {
        memmove(iif, (char *)(loader_addr + sizeof(package_header_t)+ i*sizeof(image_info_t)), sizeof(image_info_t));
        if (iif->i_type == IH_TYPE_SCRIPT)
            script_per = SZ_128K*100 / phdr->p_datasize; /* evn size 128KB*/
        else if (iif->i_type == IH_TYPE_UBOOT)
            uboot_per = iif->i_imagesize*100 /phdr->p_datasize;
        else if (iif->i_type == IH_TYPE_KERNEL)
            kernel_per = iif->i_imagesize*100 /phdr->p_datasize;
        else if (iif->i_type == IH_TYPE_ROOTFS)
            rootfs_per += iif->i_imagesize*100 /phdr->p_datasize;
    }
}

/*use do_nand can skip the bad block, 
if derectly use nand_write or nand_erase can not skip bad block*/
static int run_nand_cmd(const char *cmd, ulong off, ulong size, u_char *addr)
{
    char argv[5][20];
    char *arg[5]={argv[0],argv[1],argv[2],argv[3],argv[4]};
    strncpy(argv[0], "nand", 30);
    strncpy(argv[1], cmd, 30);
    if(!strcmp(argv[1], "erase"))
    {
        sprintf(argv[2], "%x", off);
        sprintf(argv[3], "%x", size);
        return do_nand(NULL, 0, 4, arg);
    }
    else
    {
        sprintf(argv[3], "%x", off);
        sprintf(argv[4], "%x", size);
        sprintf(argv[2], "%x", (ulong)addr);
        return do_nand(NULL, 0, 5, arg);
    }
}

static int actual_update(void)
{
    int i,j, rc;
    ulong temp_ihdr;

    package_header_t *phdr = &p_head;
    image_info_t     *iif  = &i_info;
    nand_info_t *nand;
    ulong offset, size;
    u_boot_desc_t desc = {
        0xA1ACED66,
        0x81080000,
        0x00000600,
        0x00000006,
        0x00000001,
        0x81080000
    };

    percent_num = 0;
    //show_percent(percent_num);
    calculate_percent();
    nand = &nand_info[nand_curr_device];
    for (i=0; i < phdr->p_imagenum; i++)
    {
        memmove (iif, (char *)(loader_addr + sizeof(package_header_t)+ i*sizeof(image_info_t)), sizeof(image_info_t));

        temp_ihdr = loader_addr + iif->i_startaddr_p;
        printf("image size : %x\n", iif->i_imagesize);

        if (iif->i_type == IH_TYPE_ROOTFS)
        {
            offset = iif->i_startaddr_f;
            size   = iif->i_endaddr_f - iif->i_startaddr_f + 1;            
            printf("NAND start address: %x, size: %x \n",offset, size);
            if (run_nand_cmd("erase", offset, size, NULL))
            {
                printf("nand erase failed\n");
                run_command("reset", 0);
                return(-1);
            }
            size = iif->i_imagesize-1;
            rc = run_nand_cmd("write.yaffs1", offset, size, (uchar *)temp_ihdr);
            if (rc != 0 )
            {
                printf("nand write failed\n");
                run_command("reset", 0);
                return(-1);
            }
            percent_num += rootfs_per;
            //show_percent(percent_num);
            continue;
        }
        else if (iif->i_type == IH_TYPE_KERNEL)
        {
            offset = iif->i_startaddr_f;
            size   = iif->i_endaddr_f - iif->i_startaddr_f + 1;            
            printf("NAND start address: %x, size: %x \n",offset, size);
            if (run_nand_cmd("erase", offset, size, NULL))
            {
                printf("nand erase failed\n");
                run_command("reset", 0);
                return(-1);
            }
            size = size - (size>>4); //must reserve some redundance for bad block
            size = (size / NAND_PAGE_SIZE) * NAND_PAGE_SIZE;
            rc = run_nand_cmd("write.jffs2", offset, size, (uchar *)temp_ihdr);
            if (rc != 0 )
            {
                printf("nand write failed\n");
                run_command("reset", 0);
                return(-1);
            }
            percent_num += kernel_per;
            //show_percent(percent_num);
            continue;
        }
        else if (iif->i_type == IH_TYPE_UBOOT)
        {
            offset = iif->i_startaddr_f;
            size   = iif->i_endaddr_f - iif->i_startaddr_f + 1;
            printf("NAND start address: %x, size: %x \n",offset, size);
            if (run_nand_cmd("erase", offset, size, NULL))
            {
                printf("nand erase failed\n");
                run_command("reset", 0);
                return(-1);
            }
            for(j=0; j<6; j++) 
            {
                offset = iif->i_startaddr_f + j * NAND_BLOCK_SIZE;
                size = NAND_PAGE_SIZE;
                rc = nand_write(nand, offset, &size, (uchar *)(&desc));
                if (rc != 0 )
                {
                    printf("write u-boot-desc failed at 0x%x\n", offset);
                    desc.start_block++;
                }
                else 
                {                    
                    break;
                }
            }
            if(j == 6) 
            {
                run_command("reset", 0);
                return -1;
            }
            offset = offset + NAND_PAGE_SIZE;
            size   = iif->i_endaddr_f - iif->i_startaddr_f - NAND_PAGE_SIZE + 1;
            size = size - (size>>4); //must reserve some redundance for bad block
            size = (size / NAND_PAGE_SIZE) * NAND_PAGE_SIZE;
            rc = run_nand_cmd("write", offset, size, (uchar *)temp_ihdr);
            if (rc != 0 )
            {
                printf("nand write failed\n");
                run_command("reset", 0);
                return(-1);
            }
            percent_num += uboot_per;
            //show_percent(percent_num);
            continue;
        }
        else if (iif->i_type == IH_TYPE_SCRIPT)
        {
            if (autoscript(temp_ihdr) != 0) return(-1);
            setenv("bootcmd", "run nand_boot");
            saveenv();
            percent_num += script_per;
            //show_percent(percent_num);
            continue;
        }
    }

    percent_num = 100;
    //show_percent(percent_num);
    //show_win();
    udelay(TIMEDELAY2);
    //textout(msg4_pos1.x, msg4_pos1.y, "Upgrade completed,\n", COLOR_GRAY, COLOR_BLACK);
    //textout(msg4_pos2.x, msg4_pos2.y, "rebooting...\n", COLOR_GRAY, COLOR_BLACK);
    udelay(TIMEDELAY1);

    return(0);
}

static int do_update(void)
{
    //clearscreen();
    //show_basic_gui();
    if (check_image() != 0)
    {
        //gio_set_bitset(GIO_LED_RED);
        //gio_set_bitclr(GIO_LED_GREEN);
        return(-1);
    }
    if (actual_update() != 0)
    {
        //gio_set_bitset(GIO_LED_RED);
        //gio_set_bitclr(GIO_LED_GREEN);
        return(-1);
    }
    else
    {
        //gio_set_bitclr(GIO_LED_RED);
        //gio_set_bitset(GIO_LED_GREEN);
    }

    return(0);
}

/** open for the flash burning can update the percent num */
void update_percent(void)
{
    if (++percent_num > 100) percent_num = 100;
    //show_percent(percent_num);
}

/** upgrade process  */
int chk_upgrade_flag(void)
{
    printf("CFG_PACKAGE_ADDR = %x\n",CFG_PACKAGE_ADDR);
#if defined(CONFIG_CMD_MMCSD) || defined(CONFIG_CMD_MS)
    if (copy_package_to_ram() != 0) return(0);
    setenv("upgrade_type", "emergency");
    if (do_update() == 0)
    {
        run_command("reset", 0);
    }
#endif
    return(0);
}

#endif
