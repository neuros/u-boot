#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>
#include <neuros_sil9034.h>

#define mdelay(n) ({unsigned long msec=(n); while (msec--) udelay(1000);})
/* to enable the debug msg, change the value below to 1 */
#define DEBUG 0

#if DEBUG
#define sil9034_dbg(fmt, args...) printf(fmt, ##args)
#else
#define sil9034_dbg(fmt...)
#endif

#define HDCP_HANDSHAKE_RETRY	5

static char an_ksv_data[10] ;
/* sil9034 i2c read function . return error -1, success value*/
static int sil9034_read(u8 slave,u8 reg)
{
	u_int8_t tmp ;

	if(slave == SLAVE0)
	{
		/* slave 1 */
		i2c_read(TX_SLV0,reg,1,&tmp,1) ;
		return(tmp) ;
	}
	else if(slave == SLAVE1)
	{
		/* slave 2 */
		i2c_read(TX_SLV1,reg,1,&tmp,1) ;
		return (tmp) ;
	}
	else
		return -1 ;
}

/* sil9034 i2c write function . return error -1 , success 0*/
static int sil9034_write(u8 slave, u8 reg, uchar value)
{
	int error = 0 ;

	if(slave == SLAVE0)
	{
		/* slave 1 */
		if(i2c_write(TX_SLV0,reg,1,&value,1)==-1)
			error = 1 ;
	}
	else if(slave == SLAVE1)
	{
		/* slave 2 */
		if(i2c_write(TX_SLV1,reg,1,&value,1))
			error = 1 ;
	}
	else
	{
		printf("No such slave\n") ;
		error = 1 ;
	}

	if(error)
	{
		printf("I2c write error\n") ;
		return -1 ;
	}
	else
		return 0 ;
}

static int sil9034_audioInfoFrameSetting(void)
{
	u8 aud_info_addr ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* set the audio info frame type according to EIA-CEA-861-B datasheet */
	aud_info_addr = AUD_IF_ADDR ;

	/* Audio type , pg 78 on EIA_CEA_861_B.pdf */
	sil9034_write(SLAVE1,aud_info_addr++,0x04) ;

	/* Audio version */
	sil9034_write(SLAVE1,aud_info_addr++,0x01) ;

	/* Audio info frame length */
	sil9034_write(SLAVE1,aud_info_addr++,0x10) ;

	/* Audio info frame chsum */
	sil9034_write(SLAVE1,aud_info_addr++,(0x04+0x01+0x10)) ;

	/* AUDIO INFO DATA BYTE , according to Sil FAE, 5 byte is enought.
	 * page 56
	 */
	/* CT3 | CT2 | CT1 | CT0 | Rsvd | CC2 | CC1 | CC0| */
	sil9034_write(SLAVE1,aud_info_addr++,0x11) ;

	/* Reserved (shall be 0) | SF2 | SF1 | SF0 | SS1 | SS0 |*/
	/* I should provide ioctl to re-sampling the frequence according
	 * to audio header type in user space program.
	 */
	sil9034_write(SLAVE1,aud_info_addr++,0x1D) ;

	/* format depend on data byte 1 */
	sil9034_write(SLAVE1,aud_info_addr++,0x11) ;

	/* CA7 | CA6 | CA5 | CA4 | CA3 | CA2 | CA1 | CA0 | */
	sil9034_write(SLAVE1,aud_info_addr++,0) ;

	/* DM_I NH | LSV3 | LSV2 | LSV1 | LSV0 | Reserved (shall be 0)| */
	sil9034_write(SLAVE1,aud_info_addr++,0) ;

	return 0 ;
}

static int sil9034_cea861InfoFrameControl1(u8 enable)
{
	u8 reg_value ;

	/* enable the avi repeat transmission */
	reg_value = sil9034_read(SLAVE1,INF_CTRL1) ;
	if(enable)
		sil9034_write(SLAVE1,INF_CTRL1,(reg_value | (BIT_AVI_REPEAT |BIT_AUD_ENABLE |BIT_AUD_REPEAT))) ;
	else
		sil9034_write(SLAVE1,INF_CTRL1,0) ;
	reg_value = sil9034_read(SLAVE1,INF_CTRL1) ;
	sil9034_dbg("InfoFrame control#1 register 0x%x = 0x%x\n",INF_CTRL1,reg_value) ;


	return 0 ;
}

static int sil9034_cea861InfoFrameControl2(u8 enable)
{
	u8 reg_value ;

	/* Generic packet transmittion & repeat mode enable */
	reg_value = sil9034_read(SLAVE1,INF_CTRL2) ;
	if(enable)
	{
		/* enable GCP_RPT , GCP_EN */
		sil9034_write(SLAVE1,INF_CTRL2,(reg_value |(GCP_EN|GCP_RPT))) ;
	}
	else
	{
		sil9034_write(SLAVE1,INF_CTRL2,reg_value & ~(GCP_EN|GCP_RPT)) ;
	}
	reg_value = sil9034_read(SLAVE1,INF_CTRL2) ;
	sil9034_dbg("InfoFrame control#2 register 0x%x = 0x%x\n",INF_CTRL2,reg_value) ;

	return 0 ;
}

