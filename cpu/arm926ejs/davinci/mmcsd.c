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
#include <malloc.h>
#include <config.h>
#include <linux/types.h>
#include <asm/io.h>
#include <mmcsd/mmcsd.h>
#include <mmcsd/protocol.h>
#include <mmcsd/card.h>
#include <asm-arm/arch-davinci/io_registers.h>
#include <mmcsd/mmc_debug.h>
#include <mmcsd/itmmcsd.h>

#if defined(CONFIG_CMD_MMCSD)

#define MMC_BUS_WIDTH_4 2
#define MMC_BUS_WIDTH_1 1
#define MMC_MODE_SD 2
#define MMC_MODE_MMC 1
#define CMD_RETRIES 3
#define HOST_BIGENDIAN 0
#define MAX_READ_CNT 0xffff

static const unsigned int tran_exp[] = {
    10000,      100000,     1000000,    10000000,
    0,      0,      0,      0
};

static const unsigned char tran_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};
static const unsigned int tacc_exp[] = {
    1,  10, 100,    1000,   10000,  100000, 1000000, 10000000,
};

static const unsigned int tacc_mant[] = {
    0,  10, 12, 13, 15, 20, 25, 30,
    35, 40, 45, 50, 55, 60, 70, 80,
};

block_dev_desc_t mmcsd_dev;
struct mmc_card *mmc_sd_card=NULL;
static int card_type;

void init_clocks(void)
{
    unsigned int reg = 0;

    /* disable clock */
    outl(0,IO_MMC_MEM_CLK_CONTROL);
    wmb();

    udelay(500);

    /* set mmc clock to ~ 400 kHz */ /* 338 KHZ */
    outl(0x80,IO_MMC_MEM_CLK_CONTROL);
    wmb();
    udelay(500);

    /* enable clock */
    reg = inl(IO_MMC_MEM_CLK_CONTROL);
    rmb();
    outl(reg | 0x0100, IO_MMC_MEM_CLK_CONTROL);
    wmb();
    reg = inl(IO_MMC_MEM_CLK_CONTROL);
    rmb();
}

static void host_configuration(void)
{
    unsigned int reg = 0;

    /* support mmc sd and ms card in the same slot */
    /*
    reg = inl(IO_CLK_MOD2);
    outl(reg & (~(0x4000)), IO_CLK_MOD2); // disable ms clk 
    reg = inl(IO_CLK_MOD2);
    outl(0, IO_MEM_STICK_MODE);
    reg = inl(IO_CLK_MOD2);
    outl(reg | 0x0800, IO_CLK_MOD2);      // enable mmc clk 
    */

    /* enable mmc/sd module */
    while(inl(IO_PTSTAT)&1);
    reg=inl(IO_MDCTL15);
    outl(reg|0x3, IO_MDCTL15);
    wmb();
    reg=inl(IO_MDCTL15);
    rmb();
    outl(reg|(1<<9), IO_MDCTL15);
    wmb();
    outl(1, IO_PTCMD);
    wmb();
    while(inl(IO_PTSTAT)&1);

    /* bring the controller to reset state */
    outl(0x01, IO_MMC_CONTROL);
    wmb();
    outl(0x02, IO_MMC_CONTROL);
    wmb();
    reg = inl(IO_MMC_CONTROL);
    rmb();

    init_clocks();

#if HOST_BIGENDIAN
    outl(0x0200 | 0x0400 | 0x3, IO_MMC_CONTROL);
#else
    outl(0x3, IO_MMC_CONTROL);
#endif

    /*Data timeout register*/
    /*
    reg = inl (IO_MMC_RESPONSE_TIMEOUT);
    outl(reg | 0x1F00,IO_MMC_RESPONSE_TIMEOUT);
    */
    outl(0xFFFF,IO_MMC_RESPONSE_TIMEOUT);
    outl(0xFFFF,IO_MMC_READ_TIMEOUT);

    /* release controller reset state */
    reg = inl(IO_MMC_CONTROL);
    outl(reg & ~(1),IO_MMC_CONTROL);
    reg = inl(IO_MMC_CONTROL);
    outl(reg & ~(1 << 1),IO_MMC_CONTROL);
    reg = inl(IO_MMC_CONTROL);
}

/*
 * This function is used for SDHC when initial must send this command first
 */
static int mmc_send_if_cond(u32 ocr, int *status)
{
    struct mmc_command cmd;
    const u8 sdch_code = 0xAA;
    int err, sdhc;

    cmd.arg = ((ocr & 0xFF8000) != 0) << 8 | sdch_code;
    cmd.opcode = SD_SEND_IF_COND;
    cmd.flags = MMCSD_RSP7;
    err = mmc_wait_for_cmd(&cmd, 0);
    if(err == MMC_ERR_NONE)
    {
        if((cmd.resp[6] & 0xFF) == sdch_code)
        {
            sdhc = 1;
        }
        else
        {
            sdhc = 0;
            err = MMC_ERR_FAILED;
        }
    }
    else
    {
        sdhc = 0;
        err = MMC_ERR_NONE;
    }
    if(status)
        *status = sdhc;

    return err;
}

