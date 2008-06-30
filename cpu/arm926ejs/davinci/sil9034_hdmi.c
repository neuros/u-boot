#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>
#include <asm/arch/i2c_defs.h>
#include <neuros_sil9034.h>

/* to enable the debug msg, change the value below to 1 */
#define DEBUG 0

#if DEBUG
#define sil9034_dbg(fmt, args...) printf(fmt, ##args)
#else
#define sil9034_dbg(fmt...)
#endif

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
		error = 1 ;

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

static int sil9034_cea861InfoFrameSetting(void)
{
	u8 avi_info_addr ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	/* set the info frame type according to CEA-861 datasheet */
	avi_info_addr = AVI_IF_ADDR ;

	/* AVI type */
	sil9034_write(SLAVE1,avi_info_addr++,0x82) ;

	/* AVI version */
	sil9034_write(SLAVE1,avi_info_addr++,0x02) ;

	/* AVI length */
	sil9034_write(SLAVE1,avi_info_addr++,0x0D) ;

	/* AVI CRC */
	sil9034_write(SLAVE1,avi_info_addr++,(0x82 + 0x02 + 0x0D + 0x3D)) ;

	/* AVI DATA BYTE , according to Sil FAE, 3 byte is enought.
	 * page 102
	 */
	/* 0 | Y1 | Y0 | A0 | B1 | B0 | S1 | S0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x3D) ;

	/* C1 | C0 | M1 | M0 | R3 | R2 | R1 | R0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x68) ;

	/*  0 | 0 | 0 | 0 | 0 | 0 | SC1 | SC0 */
	sil9034_write(SLAVE1,avi_info_addr++,0x3) ;

	return 0 ;
}

static int sil9034_ddcSetting(void)
{
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
		sil9034_write(SLAVE1,DIAG_PD_ADDR,~(0x7)) ;
		sil9034_write(SLAVE0,TX_SYS_CTRL1_ADDR,reg_value & ~0x1) ;
	}
	else
	{
		sil9034_write(SLAVE1,DIAG_PD_ADDR,0x7) ;
		sil9034_write(SLAVE0,TX_SYS_CTRL1_ADDR,(reg_value | 0x1)) ;
	}
	reg_value = sil9034_read(SLAVE0,TX_SYS_CTRL1_ADDR) ;
	sil9034_dbg("System control register #1 0x%x = 0x%x\n",TX_SYS_CTRL1_ADDR,reg_value) ;

	reg_value = sil9034_read(SLAVE1,DIAG_PD_ADDR) ;
	sil9034_dbg("Diagnostic power down register 0x%x = 0x%x\n",DIAG_PD_ADDR,reg_value) ;
	return 0 ;
}

static int sil9034_swReset(void)
{
	/*
	 * audio fifo reset enable
	 * software reset enable
	 */
	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	sil9034_write(SLAVE0,TX_SWRST_ADDR,(BIT_TX_SW_RST|BIT_TX_FIFO_RST)) ;
	udelay(10000) ;
	sil9034_write(SLAVE0,TX_SWRST_ADDR,~(BIT_TX_SW_RST|BIT_TX_FIFO_RST)) ;
	return 0 ;
}

static int sil9034_generalControlPacket(u8 enable)
{
	u8 reg_value ;
	/*
	 * mute the video & audio
	 */
	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE1,GCP_BYTE1) ;
	if(enable)
	{
		/* set avmute flag */
		sil9034_write(SLAVE1,GCP_BYTE1,(reg_value | SET_AVMUTE)) ;
	}
	else
	{
		/* clear avmute flag */
		sil9034_write(SLAVE1,GCP_BYTE1,(reg_value | CLR_AVMUTE)) ;
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
	sil9034_write(SLAVE1,AUD_MODE_ADDR,0xF9) ;
	reg_value = sil9034_read(SLAVE1,AUD_MODE_ADDR) ;
	sil9034_dbg("Audio in mode register 0x%x = 0x%x\n",AUD_MODE_ADDR,reg_value) ;

	/* ACR audio frequency register: * MCLK=128 Fs */
	sil9034_write(SLAVE1,FREQ_SVAL_ADDR,0) ;
	reg_value = sil9034_read(SLAVE1,FREQ_SVAL_ADDR) ;
	sil9034_dbg("Audio frequency register 0x%x = 0x%x\n",FREQ_SVAL_ADDR,reg_value) ;
	
	return 0 ;
}

static int sil9034_hdmiVideoEmbSyncDec(void)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	reg_value = sil9034_read(SLAVE0,INTERLACE_ADJ_MODE) ;
	sil9034_write(SLAVE0,INTERLACE_ADJ_MODE,(reg_value & ~(0x7))) ;
	reg_value = sil9034_read(SLAVE0,INTERLACE_ADJ_MODE) ;
	sil9034_dbg("Interlace Adjustment register 0x%x = 0x%x\n",INTERLACE_ADJ_MODE,reg_value) ;
	return 0 ;
}