static int sil9034_chipInfo(void)
{
 	u8 device_info[3] = {255,255,255} ;

	device_info[1] = sil9034_read(SLAVE0,DEV_IDL) ;
	device_info[0] = sil9034_read(SLAVE0,DEV_IDH) ;
	device_info[2] = sil9034_read(SLAVE0,DEV_REV) ;
	printf("Silicon Image Device Driver Id 0x%02X%02X. Rev %02i.\n",device_info[0],device_info[1],device_info[2]) ;

	return 0 ;
}
static int sil9034_autoRiCheck(u8 enable)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,RI_CMD_ADDR) ;
	sil9034_write(SLAVE0,RI_CMD_ADDR, reg_value|SET_RI_ENABLE);
	return 0 ;
}

static int sil9034_sentCPPackage(u8 enable)
{
	u8 reg_value ;
	u8 timeout = 64 ;

	reg_value = sil9034_read(SLAVE0,INF_CTRL2) ;
	sil9034_write(SLAVE1,INF_CTRL2,reg_value &~BIT_CP_REPEAT) ;
	if(enable)
		sil9034_write(SLAVE1,CP_IF_ADDR,BIT_CP_AVI_MUTE_SET) ;
	else
		sil9034_write(SLAVE1,CP_IF_ADDR,BIT_CP_AVI_MUTE_CLEAR) ;

	while(timeout--)
	{
		if(!sil9034_read(SLAVE0,INF_CTRL2)&BIT_CP_REPEAT)
			break ;
	}

	if(timeout)
		sil9034_write(SLAVE1,INF_CTRL2,reg_value |(BIT_CP_REPEAT|BIT_CP_ENABLE)) ;

	return 0 ;
}

static int sil9034_wakeupHdmiTx(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,TX_SYS_CTRL1_ADDR) ;
	sil9034_write(SLAVE0,TX_SYS_CTRL1_ADDR,reg_value|SET_PD) ;
	sil9034_write(SLAVE0,INT_CNTRL_ADDR,0) ;
	reg_value = sil9034_read(SLAVE0,INF_CTRL1) ;
	sil9034_write(SLAVE1,INF_CTRL1,reg_value |BIT_AVI_REPEAT|BIT_AUD_REPEAT) ;

	return 0 ;
}

static int sil9034_cea861InfoFrameSetting(void)
{
	u8 avi_info_addr ;
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	reg_value = sil9034_read(SLAVE1,INF_CTRL1) ;
	sil9034_write(SLAVE1,INF_CTRL1,reg_value & (~BIT_AVI_REPEAT)) ;
	mdelay(64) ; // Delay VSync for unlock DATA buffer
	if(sil9034_read(SLAVE1,INF_CTRL1)&BIT_AVI_ENABLE)
		sil9034_dbg("Sent AVI error\n") ;
	else
		sil9034_dbg("Silicon Image sending AVI success.\n") ;

	if(sil9034_read(SLAVE1,INF_CTRL1)&BIT_AUD_ENABLE)
		sil9034_dbg("Sent AUD error\n") ;
	else
		sil9034_dbg("Silicon Image sending AUD success.\n") ;


	/* set the info frame type according to CEA-861 datasheet */
	avi_info_addr = AVI_IF_ADDR ;

	/* AVI type */
	sil9034_write(SLAVE1,avi_info_addr++,0x82) ;

	/* AVI version */
	sil9034_write(SLAVE1,avi_info_addr++,0x02) ;

	/* AVI length */
	sil9034_write(SLAVE1,avi_info_addr++,0x13) ;

	/* AVI CRC */
	sil9034_write(SLAVE1,avi_info_addr++,(0x82 + 0x02 + 0x13 + 0x1D)) ;

	/* AVI DATA BYTE , according to Sil FAE, 3 byte is enought.
	 * page 102
	 */
	/* 0 | Y1 | Y0 | A0 | B1 | B0 | S1 | S0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x1D) ;

	/* C1 | C0 | M1 | M0 | R3 | R2 | R1 | R0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x68) ;

	/*  0 | 0 | 0 | 0 | 0 | 0 | SC1 | SC0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x3) ;

	reg_value = sil9034_read(SLAVE1,INF_CTRL1) ;
	sil9034_write(SLAVE1,INF_CTRL1,reg_value | (BIT_AVI_ENABLE|BIT_AVI_REPEAT)) ;
	return 0 ;
}

static int sil9034_ddcSetting(void)
{
	sil9034_write(SLAVE0,DDC_ADDR,HDCP_RX_SLAVE) ;
	return 0 ;
}

static int sil9034_powerDown(u8 enable)
{
	/* power down internal oscillator
	 * disable internal read of HDCP keys and KSV
	 * disable master DDC block
	 * page 4,50,113
	 */
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,TX_SYS_CTRL1_ADDR) ;
	if(enable)
	{
		sil9034_write(SLAVE1,DIAG_PD_ADDR,PDIDCK_NORMAL|PDOSC_NORMAL|PDTOT_NORMAL) ;
		sil9034_write(SLAVE0,TX_SYS_CTRL1_ADDR,reg_value & ~(SET_PD)) ;
	}
	else
	{
		sil9034_write(SLAVE1,DIAG_PD_ADDR,~(PDIDCK_NORMAL|PDOSC_NORMAL|PDTOT_NORMAL)) ;
		sil9034_write(SLAVE0,TX_SYS_CTRL1_ADDR,(reg_value | SET_PD)) ;
	}
	reg_value = sil9034_read(SLAVE0,TX_SYS_CTRL1_ADDR) ;
	sil9034_dbg("System control register #1 0x%x = 0x%x\n",TX_SYS_CTRL1_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE1,DIAG_PD_ADDR) ;
	sil9034_dbg("Diagnostic power down register 0x%x = 0x%x\n",DIAG_PD_ADDR,reg_value) ;
	return 0 ;
}