struct mmc_card *mmc_setup(void)
{
    int err;
    u32 ocr;
    struct mmc_card *card;
    unsigned char bus_width;
    unsigned char mode;

    mmc_idle_cards(); 

    mode = MMC_MODE_SD;
    err = mmc_send_if_cond(1 << 16, NULL);
    if(err != MMC_ERR_NONE)
    {
        return NULL;
    }

    err = mmc_send_app_op_cond(0, &ocr);
    bus_width = MMC_BUS_WIDTH_1;

    /*
     * If we fail to detect any SD cards then try
     * searching for MMC cards.
     */
    if(err != MMC_ERR_NONE)
    {
        mmc_debug_msg("Searching for SD card\n");
        mmc_idle_cards();
        mode = MMC_MODE_MMC;
        bus_width = MMC_BUS_WIDTH_1;
        err = mmc_send_op_cond(0, &ocr);
        if(err != MMC_ERR_NONE)
            return NULL;
    }

    /*
     * Since we're changing the OCR value, we seem to
     * need to tell some cards to go back to the idle
     * state.  We wait 1ms to give cards time to
     * respond.
     */
    mmci_set_ios(bus_width);

    /*
     * We should remember the OCR mask from the existing
     * cards, and detect the new cards OCR mask, combine
     * the two and re-select the VDD.  However, if we do
     * change VDD, we should do an idle, and then do a
     * full re-initialisation.  We would need to notify
     * drivers so that they can re-setup the cards as
     * well, while keeping their queues at bay.
     *
     * For the moment, we take the easy way out - if the
     * new cards don't like our currently selected VDD,
     * they drop off the bus.
     */
    if(ocr == 0)
        return NULL;

    /*
     * Send the selected OCR multiple times... until the cards
     * all get the idea that they should be ready for CMD2.
     * (My SanDisk card seems to need this.)
     */
    mmc_idle_cards();
    if(mode == MMC_MODE_SD)
    {
        int sdhc;
        err = mmc_send_if_cond(ocr, &sdhc);
        if(err == MMC_ERR_NONE)
        {
            if(sdhc)
                mmc_send_app_op_cond(ocr | (1<<30), NULL);
            else
                mmc_send_app_op_cond(ocr, NULL);    
        }
    }
    else
        mmc_send_op_cond(ocr, NULL);

    card_type = mode;
    card = mmc_discover_cards(mode);
    if(card==NULL)
        printf("card=NULL\n");

    /*
     * Ok, now switch to push-pull mode.
     */

    mmc_read_csds(card);

    change_clk25m();    
    return card;
}

static void mmc_read_csds(struct mmc_card *card)
{
    struct mmc_command cmd;
    int err;

    mmc_debug_msg ("%s %d card->state : %d\n",__FUNCTION__,__LINE__,card->state);

    if(card->state & (MMC_STATE_DEAD|MMC_STATE_PRESENT))
        return;

    cmd.opcode = MMC_SEND_CSD;
    cmd.arg = card->rca << 16;
    cmd.flags = MMCSD_RSP2;

    err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);
    if(err != MMC_ERR_NONE)
    {
        mmc_card_set_dead(card);
        return;
    }

    memcpy(card->raw_csd, cmd.resp, sizeof(card->raw_csd));

    mmc_decode_csd(card);
    mmc_decode_cid(card);
}

    #define UNSTUFF_BITS(resp,start,size) stuff_bits(resp,start,size)
int stuff_bits (u16 * resp,int start,int size)
{
    u16 __size = size;
    u16 __mask = (__size < 16 ? 1 << __size : 0) - 1;
    int __off = ((start) / 16);
    int __shft = (start) & 15;
    u16 __res = 0;

    __res = resp [__off] >> __shft;
    if(__size + __shft > 16)
    {
        __res = resp [__off] >> __shft;
        __res |= resp[__off+1] << ((16 - __shft) % 16);
    }

    return(__res & __mask);
}