static int sil9034_hdmiOutputConfig(void)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* HDMI control register , enable HDMI, disable DVI */
	reg_value = sil9034_read(SLAVE1,HDMI_CTRL_ADDR) ;
	sil9034_write(SLAVE1,HDMI_CTRL_ADDR,(reg_value | 0x1)) ;
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
	sil9034_write(SLAVE0,TX_TMDS_CTRL_ADDR,reg_value|0x5) ;
	reg_value = sil9034_read(SLAVE0,TX_TMDS_CTRL_ADDR) ;
	sil9034_dbg("TMDS control register 0x%x = 0x%x\n",TX_TMDS_CTRL_ADDR,reg_value) ;
}

static int sil9034_hdmiHdcpConfig(u8 enable)
{
	u8 reg_value ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;
	/* HDMI HDCP configuration */
	reg_value = sil9034_read(SLAVE0,HDCP_CTRL_ADDR) ;
	if(enable)
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,(reg_value | 0x1)) ;
	else
		sil9034_write(SLAVE0,HDCP_CTRL_ADDR,(reg_value & ~(0x7F))) ;

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
	sil9034_write(SLAVE0,DE_CTRL_ADDR,(reg_value|0x41)) ;
	reg_value = sil9034_read(SLAVE0,DE_CTRL_ADDR) ;
	sil9034_dbg("Video DE control register 0x%x = 0x%x\n",DE_CTRL_ADDR,reg_value) ;

	/* Video DE delay register */
	sil9034_write(SLAVE0,DE_DELAY_ADDR,0x2B) ;
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
	sil9034_write(SLAVE0,TX_VID_CTRL_ADDR,(reg_value|0x30)) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_dbg("Video control register 0x%x = 0x%x\n",TX_VID_CTRL_ADDR,reg_value) ;

	/* Video mode register , SYNCEXT=0 DEMUX=0 UPSMP=0 CSC=0 DITHER = 0*/
	/* Jchen: according to datasheet, 0x0 value is true, but Bob's propose
	 * value 0x3C.
	 * sil9034_write(SLAVE0,TX_VID_MODE_ADDR,0x00) ;
	 */
	sil9034_write(SLAVE0,TX_VID_MODE_ADDR,0x3C) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_MODE_ADDR) ;
	sil9034_dbg("Video mode register 0x%x = 0x%x\n",TX_VID_MODE_ADDR,reg_value) ;

	return 0 ;
}

static int sil9034_1080i_VideoInputConfig(void)
{
	/* Output Mode YcbCr 4:2:2 */
	u8 reg_value = 0 ;

	sil9034_dbg("----------%s----------\n",__FUNCTION__) ;

	sil9034_write(SLAVE0,DE_CTRL_ADDR,0x40) ;
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
	sil9034_write(SLAVE0,TX_VID_CTRL_ADDR,(reg_value|0x30)) ;
	reg_value = sil9034_read(SLAVE0,TX_VID_CTRL_ADDR) ;
	sil9034_dbg("Video control register 0x%x = 0x%x\n",TX_VID_CTRL_ADDR,reg_value) ;

	/* Video mode register , SYNCEXT=0 DEMUX=0 UPSMP=0 CSC=0 DITHER = 0*/
	sil9034_write(SLAVE0,TX_VID_MODE_ADDR,0x3C) ;
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
        printf("Silicon Image 9034 initialize.\n");

	/* read chip id & revision */
	sil9034_chipInfo() ;

	/* power down occilator */
	sil9034_powerDown(ENABLE) ;

	/* read flag from env and tune the video input table 
	 * according to DM320 hardware spec */
       	sil9034_720p_VideoInputConfig() ;

	/* Tune the audio input table according to DM320 hardware spec */
	sil9034_audioInputConfig() ;

	/* software reset */
	sil9034_swReset() ;

	/* power up occilator */
	sil9034_powerDown(DISABLE) ;

	/* unmask the interrupt status */
	sil9034_unmaskInterruptStatus() ;

	/* Hdmi output setting */
	sil9034_hdmiOutputConfig() ;

	/* TMDS control register */
	sil9034_hdmiTmdsConfig() ;

	/* ddc master config */
	sil9034_ddcSetting() ;

	/* HDCP control handshaking */
	sil9034_hdmiHdcpConfig(DISABLE) ;

	/* enable the avi repeat transmission */
	sil9034_cea861InfoFrameControl1(ENABLE) ;
	//sil9034_cea861InfoFrameControl2(ENABLE) ;

	/* CEA-861 Info Frame control setting */
	sil9034_cea861InfoFrameSetting() ;

	/* Audio Info Frame control setting */
	sil9034_audioInfoFrameSetting() ;

	sil9034_dumpSystemStatus() ;
	sil9034_dumpDataCtrlStatus() ;
	sil9034_dumpInterruptStateStatus() ;
	sil9034_dumpVideoConfigureStatus() ;

	return 0;
}