static int sil9034_triggerRom(void)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,KEY_COMMAND_ADDR) ;
	sil9034_write(SLAVE0,KEY_COMMAND_ADDR,reg_value & ~LD_KSV) ;
	mdelay(10) ;
	sil9034_write(SLAVE0,KEY_COMMAND_ADDR,reg_value |LD_KSV) ;
	mdelay(10) ;
	sil9034_write(SLAVE0,KEY_COMMAND_ADDR,reg_value & ~LD_KSV) ;
	return 0 ;
}

static int sil9034_swReset(void)
{
	/* use to temporary save inf_ctrl */
	u8 temp1 ;
	u8 temp2 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	temp1 = sil9034_read(SLAVE1,INF_CTRL1) ;
	temp2 = sil9034_read(SLAVE1,INF_CTRL2) ;
	/*
	 * audio fifo reset enable
	 * software reset enable
	 */
	while(!sil9034_read(SLAVE0,TX_STAT_ADDR)&P_STABLE)
		mdelay(10) ;
	sil9034_write(SLAVE0,TX_SWRST_ADDR,(BIT_TX_SW_RST|BIT_TX_FIFO_RST)) ;
	mdelay(10) ;
	sil9034_write(SLAVE0,TX_SWRST_ADDR,0) ;
	mdelay(64) ; // allow TCLK (sent to Rx across the HDMS link) to stabilize

	/* restore */
	sil9034_write(SLAVE1,INF_CTRL1,temp1) ;
	sil9034_write(SLAVE1,INF_CTRL2,temp2) ;
	return 0 ;
}

static char *sil9034_ddc_read(u8 *value,u8 reg, u8 length)
{
	u8 count = 0 ;

	sil9034_write(SLAVE0,DDC_ADDR,HDCP_RX_SLAVE) ;
	sil9034_write(SLAVE0,DDC_OFFSET_ADDR,reg) ;
	sil9034_write(SLAVE0,DDC_CNT1_ADDR,length) ;
	sil9034_write(SLAVE0,DDC_CNT2_ADDR,0) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_CLEAR_FIFO) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_SEQ_RD) ;

	while(sil9034_read(SLAVE0,DDC_STATUS_ADDR)&BIT_MDDC_ST_IN_PROGR)
		mdelay(10) ;

	for(count=0 ;count < length ; count++)
	{
		value[count] = sil9034_read(SLAVE0,DDC_DATA_ADDR) ;
	}

	while(sil9034_read(SLAVE0,DDC_STATUS_ADDR)&BIT_MDDC_ST_IN_PROGR)
		mdelay(10) ;

	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_ABORT) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_CLOCK) ;
	sil9034_write(SLAVE0,DDC_MAN_ADDR,0) ;
	return NULL ;
}
static int sil9034_compareR0(void)
{
	u8 r0rx[2] ;
	u8 r0tx[2] ;

	/* read 2 byte from ddc */
	sil9034_ddc_read(&r0rx[0],DDC_RI_ADDR,2) ;
	r0tx[0] = sil9034_read(SLAVE0,HDCP_RI1) ;
	r0tx[1] = sil9034_read(SLAVE0,HDCP_RI2) ;

	if((r0rx[0]==r0tx[0])&&(r0rx[1]==r0tx[1]))
	{
		printf("HDCP handshake complete match.\n") ;
		return TRUE ;
	}

	return FALSE ;
}
static int sil9034_isRepeater(void)
{
	u8 reg_value ;

	/* read 1 byte from ddc */
	sil9034_ddc_read(&reg_value,DDC_BCAPS_ADDR,1) ;
	if(reg_value&DDC_BIT_REPEATER)
		return TRUE ;

	return FALSE ;
}