static void mmc_decode_csd(struct mmc_card *card)
{
    struct mmc_sd_csd *csd = &card->csd;
    unsigned int e, m, csd_struct;
    u16 *resp =(unsigned short *) card->raw_csd;

    if(mmc_card_sd(card))
    {
        csd_struct = UNSTUFF_BITS(resp, 126, 2);
        if(csd_struct == 0)
        {
            m = UNSTUFF_BITS(resp, 115, 4);
            e = UNSTUFF_BITS(resp, 112, 3);
            csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
            csd->tacc_clks = UNSTUFF_BITS(resp, 104, 8) * 100;

            m = UNSTUFF_BITS(resp, 99, 4);
            e = UNSTUFF_BITS(resp, 96, 3);
            csd->max_dtr = tran_exp[e] * tran_mant[m];
            csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

            e = UNSTUFF_BITS(resp, 47, 3);
            m = UNSTUFF_BITS(resp, 62, 12);
            csd->capacity = (1 + m) << (e + 2);
            mmc_debug_msg ("device_size : %d\n",m);
            mmc_debug_msg ("device_size multiplier: %d\n",e);

            csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
            mmc_debug_msg ("blocklen : %d\n",(1 <<csd->read_blkbits));
        }
        else if(csd_struct == 1)
        {
            mmc_card_set_blockrw(card);
            m = UNSTUFF_BITS(resp, 115, 4);
            e = UNSTUFF_BITS(resp, 112, 3);
            csd->tacc_ns = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
            csd->tacc_clks = UNSTUFF_BITS(resp, 104, 8) * 100;

            m = UNSTUFF_BITS(resp, 99, 4);
            e = UNSTUFF_BITS(resp, 96, 3);
            csd->max_dtr = tran_exp[e] * tran_mant[m];
            csd->cmdclass = UNSTUFF_BITS(resp, 84, 12);

            m = UNSTUFF_BITS(resp, 48, 22);
            csd->capacity = (1 + m) << 10;
            mmc_debug_msg ("device_size : %d\n",m);
            mmc_debug_msg ("device_size multiplier: %d\n",e);

            csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
            mmc_debug_msg ("blocklen : %d\n",(1 <<csd->read_blkbits));
        }
        else
        {
            mmc_card_set_bad(card);
            return;
        }
    }
    else
    {
        /*
         * We only understand CSD structure v1.1 and v1.2.
         * v1.2 has extra information in bits 15, 11 and 10.
         */
        csd_struct = UNSTUFF_BITS(resp, 126, 2);
        if(csd_struct != 1 && csd_struct != 2)
        {
            mmc_card_set_bad(card);
            return;
        }

        csd->mmca_vsn    = UNSTUFF_BITS(resp, 122, 4);
        m = UNSTUFF_BITS(resp, 115, 4);
        e = UNSTUFF_BITS(resp, 112, 3);
        csd->tacc_ns     = (tacc_exp[e] * tacc_mant[m] + 9) / 10;
        csd->tacc_clks   = UNSTUFF_BITS(resp, 104, 8) * 100;

        m = UNSTUFF_BITS(resp, 99, 4);
        e = UNSTUFF_BITS(resp, 96, 3);
        csd->max_dtr      = tran_exp[e] * tran_mant[m];
        csd->cmdclass     = UNSTUFF_BITS(resp, 84, 12);

        e = UNSTUFF_BITS(resp, 47, 3);
        m = UNSTUFF_BITS(resp, 62, 12);
        csd->capacity     = (1 + m) << (e + 2);

        csd->read_blkbits = UNSTUFF_BITS(resp, 80, 4);
    }

    //HACK: For whatever reason, DM320 mmc controller is not able to support
    // block size other than 512, thus block length is hard coded here.
    // ------------------------------------------mgao@neuros 2006-11-10
    {
        csd->capacity <<= (csd->read_blkbits-9);
        csd->read_blkbits = 9;
    }
    // ----------------------------------------------------------end of HACK
}