static int sil9034_checkHdcpDevice(void)
{
	u8 total = 0 ;
	u8 bits = 0 ;
	u8 count = 0 ;

	/* read 5 byte from ddc */
	sil9034_ddc_read(&an_ksv_data[0],DDC_BKSV_ADDR,5) ;

	/* calculate bits */
	for(count=0 ;count<5 ; count++)
	{
		sil9034_dbg("bksv %d,0x%x\n",count,an_ksv_data[count]) ;
		for(bits=0 ;bits<8 ; bits++)
			if(an_ksv_data[count] & (1<<bits))
				total++ ;
	}

	if(total == HDCP_ACC)
		return TRUE ;
	else
		return FALSE ;
}

static int sil9034_generalControlPacket(u8 enable)
{
	u8 reg_value ;
	/*
	 * mute the video & audio
	 */
	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	if(enable)
	{
		/* set avmute flag */
		sil9034_write(SLAVE1,GCP_BYTE1,SET_AVMUTE) ;
	}
	else
	{
		/* clear avmute flag */
		sil9034_write(SLAVE1,GCP_BYTE1,CLR_AVMUTE) ;
	}
	reg_value = sil9034_read(SLAVE1,GCP_BYTE1) ;
	sil9034_dbg("General control packet register 0x%x = 0x%x\n",GCP_BYTE1,reg_value) ;

	return 0 ;
}

static int sil9034_audioInputConfig(void)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* Audio mode register */
	sil9034_write(SLAVE1,AUD_MODE_ADDR,SPDIF_ENABLE|AUD_ENABLE) ;
	reg_value = sil9034_read(SLAVE1,AUD_MODE_ADDR) ;
	sil9034_dbg("Audio in mode register 0x%x = 0x%x\n",AUD_MODE_ADDR,reg_value) ;

	/* ACR N software value */
	sil9034_write(SLAVE1,N_SVAL1_ADDR,0) ;
	sil9034_write(SLAVE1,N_SVAL2_ADDR,0x18) ;
	sil9034_write(SLAVE1,N_SVAL3_ADDR,0) ;

	/* ACR ctrl */
	sil9034_write(SLAVE1,ACR_CTRL_ADDR,NCTSPKT_ENABLE) ;

	/* ACR audio frequency register: * MCLK=128 Fs */
	sil9034_write(SLAVE1,FREQ_SVAL_ADDR,0x4) ;
	reg_value = sil9034_read(SLAVE1,FREQ_SVAL_ADDR) ;
	sil9034_dbg("Audio frequency register 0x%x = 0x%x\n",FREQ_SVAL_ADDR,reg_value) ;
	
	return 0 ;
}



static int sil9034_hdmiOutputConfig(void)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* HDMI control register , enable HDMI, disable DVI */
	reg_value = sil9034_read(SLAVE1,HDMI_CTRL_ADDR) ;
	sil9034_write(SLAVE1,HDMI_CTRL_ADDR,(reg_value | HDMI_MODE_ENABLE)) ;
	reg_value = sil9034_read(SLAVE1,HDMI_CTRL_ADDR) ;
	sil9034_dbg("Hdmi control register 0x%x = 0x%x\n",HDMI_CTRL_ADDR,reg_value) ;

	return 0 ;
}

static int sil9034_hdmiTmdsConfig(void)
{
	u8 reg_value ;

	/* TMDS control register
	 * FPLL is 1.0*IDCK.
	 * Internal source termination enabled.
	 * Driver level shifter bias enabled.
	 * page 27
	 */
	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL_ADDR) ;
	sil9034_write(SLAVE0,TX_TMDS_CTRL_ADDR,reg_value|(LVBIAS_ENABLE|STERM_ENABLE)) ;
	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL_ADDR) ;
	sil9034_dbg("TMDS control register 0x%x = 0x%x\n",TX_TMDS_CTRL_ADDR,reg_value) ;
}

char sil9034_hotplugEvent(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,TX_STAT_ADDR) ;
	if(reg_value&SET_HPD)
		return 1 ;
	else
		return 0 ;
}

static char *sil9034_ddc_write(u8 *value,u8 reg, u8 length)
{
	u8 count = 0 ;

	while(sil9034_read(SLAVE0,DDC_STATUS_ADDR)&BIT_MDDC_ST_IN_PROGR)
		mdelay(10) ;

	sil9034_write(SLAVE0,DDC_ADDR,HDCP_RX_SLAVE) ;
	sil9034_write(SLAVE0,DDC_OFFSET_ADDR,reg) ;
	sil9034_write(SLAVE0,DDC_CNT1_ADDR,length) ;
	sil9034_write(SLAVE0,DDC_CNT2_ADDR,0) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_CLEAR_FIFO) ;

	for(count=0 ;count < length ; count++)
	{
		sil9034_write(SLAVE0,DDC_DATA_ADDR,value[count]) ;
		sil9034_dbg("DDC write 0x%x\n",value[count]) ;
	}
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_SEQ_WR) ;

	while(sil9034_read(SLAVE0,DDC_STATUS_ADDR)&BIT_MDDC_ST_IN_PROGR)
		mdelay(10) ;

	sil9034_dbg("FIFO is %d\n",sil9034_read(SLAVE0,DDC_FIFOCNT_ADDR)) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_ABORT) ;
	sil9034_write(SLAVE0,DDC_CMD_ADDR,MASTER_CMD_CLOCK) ;
	sil9034_write(SLAVE0,DDC_MAN_ADDR,0) ;
	return NULL ;
}

static int sil9034_writeAnHdcpRx(void)
{ 
	/* write 8 byte to ddc hdcp rx*/
	sil9034_ddc_write(&an_ksv_data[0],DDC_AN_ADDR,8) ;

	return 0 ;
}
static int sil9034_writeBksvHdcpTx(void)
{ 
	u8 count = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	for(count=0; count<5; count++)
	{
		 sil9034_write(SLAVE0,HDCP_BKSV1_ADDR+count,an_ksv_data[count]) ;
		 sil9034_dbg("write bksv to tx 0x%x\n",an_ksv_data[count]) ;
	}

	return 0 ;
}
static int sil9034_readBksvHdcpRx(void)
{ 
	u8 count = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	/* read 5 byte from ddc */
	sil9034_ddc_read(&an_ksv_data[0],DDC_BKSV_ADDR,5) ;
	for(count=0; count<5; count++)
	{
		sil9034_dbg("bksv data %d 0x%x\n",count,an_ksv_data[count]) ;
	}

	return 0 ;
}

static int sil9034_writeAksvHdcpRx(void)
{ 
	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* write 5 byte to ddc hdcp rx*/
	sil9034_ddc_write(&an_ksv_data[0],DDC_AKSV_ADDR,5) ;

	return 0 ;
}

static int sil9034_readAksvHdcpTx(void)
{ 
	u8 count = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	for(count=0; count<5; count++)
	{
		an_ksv_data[count] = sil9034_read(SLAVE0,HDCP_AKSV1_ADDR+count) ;
		sil9034_dbg("aksv data %d 0x%x\n",count,an_ksv_data[count]) ;
	}

	return 0 ;
}

static int sil9034_readAnHdcpTx(void)
{ 
	u8 count = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	for(count=0; count<8; count++)
	{
		an_ksv_data[count] = sil9034_read(SLAVE0,HDCP_AN1_ADDR+count) ;
		sil9034_dbg("an data %d 0x%x\n",count,an_ksv_data[count]) ;
	}

	return 0 ;
}

static int sil9034_generateAn(void)
{ 
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value&~TX_ANSTOP) ;
	mdelay(10) ;
	sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value|TX_ANSTOP) ;

	return 0 ;
}

static int sil9034_StopRepeatBit(void)
{ 
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value&~RX_RPTR_ENABLE) ;
	return 0 ;
}

static int sil9034_releaseCPReset(void)
{ 
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value|SET_CP_RESTN) ;

	return 0 ;
}

static int sil9034_toggleRepeatBit(void)
{ 
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	if(reg_value & RX_RPTR_ENABLE)
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value&~RX_RPTR_ENABLE) ;
	else
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,reg_value|RX_RPTR_ENABLE) ;

	return 0 ;
}

static int sil9034_hdmiHdcpConfig(u8 enable)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* HDMI HDCP configuration */
	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	if(enable)
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,(reg_value | SET_ENC_EN)) ;
	else
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,(reg_value & ~(SET_ENC_EN))) ;

	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	sil9034_dbg("Hdmi hdcp register 0x%x = 0x%x\n",HDCP_CTRL_ADDR,reg_value) ;

	return 0 ;
}