static void mmc_decode_cid(struct mmc_card *card)
{
    u16 *resp =(unsigned short *) card->raw_cid;

    memset(&card->cid, 0, sizeof(struct mmc_sd_cid));

    if(mmc_card_sd(card))
    {
        /*
         * SD doesn't currently have a version field so we will
         * have to assume we can parse this.
         */
        card->cid.manfid        = UNSTUFF_BITS(resp, 120, 8);
        card->cid.oemid         = UNSTUFF_BITS(resp, 104, 16);
        card->cid.prod_name[0]      = UNSTUFF_BITS(resp, 96, 8);
        card->cid.prod_name[1]      = UNSTUFF_BITS(resp, 88, 8);
        card->cid.prod_name[2]      = UNSTUFF_BITS(resp, 80, 8);
        card->cid.prod_name[3]      = UNSTUFF_BITS(resp, 72, 8);
        card->cid.prod_name[4]      = UNSTUFF_BITS(resp, 64, 8);
        card->cid.hwrev         = UNSTUFF_BITS(resp, 60, 4);
        card->cid.fwrev         = UNSTUFF_BITS(resp, 56, 4);
        card->cid.serial        = UNSTUFF_BITS(resp, 24, 32);
        card->cid.year          = UNSTUFF_BITS(resp, 12, 8);
        card->cid.month         = UNSTUFF_BITS(resp, 8, 4);
        card->cid.year += 2000; /* SD cards year offset */
    }
    else
    {
        /*
         * The selection of the format here is based upon published
         * specs from sandisk and from what people have reported.
         */
        switch(card->csd.mmca_vsn)
        {
        case 0: /* MMC v1.0 - v1.2 */
        case 1: /* MMC v1.4 */
            card->cid.manfid    = UNSTUFF_BITS(resp, 104, 24);
            card->cid.prod_name[0]  = UNSTUFF_BITS(resp, 96, 8);
            card->cid.prod_name[1]  = UNSTUFF_BITS(resp, 88, 8);
            card->cid.prod_name[2]  = UNSTUFF_BITS(resp, 80, 8);
            card->cid.prod_name[3]  = UNSTUFF_BITS(resp, 72, 8);
            card->cid.prod_name[4]  = UNSTUFF_BITS(resp, 64, 8);
            card->cid.prod_name[5]  = UNSTUFF_BITS(resp, 56, 8);
            card->cid.prod_name[6]  = UNSTUFF_BITS(resp, 48, 8);
            card->cid.hwrev     = UNSTUFF_BITS(resp, 44, 4);
            card->cid.fwrev     = UNSTUFF_BITS(resp, 40, 4);
            card->cid.serial    = UNSTUFF_BITS(resp, 16, 24);
            card->cid.month     = UNSTUFF_BITS(resp, 12, 4);
            card->cid.year      = UNSTUFF_BITS(resp, 8, 4) + 1997;
            break;

        case 2: /* MMC v2.0 - v2.2 */
        case 3: /* MMC v3.1 - v3.3 */
            card->cid.manfid    = UNSTUFF_BITS(resp, 120, 8);
            card->cid.oemid     = UNSTUFF_BITS(resp, 104, 16);
            card->cid.prod_name[0]  = UNSTUFF_BITS(resp, 96, 8);
            card->cid.prod_name[1]  = UNSTUFF_BITS(resp, 88, 8);
            card->cid.prod_name[2]  = UNSTUFF_BITS(resp, 80, 8);
            card->cid.prod_name[3]  = UNSTUFF_BITS(resp, 72, 8);
            card->cid.prod_name[4]  = UNSTUFF_BITS(resp, 64, 8);
            card->cid.prod_name[5]  = UNSTUFF_BITS(resp, 56, 8);
            card->cid.serial    = UNSTUFF_BITS(resp, 16, 32);
            card->cid.month     = UNSTUFF_BITS(resp, 12, 4);
            card->cid.year      = UNSTUFF_BITS(resp, 8, 4) + 1997;
            break;

        default:
            mmc_card_set_bad(card);
            break;
        }
    }
}

static void mmc_idle_cards(void)
{
    struct mmc_command cmd;

    cmd.opcode = MMC_GO_IDLE_STATE | 0x0000;
    cmd.arg = 0;
    cmd.flags = MMCSD_RSPNONE;
    mmc_wait_for_cmd(&cmd, 0);
}

static void mmci_set_ios(char bus_width)
{
    volatile u32 reg = 0;

    /* bring the sd/mmc controller to idle state */
    reg = inl (IO_MMC_CONTROL);
    outl(reg | 0x03,IO_MMC_CONTROL);

    if(bus_width == MMC_BUS_WIDTH_4)
    {
        /*changing to 4-data line mode*/
        outl(reg | (1 << 2),IO_MMC_CONTROL); 
    }
    else
    {
        /*changing to 4-data line mode*/
        outl(reg & (~(1 << 2)),IO_MMC_CONTROL);
    }

    /* release idle state */
    reg = inl(IO_MMC_CONTROL);
    outl(reg & 0xFFFC,IO_MMC_CONTROL);
}

struct mmc_card *mmc_discover_cards(unsigned char mode)
{
    struct mmc_card *card;
    unsigned int first_rca = 1, err;
    struct mmc_command cmd;

    mmc_debug_msg("\n");
    cmd.opcode = MMC_ALL_SEND_CID;
    cmd.arg = 0;
    cmd.flags = MMCSD_RSP2; 

    err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);

    if(err != MMC_ERR_NONE)
    {
        mmc_debug_msg("err!= MMC_ERR_NONE\n");
        return NULL;
    }

    mmc_debug_msg ("%s in %d\n",__FUNCTION__,__LINE__);

    card = mmc_alloc_card(cmd.resp, &first_rca);
    if(card==NULL)
    {
        mmc_debug_msg("card==NULL\n");
        return NULL;
    }

    card->state &= ~MMC_STATE_DEAD;
    card->mode = mode;
    if(mode == MMC_MODE_SD)
    {
        mmc_card_set_sd(card);
        cmd.opcode = SD_SEND_RELATIVE_ADDR;
        cmd.arg = 0;
        cmd.flags = MMCSD_RSP1;

        err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);
        if(err != MMC_ERR_NONE)
            mmc_card_set_dead(card);
        else
            card->rca = cmd.resp[7];
    }
    else
    {
        cmd.opcode = MMC_SET_RELATIVE_ADDR;
        cmd.arg = card->rca << 16;
        cmd.flags = MMCSD_RSP6;

        err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);
        if(err != MMC_ERR_NONE)
            mmc_card_set_dead(card);
    }
    return card;
}