static int sil9034_720p_VideoInputConfig(void)
{
	/* Input Mode YCbCr 4:2:2 Mux YC Separate Syncs */
	/* Output Mode YcbCr 4:2:2 */
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,DE_CTRL_ADDR) ;
	sil9034_write(SLAVE0,DE_CTRL_ADDR,(reg_value|(DE_GEN_ENABLE|0x1))) ;
	reg_value = sil9034_read(SLAVE0,DE_CTRL_ADDR) ;
	sil9034_dbg("Video DE control register 0x%x = 0x%x\n",DE_CTRL_ADDR,reg_value) ;

	/* Video DE delay register */
	sil9034_write(SLAVE0,DE_DELAY_ADDR,0x04) ;
	reg_value = sil9034_read(SLAVE0,DE_DELAY_ADDR) ;
	sil9034_dbg("Video DE delay register 0x%x = 0x%x\n",DE_DELAY_ADDR,reg_value) ;

	/* Video DE top register */
	reg_value = sil9034_read(SLAVE0,DE_TOP_ADDR) ;
	sil9034_write(SLAVE0,DE_TOP_ADDR,reg_value|0x19) ;
	reg_value = sil9034_read(SLAVE0,DE_TOP_ADDR) ;
	sil9034_dbg("Video DE top register 0x%x = 0x%x\n",DE_TOP_ADDR,reg_value) ;

	/* Video DE cnt high byte register */
	reg_value = sil9034_read(SLAVE0,DE_CNTH_ADDR) ;
	sil9034_write(SLAVE0,DE_CNTH_ADDR,(reg_value | 0x5)) ;
	reg_value = sil9034_read(SLAVE0,DE_CNTH_ADDR) ;
	sil9034_dbg("Video DE cnt high register 0x%x = 0x%x\n",DE_CNTH_ADDR,reg_value) ;

	/* Video DE cnt low byte register */
	sil9034_write(SLAVE0,DE_CNTL_ADDR,0x00) ;
	reg_value = sil9034_read(SLAVE0,DE_CNTL_ADDR) ;
	sil9034_dbg("Video DE cnt low register 0x%x = 0x%x\n",DE_CNTL_ADDR,reg_value) ;

	/* Video DE line high byte register */
	sil9034_write(SLAVE0,DEL_H_ADDR,0x2) ;
	reg_value = sil9034_read(SLAVE0,DEL_H_ADDR) ;
	sil9034_dbg("Video DE line high register 0x%x = 0x%x\n",DEL_H_ADDR,reg_value) ;

	/* Video DE line low byte register */
	sil9034_write(SLAVE0,DEL_L_ADDR,0xD0) ;
	reg_value = sil9034_read(SLAVE0,DEL_L_ADDR) ;
	sil9034_dbg("Video DE line high register 0x%x = 0x%x\n",DEL_L_ADDR,reg_value) ;

	/* Video control register , ICLK = 00 EXTN = 1*/
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_write(SLAVE0,TX_VID_CTRL_ADDR,(reg_value|(CSCSEL_BT709|SET_EXTN_12BIT))) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_dbg("Video control register 0x%x = 0x%x\n",TX_VID_CTRL_ADDR,reg_value) ;

	/* Video mode register , SYNCEXT=0 DEMUX=0 UPSMP=1 CSC=1 DITHER = 0*/
	sil9034_write(SLAVE0,TX_VID_MODE_ADDR,(UPSMP_ENABLE|CSC_ENABLE)) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_MODE_ADDR) ;
	sil9034_dbg("Video mode register 0x%x = 0x%x\n",TX_VID_MODE_ADDR,reg_value) ;

	return 0 ;
}

static int sil9034_1080i_VideoInputConfig(void)
{
	/* Output Mode YcbCr 4:2:2 */
	u8 reg_value = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	reg_value = sil9034_read(SLAVE0,DE_CTRL_ADDR) ;
	sil9034_write(SLAVE0,DE_CTRL_ADDR,(reg_value|DE_GEN_ENABLE)) ;
	reg_value = sil9034_read(SLAVE0,DE_CTRL_ADDR) ;
	sil9034_dbg("Video DE control register 0x%x = 0x%x\n",DE_CTRL_ADDR,reg_value) ;

	/* Video DE delay register 0x32*/
	sil9034_write(SLAVE0,DE_DELAY_ADDR,0xC0) ;
	reg_value = sil9034_read(SLAVE0,DE_DELAY_ADDR) ;
	sil9034_dbg("Video DE delay register 0x%x = 0x%x\n",DE_DELAY_ADDR,reg_value) ;

	/* Video DE top register 0x34*/
	reg_value = sil9034_read(SLAVE0,DE_TOP_ADDR) ;
	sil9034_write(SLAVE0,DE_TOP_ADDR,reg_value|0x14) ;
	reg_value = sil9034_read(SLAVE0,DE_TOP_ADDR) ;
	sil9034_dbg("Video DE top register 0x%x = 0x%x\n",DE_TOP_ADDR,reg_value) ;

	/* Video DE cnt high byte register 0x37*/
	reg_value = sil9034_read(SLAVE0,DE_CNTH_ADDR) ;
	sil9034_write(SLAVE0,DE_CNTH_ADDR,(reg_value | 0x7)) ;
	reg_value = sil9034_read(SLAVE0,DE_CNTH_ADDR) ;
	sil9034_dbg("Video DE cnt high register 0x%x = 0x%x\n",DE_CNTH_ADDR,reg_value) ;

	/* Video DE cnt low byte register 0x36*/
	sil9034_write(SLAVE0,DE_CNTL_ADDR,0x80) ;
	reg_value = sil9034_read(SLAVE0,DE_CNTL_ADDR) ;
	sil9034_dbg("Video DE cnt low register 0x%x = 0x%x\n",DE_CNTL_ADDR,reg_value) ;

	/* Video DE line high byte register 0x39*/
	reg_value = sil9034_read(SLAVE0,DEL_H_ADDR) ;
	sil9034_write(SLAVE0,DEL_H_ADDR,(reg_value)|0x2) ;
	reg_value = sil9034_read(SLAVE0,DEL_H_ADDR) ;
	sil9034_dbg("Video DE line high register 0x%x = 0x%x\n",DEL_H_ADDR,reg_value) ;

	/* Video DE line low byte register 0x38*/
	sil9034_write(SLAVE0,DEL_L_ADDR,0x1C) ;
	reg_value = sil9034_read(SLAVE0,DEL_L_ADDR) ;
	sil9034_dbg("Video DE line high register 0x%x = 0x%x\n",DEL_L_ADDR,reg_value) ;

	/* Video control register , ICLK = 00 EXTN = 1*/
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_write(SLAVE0,TX_VID_CTRL_ADDR,(reg_value|SET_EXTN_12BIT)) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_dbg("Video control register 0x%x = 0x%x\n",TX_VID_CTRL_ADDR,reg_value) ;

	/* Video mode register , SYNCEXT=0 DEMUX=0 UPSMP=1 CSC=1 DITHER = 0*/
	sil9034_write(SLAVE0,TX_VID_MODE_ADDR,(UPSMP_ENABLE|CSC_ENABLE)) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_MODE_ADDR) ;
	sil9034_dbg("Video mode register 0x%x = 0x%x\n",TX_VID_MODE_ADDR,reg_value) ;

	return 0 ;
}