static struct mmc_card *
mmc_alloc_card(u16 *raw_cid, unsigned int *frca)
{
    struct mmc_card *card;
    unsigned int rca = *frca;

    card = malloc(sizeof(struct mmc_card));
    if(!card)
        return NULL;
    memset(card, 0, sizeof(struct mmc_card));
    memcpy(card->raw_cid, raw_cid, sizeof(card->raw_cid));

    card->rca = rca;

    return card;
}

static int mmc_send_app_op_cond(u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int err = 0;

    cmd.opcode = SD_APP_OP_COND;
    cmd.arg = ocr;
    cmd.flags = MMCSD_RSP3;

    err = mmc_wait_for_app_cmd(0, &cmd, CMD_RETRIES);

    while(!((cmd.resp[6]|(cmd.resp[7] << 16)) & MMC_CARD_BUSY || ocr == 0))
    {
        if(err != MMC_ERR_NONE)
            break;
        err = mmc_wait_for_app_cmd(0, &cmd, CMD_RETRIES);
    }

    if(rocr)
        *rocr = cmd.resp[6]|(cmd.resp[7] << 16);

    return err;
}

static int mmc_send_op_cond(u32 ocr, u32 *rocr)
{
    struct mmc_command cmd;
    int err = 0;

    cmd.opcode = MMC_SEND_OP_COND;
    cmd.arg = ocr;
    cmd.flags = MMCSD_RSP3;

    err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);

    while(!((cmd.resp[6]|(cmd.resp[7] << 16)) & MMC_CARD_BUSY || ocr == 0))
    {
        if(err != MMC_ERR_NONE)
            break;
        err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);
    }
    if(rocr)
        *rocr = cmd.resp[6]|(cmd.resp[7] << 16);

    return err;
}

int mmc_wait_for_app_cmd(unsigned int rca,
                         struct mmc_command *cmd, int retries)
{
    struct mmc_request mrq;
    struct mmc_command appcmd;
    int i, err;

    err = MMC_ERR_INVALID;

    /*
     * We have to resend MMC_APP_CMD for each attempt so
     * we cannot use the retries field in mmc_command.
     */
    for(i = 0;i <= retries;i++)
    {
        memset(&mrq, 0, sizeof(struct mmc_request));

        appcmd.opcode = MMC_APP_CMD;
        appcmd.arg = rca << 16;
        appcmd.flags = MMCSD_RSP1;
        appcmd.retries = 0;
        memset(appcmd.resp, 0, sizeof(appcmd.resp));

        mrq.cmd = &appcmd;

        mmc_wait_for_req(&mrq);

        if(appcmd.error)
        {
            mmc_debug_msg ("appcmd.error : %d \n",appcmd.error);
            err = appcmd.error;
            continue;
        }

        /* Check that card supported application commands */
        if(!(appcmd.resp[6] & R1_APP_CMD ))/* I m checking for if it can accept an application command */
            return MMC_ERR_FAILED;

        memset(&mrq, 0, sizeof(struct mmc_request));

        memset(cmd->resp, 0, sizeof(cmd->resp));
        cmd->retries = 0;

        mrq.cmd = cmd;

        mmc_wait_for_req(&mrq);

        err = cmd->error;

        if(cmd->error == MMC_ERR_NONE)
            break;
    }

    return err;
}


int mmc_wait_for_cmd(struct mmc_command *cmd, int retries)
{
    int n;
    struct mmc_request mrq;

    memset(&mrq, 0, sizeof(struct mmc_request));
    memset(cmd->resp, 0, sizeof(cmd->resp));

    mrq.cmd = cmd;
    for(n=0; n<=retries; n++)
    {
        if(mmc_wait_for_req(&mrq)!=0)
            break;
    }
    return cmd->error;
}

int mmc_wait_for_req(struct mmc_request *mrq)
{
    int status0=4;

    mmc_start_request(mrq);
    status0 = inl( IO_MMC_STATUS0);
    mmc_debug_msg("status0=%x\n",status0);
    while((status0 & 4) == 0)
    {
        status0 = inl( IO_MMC_STATUS0);
        if(status0 & (MMC_INT_RSP_CRC_ERROR | MMC_INT_RSP_TIMEOUT))
        {
            mmc_cmd_err(mrq->cmd,status0);
            mmc_debug_msg("status0=%x\n",status0);
            return 0;
        }
    }

    it_mmcsd_get_status(mrq->cmd);

    return 1;
}

void mmc_start_request(struct mmc_request *mrq)
{
    mrq->cmd->error = 0;
    mrq->cmd->mrq = mrq;
    mmci_request(mrq);
}

void mmci_start_command(struct mmc_command *cmd,u32 c)
{
    u32 marg;
    u16 cmd1;

    mmc_debug_msg_command ("\nopcode : %d\targ : %x\n",(cmd->opcode & 0x3F),cmd->arg);

    it_mmcsd_clear_response_reg(cmd);
    marg = cmd->arg; 
    cmd1 = cmd->opcode | cmd->flags | 0x0000;   
    outl(marg,IO_MMC_ARG);
    //marg=inl(IO_MMC_ARG);
    outl(cmd1,IO_MMC_COMMAND);
}

void it_mmcsd_clear_response_reg(struct mmc_command *cmd)
{
    outl(0,IO_MMC_RESPONSE01);
    outl(0,IO_MMC_RESPONSE23);
    outl(0,IO_MMC_RESPONSE45);
    outl(0,IO_MMC_RESPONSE67);
    outl(0,IO_MMC_COMMAND_INDEX);
    cmd->resp[0] = 0;
    cmd->resp[1] = 0;
    cmd->resp[2] = 0;
    cmd->resp[3] = 0;
    cmd->resp[4] = 0;
    cmd->resp[5] = 0;
    cmd->resp[6] = 0;
    cmd->resp[7] = 0;
}

static void mmci_request(struct mmc_request *mrq)
{
    mmci_start_command(mrq->cmd, 0);
}

void it_mmcsd_get_status(struct mmc_command *cmd)
{
    cmd->status0 = inl(IO_MMC_STATUS0);
    cmd->status1 = inl(IO_MMC_STATUS1);
    cmd->resp[0] = inl(IO_MMC_RESPONSE01)&0xffff;
    cmd->resp[1] = inl(IO_MMC_RESPONSE01)>>16;
    cmd->resp[2] = inl(IO_MMC_RESPONSE23)&0xffff;
    cmd->resp[3] = inl(IO_MMC_RESPONSE23)>>16;
    cmd->resp[4] = inl(IO_MMC_RESPONSE45)&0xffff;
    cmd->resp[5] = inl(IO_MMC_RESPONSE45)>>16;
    cmd->resp[6] = inl(IO_MMC_RESPONSE67)&0xffff;
    cmd->resp[7] = inl(IO_MMC_RESPONSE67)>>16;
    cmd->cmd_index = inl (IO_MMC_COMMAND_INDEX);
}

void mmc_cmd_err(struct mmc_command*cmd,int status)
{
    if(status & MMC_INT_RSP_TIMEOUT)
    {
        cmd->error = MMC_ERR_TIMEOUT;
        mmc_debug_msg ("Card has not responded \n");
    }
    else if(status & MMC_INT_RSP_CRC_ERROR)
    {
        mmc_debug_msg("CRC ERROR\n");
        cmd->error = MMC_ERR_BADCRC;
    }
}

void change_clk25m(void)
{
    volatile u16 reg = 0;

    mmc_debug_msg ("%s\n",__FUNCTION__);    

    /*diable clock*/
    reg = inl(IO_MMC_MEM_CLK_CONTROL);
    outl(reg & 0xFEFF, IO_MMC_MEM_CLK_CONTROL);
    reg = inl(IO_MMC_MEM_CLK_CONTROL);

    udelay(500);

    /* set mmc clock to ~ 25 MHz */ 
    outl(0x1,IO_MMC_MEM_CLK_CONTROL);

    udelay(500);

    /* enable clock */
    reg = inl(IO_MMC_MEM_CLK_CONTROL);
    outl(reg | 0x0100, IO_MMC_MEM_CLK_CONTROL);
}

static int mmc_select_card(struct mmc_card *card)
{
    int err;
    struct mmc_command cmd;

#ifdef MULTICARD 
    if(card_selected == card)
        return MMC_ERR_NONE;

    card_selected = card;
#endif	
    mmc_debug_msg ("%x\n", card->rca );

    cmd.opcode = MMC_SELECT_CARD;
    cmd.arg = card->rca << 16;
    cmd.flags = MMCSD_RSP1;

    err = mmc_wait_for_cmd(&cmd, CMD_RETRIES);
    if(err != MMC_ERR_NONE)
        return err;

#ifdef MULTICARD    
    /*
     * Default bus width is 1 bit.
     */
    host->ios.bus_width = MMC_BUS_WIDTH_1;

    /*
     * We can only change the bus width of the selected
     * card so therefore we have to put the handling
     * here.
     */
    if(host->caps & MMC_CAP_4_BIT_DATA)
    {
        /*
         * The card is in 1 bit mode by default so
         * we only need to change if it supports the
         * wider version.
         */
        if(mmc_card_sd(card) &&
           (card->scr.bus_widths & SD_SCR_BUS_WIDTH_4))
        {
            struct mmc_command cmd;
            cmd.opcode = SD_APP_SET_BUS_WIDTH;
            cmd.arg = SD_BUS_WIDTH_4;
            cmd.flags = MMCSD_RSP1;

            err = mmc_wait_for_app_cmd(host, card->rca, &cmd,
                                       CMD_RETRIES);
            if(err != MMC_ERR_NONE)
                return err;

            host->ios.bus_width = MMC_BUS_WIDTH_4;
        }
    }

    host->ops->set_ios(host, &host->ios);
#endif
    return MMC_ERR_NONE;
}