int sil9034_dumpSystemStatus(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,TX_STAT_ADDR) ;
	sil9034_dbg("System status register 0x%x = 0x%x\n",TX_STAT_ADDR,reg_value) ;
	return 0 ;
}

int sil9034_dumpDataCtrlStatus(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,DCTL_ADDR) ;
	sil9034_dbg("Data control register 0x%x = 0x%x\n",DCTL_ADDR,reg_value) ;
	return 0 ;
}

int sil9034_unmaskInterruptStatus(void)
{
	u8 reg_value = 0xFF ;

	sil9034_write(SLAVE0,HDMI_INT1_MASK,reg_value) ;
	sil9034_write(SLAVE0,HDMI_INT2_MASK,reg_value) ;
	sil9034_write(SLAVE0,HDMI_INT3_MASK,reg_value) ;

	return 0 ;
}

int sil9034_clearInterruptStatus(void)
{

	/*
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,INT_CNTRL_ADDR) ;
	sil9034_write(SLAVE0,INT_CNTRL_ADDR,reg_value) ;
	reg_value = sil9034_read(SLAVE0,INT_CNTRL_ADDR) ;
	sil9034_dbg("Interrupt control register 0x%x = 0x%x\n",INT_CNTRL_ADDR,reg_value) ;
	*/

	sil9034_write(SLAVE0,INT_SOURCE1_ADDR,0xFF) ;
	sil9034_write(SLAVE0,INT_SOURCE2_ADDR,0xFF) ;
	sil9034_write(SLAVE0,INT_SOURCE3_ADDR,0xFF) ;

	return 0 ;
}

int sil9034_dumpInterruptSourceStatus(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,INT_SOURCE1_ADDR) ;
	sil9034_dbg("Interrupt source 1 register 0x%x = 0x%x\n",INT_SOURCE1_ADDR,reg_value) ;
	reg_value = sil9034_read(SLAVE0,INT_SOURCE2_ADDR) ;
	sil9034_dbg("Interrupt source 2 register 0x%x = 0x%x\n",INT_SOURCE2_ADDR,reg_value) ;
	reg_value = sil9034_read(SLAVE0,INT_SOURCE3_ADDR) ;
	sil9034_dbg("Interrupt source 3 register 0x%x = 0x%x\n",INT_SOURCE3_ADDR,reg_value) ;
	/* Interrupt register will auto clean after read ?*/
	sil9034_clearInterruptStatus() ;

	return 0 ;
}

int sil9034_dumpVideoConfigureStatus(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,HRES_L_ADDR) ;
	sil9034_dbg("H resolution low register 0x%x = 0x%x\n",HRES_L_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,HRES_H_ADDR) ;
	sil9034_dbg("H resolution high register 0x%x = 0x%x\n",HRES_H_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,VRES_L_ADDR) ;
	sil9034_dbg("V resolution low register 0x%x = 0x%x\n",VRES_L_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,VRES_H_ADDR) ;
	sil9034_dbg("V resolution high register 0x%x = 0x%x\n",VRES_H_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,DEL_L_ADDR) ;
	sil9034_dbg("DE line low register 0x%x = 0x%x\n",DEL_L_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,DEL_H_ADDR) ;
	sil9034_dbg("DE line high register 0x%x = 0x%x\n",DEL_H_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,POL_DETECT_ADDR) ;
	sil9034_dbg("Video polarity detect register 0x%x = 0x%x\n",POL_DETECT_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,HLENGTH1_ADDR) ;
	sil9034_dbg("Video HSYNC length1 register 0x%x = 0x%x\n",HLENGTH1_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,HLENGTH2_ADDR) ;
	sil9034_dbg("Video HSYNC length2 register 0x%x = 0x%x\n",HLENGTH2_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,VBIT_TO_VSYNC_ADDR) ;
	sil9034_dbg("Video Vbit to VSync register 0x%x = 0x%x\n",VBIT_TO_VSYNC_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,VLENGTH_ADDR) ;
	sil9034_dbg("Video VSYNC length register 0x%x = 0x%x\n",VLENGTH_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_TMDS_CCTRL_ADDR) ;
	sil9034_dbg("TMDS C control register 0x%x = 0x%x\n",TX_TMDS_CCTRL_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL_ADDR) ;
	sil9034_dbg("TMDS control register 0x%x = 0x%x\n",TX_TMDS_CTRL_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL2_ADDR) ;
	sil9034_dbg("TMDS control #2 register 0x%x = 0x%x\n",TX_TMDS_CTRL2_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL3_ADDR) ;
	sil9034_dbg("TMDS control #3 register 0x%x = 0x%x\n",TX_TMDS_CTRL3_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL4_ADDR) ;
	sil9034_dbg("TMDS control #4 register 0x%x = 0x%x\n",TX_TMDS_CTRL4_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE0,TX_VID_ACEN_ADDR) ;
	sil9034_dbg("Video action enable register 0x%x = 0x%x\n",TX_VID_ACEN_ADDR,reg_value) ;

	return 0 ;
}