/* the max block count read_from_card can read is 0xffff for the mmc block count register is 16 bits len*/
unsigned long read_from_card(int dev,unsigned long start,lbaint_t blkcnt,unsigned long *buffer)
{
    u32 marg;
    u16 cmd1,reg;
    u32 temp_data;
    int err,i;
    char *mem_buffer;
    struct mmc_command cmd;

    mem_buffer=(char*)buffer;
    mmc_debug_msg("dev=%d,start=%d,blkcnt=%d,buffer=%d\n",dev,start,(int)blkcnt,(int)buffer);
    /*disable clock */

    reg = inl(IO_MMC_MEM_CLK_CONTROL);  
    outl(reg & 0xFEFF, IO_MMC_MEM_CLK_CONTROL);

    /*************WORKING******************/
    u32 remain = blkcnt<<mmc_sd_card->csd.read_blkbits;
    if(remain <=0)
        return 0;

    it_mmcsd_clear_response_reg(&cmd);

    outl(1<<mmc_sd_card->csd.read_blkbits,IO_MMC_BLOCK_LENGTH);
    outl(blkcnt,IO_MMC_NR_BLOCKS);

    if(!mmc_card_blockrw(mmc_sd_card))
    {
        cmd.opcode = MMC_SET_BLOCKLEN;
        cmd.arg = 1<<mmc_sd_card->csd.read_blkbits;
        cmd.flags = MMCSD_RSP1;
        err = mmc_wait_for_cmd(&cmd, 5);
        if(err != MMC_ERR_NONE)
        {
            return 0;
        }
    }

    unsigned long len = 0;
    char *buffer_short;

    buffer_short = mem_buffer;

    if(mmc_card_blockrw(mmc_sd_card))
        marg = start;
    else
        marg = start<<mmc_sd_card->csd.read_blkbits;
    cmd1 = MMC_READ_MULTIPLE_BLOCK | 0x0200 | 0x4000 | 0x2000 | 0x0080 | 0x8000;
    outl(marg,IO_MMC_ARG);
    outl(cmd1,IO_MMC_COMMAND);

    do
    {
        it_mmcsd_get_status(&cmd);
        if(cmd.status0 & (MMC_INT_RSP_CRC_ERROR | MMC_INT_RSP_TIMEOUT))
        {
            mmc_cmd_err(&cmd,cmd.status0);
            mmc_debug_msg("status0=%d\n",cmd.status0);
            return 0;
        }
    }while( !((cmd.status0 & 0x4)==4));//checking for busy state and respone of the read command 

    do
    {
        it_mmcsd_get_status(&cmd);
        reg = cmd.status0;
        if(reg & 0x0040)
        {
            printf("Read CRC error\n");
            mmc_debug_msg ("STATUS0 :%x\tSTATUS1 :%x\n",cmd.status0,cmd.status1);
            break;
        }

        if(cmd.status0 & 0x400)
        {
            for(i=0;i<4;i++)
            {
                temp_data = inl (IO_MMC_RX_DATA);
                mmc_debug_msg_rw ("%x\t",temp_data);
                mmc_debug_msg("data=%x\n",temp_data);
#if HOST_BIGENDIAN
                *buffer_short = (temp_data >> 24) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = (temp_data >> 16) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = (temp_data >> 8) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = temp_data & 0xFF;
                buffer_short = buffer_short + 1;
#else
                *((u32*)buffer_short) = temp_data;
                buffer_short += 4;
#endif
            }
            remain-=16;
            mmc_debug_msg ("remain=%d,STATUS0 :%x\tSTATUS1 :%x\n",remain,cmd.status0,cmd.status1);
        }
        else if(cmd.status0 & 0x10)
        {
            mmc_debug_msg("Command Time OUT");
            break;
        }

        if(cmd.status0 & 0x08)
        {
            mmc_debug_msg("Data Time OUT");
            break;
        }
        mmc_debug_msg ("STATUS0 :%x\tSTATUS1 :%x\n",cmd.status0,cmd.status1);
    } while(!(reg & (1)));

    while(remain)
    {
        it_mmcsd_get_status(&cmd);
        if(cmd.status0 & 0x400)
        {
            for(i=0;i<4;i++)
            {
                temp_data = inl (IO_MMC_RX_DATA);
                mmc_debug_msg("data=%x\n",temp_data);
#if HOST_BIGENDIAN
                *buffer_short = (temp_data >> 24) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = (temp_data >> 16) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = (temp_data >> 8) & 0xFF;
                buffer_short = buffer_short + 1;
                *buffer_short = temp_data & 0xFF;
                buffer_short = buffer_short + 1;
#else
                *((u32*)buffer_short) = temp_data;
                buffer_short += 4;
#endif
            }
            remain-=16;
            mmc_debug_msg ("remain=%d,STATUS0 :%x\tSTATUS1 :%x\n",remain,cmd.status0,cmd.status1);
        }
    }

    do
    {
        it_mmcsd_get_status(&cmd);
    }while((cmd.status1 & 0x1));

    len = (unsigned long)buffer_short - (unsigned long)buffer;
    mmc_debug_msg("len=%d\n",len);

    /*************WORKING******************/
    /*Stop command*/
    it_mmcsd_get_status(&cmd);  
    it_mmcsd_clear_response_reg(&cmd);

    marg = 0;
    cmd1 = MMC_STOP_TRANSMISSION | 0x0200 | 0x0100 | 0x0080;
    outl(marg,IO_MMC_ARG);
    outl(cmd1,IO_MMC_COMMAND);

    do
    {
        it_mmcsd_get_status(&cmd);  
        if(cmd.status0 & (MMC_INT_RSP_CRC_ERROR | MMC_INT_RSP_TIMEOUT))
        {
            mmc_cmd_err(&cmd,cmd.status0);
            mmc_debug_msg("status0=%d\n",cmd.status0);
            return 0;
        }
    }while( !((cmd.status0 & 0x4)==4));//checking for busy state and respone of the command

    mmc_debug_msg("respone of the read command \n");

    /* enable clock */
    reg = inl(IO_MMC_MEM_CLK_CONTROL);
    outl(reg | 0x0100, IO_MMC_MEM_CLK_CONTROL);

    return(len>>mmc_sd_card->csd.read_blkbits);
}