int sil9034_dumpInterruptStateStatus(void)
{
	u8 reg_value ;

	reg_value = sil9034_read(SLAVE0,INT_STATE_ADDR) ;
	sil9034_dbg("Interrupt state register 0x%x = 0x%x\n",INT_STATE_ADDR,reg_value) ;
	if(reg_value & INT_ENABLE)
		sil9034_dumpInterruptSourceStatus() ;
	else
		sil9034_dbg("No unmask interrupt\n") ;
	return 0 ;
}

/* sil9034 initialize function */
int sil9034_hdmi_init (void)
{
	static int check_time = 0 ;
        printf("Silicon Image 9034 initialize.\n");

	/* read chip id & revision */
	sil9034_chipInfo() ;

	/* power down occilator */
	sil9034_powerDown(ENABLE) ;

	/* TMDS control register */
	sil9034_hdmiTmdsConfig() ;

	/* Tune the audio input table according to DM320 hardware spec */
	sil9034_audioInputConfig() ;

	/* read flag from env and tune the video input table 
	 * according to DM320 hardware spec */
       	sil9034_720p_VideoInputConfig() ;

	/* software reset */
	sil9034_swReset() ;

	/* Trigger ROM */
	sil9034_triggerRom() ;

	/* Audio Info Frame control setting */
	sil9034_audioInfoFrameSetting() ;
       	sil9034_audioInputConfig() ;

	/* software reset */
	sil9034_swReset() ;

	/* CEA-861 Info Frame control setting */
	sil9034_cea861InfoFrameSetting() ;

	/* Wake up HDMI TX */
	sil9034_wakeupHdmiTx() ;

	/* Sent CP package */
	sil9034_sentCPPackage(ENABLE) ;

	/* unmask the interrupt status */
	sil9034_unmaskInterruptStatus() ;

	/* Hdmi output setting */
	sil9034_hdmiOutputConfig() ;

	/* ddc master config */
	sil9034_ddcSetting() ;

	/* General control packet */
	sil9034_sentCPPackage(DISABLE) ;

	sil9034_cea861InfoFrameControl2(ENABLE) ;

	/* HDCP control handshaking */
	sil9034_hdmiHdcpConfig(ENABLE) ;

	/* On u-boot, we check only 5 time if Hdcp failure. */
	for(check_time=0 ; check_time < HDCP_HANDSHAKE_RETRY ; check_time++)
	{
		if(sil9034_hotplugEvent())
		{
			sil9034_sentCPPackage(ENABLE) ;
			if(sil9034_checkHdcpDevice() == TRUE)
			{
				sil9034_dbg("got 20's 1 from TV\n") ;
				/* Random key */
				sil9034_toggleRepeatBit() ;
				sil9034_releaseCPReset() ;
				sil9034_StopRepeatBit() ;
				sil9034_generateAn() ;
				/* Handshake start */
				sil9034_readAnHdcpTx() ;
				sil9034_writeAnHdcpRx() ;
				sil9034_readAksvHdcpTx() ;
				sil9034_writeAksvHdcpRx() ;
				sil9034_readBksvHdcpRx() ;
				sil9034_writeBksvHdcpTx() ;
				if(sil9034_isRepeater()==TRUE)
					printf("This is repeater,not support.\n") ;
				/* Finally, compare key */
				mdelay(100) ; //delay for R0 calculation
				if(sil9034_compareR0()==FALSE)
					continue ;
				else
				{
					sil9034_sentCPPackage(DISABLE) ;
					sil9034_autoRiCheck(ENABLE) ;
					break ;
				}

			}
			else // no 20 ones and zeros
			{
				/* mute */
				sil9034_sentCPPackage(ENABLE) ;
				sil9034_dbg("TV not send 20's 1,retry!!\n") ;
			}
		}
	}

	sil9034_dumpSystemStatus() ;
	sil9034_dumpDataCtrlStatus() ;
	sil9034_dumpInterruptStateStatus() ;
	sil9034_dumpVideoConfigureStatus() ;

	return 0;
}