unsigned long block_read(int dev, unsigned long start, lbaint_t blkcnt, unsigned long *buffer)
{
    unsigned char *read_buffer = buffer;
    u32 read_start = start;
    u32 blk_cnt = blkcnt;
    u32 read_cnt;
    u32 ret;
    while(blk_cnt)
    {
        if(blk_cnt < MAX_READ_CNT)
            read_cnt = blk_cnt;
        else
            read_cnt = MAX_READ_CNT;
        ret = read_from_card(dev, read_start, read_cnt, read_buffer);
        if(ret == 0)
        {
            return 0; //error
        }
        blk_cnt -= ret;
        read_start += ret;
        read_buffer += ret << mmc_sd_card->csd.read_blkbits;
    }
    return blkcnt;
}

block_dev_desc_t *mmc_get_dev(int dev) 
{ 
    if(mmc_sd_card!=NULL)
        return &mmcsd_dev;
    else
        return NULL;
}

int get_card_type(void)
{
    return card_type;
}

int do_mmc_sd(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{ 
    int err;

    host_configuration();
    mmc_sd_card = mmc_setup();
    if(mmc_sd_card == NULL)
        return -1;
    err = mmc_select_card(mmc_sd_card);

    if(err != MMC_ERR_NONE)   return -1;

    if(mmc_sd_card != NULL)
    {
        mmcsd_dev.dev = 0;
        mmcsd_dev.if_type = IF_TYPE_MMC;   /* type of the interface */
        mmcsd_dev.part_type = PART_TYPE_DOS;   /* partition type */
        mmcsd_dev.type = DEV_TYPE_UNKNOWN;
        mmcsd_dev.removable = 1;   /* removable device */
        mmcsd_dev.block_read = block_read;
        mmcsd_dev.lba = mmc_sd_card->csd.capacity;
        mmcsd_dev.blksz = 1<<mmc_sd_card->csd.read_blkbits;
        printf("card capacity: %dKB\n",(mmc_sd_card->csd.capacity << mmc_sd_card->csd.read_blkbits) / 1024);
    }
    else
    {
        mmcsd_dev.block_read = NULL;
        printf ("No MMC card found\n");
        return -1;
    }
    return 0;
}

U_BOOT_CMD(
          mmc_sd_init,    1,  0,  do_mmc_sd,
          "mmc_sd_init - init mmc/sd card\n",
          NULL
          );

#endif  //CONFIG_CMD_MMCSD
