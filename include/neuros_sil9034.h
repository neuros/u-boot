#ifndef NEUROS_SIL9034__H
#define NEUROS_SIL9034__H
/*
 *  Copyright(C) 2006-2007 Neuros Technology International LLC. 
 *               <www.neurostechnology.com>
 *
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  This program is distributed in the hope that, in addition to its 
 *  original purpose to support Neuros hardware, it will be useful 
 *  otherwise, but WITHOUT ANY WARRANTY; without even the implied 
 *  warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 ****************************************************************************
 *
 * REVISION:
 * 
 * 1) Initial creation. ----------------------------------- 2008-06-10 JChen
 * 2) Add support for HDMI user interface . --------------- 2008-06-23 JChen
 *
 */

/********************************************/
/* 1 slave i2c address */
#define TX_SLV0							(0x39)
#define SLAVE0							0
/* provides unique vendor identification */
#define VND_IDL							(0x00)
#define VND_IDH							(0x01)
/* provides unique device type identification */
#define DEV_IDL							(0x02)
#define DEV_IDH							(0x03)
/* provides revision info */
#define DEV_REV							(0x04)

/* Software reset register */
#define TX_SWRST_ADDR						(0x05)

/* System control register #1 */
#define TX_SYS_CTRL1_ADDR					(0x08)

/* System status register */
#define TX_STAT_ADDR						(0x09)

/* System contorl register */
#define TX_SYS_CTRL4_ADDR					(0x0C)

/* BKSV register : 5 bytes, Read only (RO), receiver KSV */
#define DDC_BKSV_ADDR						(0x00)

/* AN register : 8 bytes, write only (WO) */
#define DDC_AN_ADDR						 (0x18)

/* AKSV register : 5 bytes, write only (WO) */
#define DDC_AKSV_ADDR						 (0x10)

/* RI register : 2 bytes, read only (R0) ,Ack from receiver R) */
#define DDC_RI_ADDR						 (0x08)

/* DDC fifo register : 1 bytes */
#define DDC_BCAPS_ADDR						 (0x40)


/* HDCP control register */
#define HDCP_CTRL_ADDR						(0x0F)
#define HDCP_RX_SLAVE						(0x74)

/* Data control register */
#define DCTL_ADDR						(0x0D)

/* Hdcp RI register */
#define HDCP_RI1						(0x22)
#define HDCP_RI2						(0x23)
#define HDCP_RI128_COMP						(0x24)

/* Hdcp BKSV register */
#define HDCP_BKSV1_ADDR						(0x10)
#define HDCP_BKSV2_ADDR						(0x11)
#define HDCP_BKSV3_ADDR						(0x12)
#define HDCP_BKSV4_ADDR						(0x13)
#define HDCP_BKSV5_ADDR						(0x14)

/* Hdcp AN register */
#define HDCP_AN1_ADDR						(0x15)
#define HDCP_AN2_ADDR						(0x16)
#define HDCP_AN3_ADDR						(0x17)
#define HDCP_AN4_ADDR						(0x18)
#define HDCP_AN5_ADDR						(0x19)
#define HDCP_AN6_ADDR						(0x1A)
#define HDCP_AN7_ADDR						(0x1B)
#define HDCP_AN8_ADDR						(0x1C)

/* Hdcp AKSV register */
#define HDCP_AKSV1_ADDR						(0x1D)
#define HDCP_AKSV2_ADDR						(0x1E)
#define HDCP_AKSV3_ADDR						(0x1F)
#define HDCP_AKSV4_ADDR						(0x20)
#define HDCP_AKSV5_ADDR						(0x21)

/* Rom command */
#define KEY_COMMAND_ADDR					(0xFA)

/* Hdcp master command */
#define MASTER_CMD_ABORT   					(0x0f)                    // Command Codes
#define MASTER_CMD_CLEAR_FIFO					(0x09)
#define MASTER_CMD_CLOCK					(0x0a)
#define MASTER_CMD_CUR_RD					(0x00)
#define MASTER_CMD_SEQ_RD					(0x02)
#define MASTER_CMD_ENH_RD					(0x04)
#define MASTER_CMD_SEQ_WR					(0x06)

#define MASTER_FIFO_WR_USE					(0x01)
#define MASTER_FIFO_RD_USE					(0x02)
#define MASTER_FIFO_EMPTY					(0x04)
#define MASTER_FIFO_FULL					(0x08)
#define MASTER_DDC_BUSY						(0x10)
#define MASTER_DDC_NOACK					(0x20)
#define MASTER_DDC_STUCK					(0x40)
#define MASTER_DDC_RSVD						(0x80)

#define BIT_MDDC_ST_IN_PROGR					(0x10)
#define BIT_MDDC_ST_I2C_LOW					(0x40)
#define BIT_MDDC_ST_NO_ACK					(0x20)

/* Video Hbit to HSYNC register */
#define HBIT_2HSYNC1						(0x40)
#define HBIT_2HSYNC2						(0x41)

/* Video field Hsync offset register */
#define FIELD2_HSYNC_OFFSETL_ADDR				(0x42)
#define FIELD2_HSYNC_OFFSETH_ADDR				(0x43)

/* Interrupt state Register */
#define INT_STATE_ADDR						(0x70)

/* Interrupt source register */
#define INT_SOURCE1_ADDR					(0x71)
#define INT_SOURCE2_ADDR					(0x72)
#define INT_SOURCE3_ADDR					(0x73)

/* Interrupt unmask register */
#define HDMI_INT1_MASK						(0x75)
#define HDMI_INT2_MASK						(0x76)
#define HDMI_INT3_MASK						(0x77)  // Ri error interrupts masks & DDC FIFO interrupts mask
#define BIT_INT3_RI_ERR_3_MASK					(0x80)  // Enable(1) / disable(0-default) ri err #3 interrupt

/* Interrupt control register */
#define INT_CNTRL_ADDR						(0x79)

/* TMDS control register */
#define TX_TMDS_CCTRL_ADDR					(0x80)
#define TX_TMDS_CTRL_ADDR 					(0x82)
#define TX_TMDS_CTRL2_ADDR					(0x83)
#define TX_TMDS_CTRL3_ADDR					(0x84)
#define TX_TMDS_CTRL4_ADDR					(0x85)

/* Video DE delay register */
#define DE_DELAY_ADDR						(0x32)

/* Video DE control register */
#define DE_CTRL_ADDR						(0x33)

/* Video DE top register */
#define DE_TOP_ADDR						(0x34)

/* Video DE count register */
#define DE_CNTL_ADDR						(0x36)
#define DE_CNTH_ADDR						(0x37)

/* Video DE line register */
#define DEL_L_ADDR						(0x38)
#define DEL_H_ADDR						(0x39)

/* Video resolution register */
#define HRES_L_ADDR 0x3A
#define HRES_H_ADDR 0x3B

/* Video refresh register */
#define VRES_L_ADDR 0x3C
#define VRES_H_ADDR 0x3D

/* Video interlace adjustment register */
#define INTERLACE_ADJ_MODE					(0x3E)

/* Video SYNC polarity detection register */
#define POL_DETECT_ADDR						(0x3F)

/* Video Hbit to HSYNC register */
#define HBIT_TO_HSYNC_ADDR1					(0x40)

/* Video Hbit to HSYNC register */
#define HBIT_TO_HSYNC_ADDR2					(0x41)

/* Video Field2 HSYNC Hight offset register */
#define FIELD2_HSYNC_OFFSETL_ADDR				(0x42)

/* Video Filed2 HSYNC Low offset register */
#define FIELD2_HSYNC_OFFSETH_ADDR				(0x43)

/* Video HSYNC Length register */
#define HLENGTH1_ADDR						(0x44)
#define HLENGTH2_ADDR						(0x45)

/* Video Vbit to VSYNC register */
#define VBIT_TO_VSYNC_ADDR					(0x46)

/* Video VSYNC Length register */
#define VLENGTH_ADDR						(0x47)

/* Video control register */
#define TX_VID_CTRL_ADDR					(0x48)

/* Video action enable register */
#define TX_VID_ACEN_ADDR					(0x49)

/* Video mode register */
#define TX_VID_MODE_ADDR					(0x4A)

/* DDC i2c manual register */
#define DDC_MAN_ADDR						(0xEC)
#define DDC_ADDR						(0xED)
#define DDC_SEGM_ADDR						(0xEE)
#define DDC_OFFSET_ADDR						(0xEF)
#define DDC_CNT1_ADDR						(0xF0)
#define DDC_CNT2_ADDR						(0xF1)
#define DDC_STATUS_ADDR						(0xF2)
#define DDC_CMD_ADDR						(0xF3)
#define DDC_DATA_ADDR						(0xF4)
#define DDC_FIFOCNT_ADDR					(0xF5)




/* end 1 slave i2c address */
/********************************************/



/********************************************/
/* 2 slave i2c address */
#define TX_SLV1							(0x3D)
#define SLAVE1							1

/* ACR control register */
#define ACR_CTRL_ADDR						(0x01)

/* ACR audio frequency register */
#define FREQ_SVAL_ADDR						(0x02)

/* ACR N software value register */
#define N_SVAL1_ADDR						(0x03)
#define N_SVAL2_ADDR						(0x04)
#define N_SVAL3_ADDR						(0x05)


/* Audio IN mode control register */
#define AUD_MODE_ADDR						(0x14)

/* Ri status register */
#define RI_STAT_ADDR						(0x26)

/* RI COMMAND */
#define RI_CMD_ADDR						(0x27)

/* RI Line start register */
#define RI_LINE_START_ADDR					(0x28)

/* RI RX register */
#define RI_RX_L_ADDR						(0x29)
#define RI_RX_H_ADDR						(0x2A)

/* HDMI control register */
#define HDMI_CTRL_ADDR						(0x2F)

/* Diagnostic power down register */
#define DIAG_PD_ADDR						(0x3D)

/* CEA-861 Info Frame register #1*/
#define INF_CTRL1						(0x3E)

/* CEA-861 Info Frame register #2*/
#define INF_CTRL2						(0x3F)

/* General control packet register */
#define GCP_BYTE1						(0xDF)

#define AVI_IF_ADDR 0x40
#define SPD_IF_ADDR  0x60
#define AUD_IF_ADDR 0x80
#define MPEG_IF_ADDR 0xA0
#define GENERIC1_IF_ADDR 0xC0
#define GENERIC2_IF_ADDR 0xE0
#define CP_IF_ADDR   0xDF // Contain Protect 1- byte Frame Info Frame
#define BIT_CP_AVI_MUTE_SET    0x01
#define BIT_CP_AVI_MUTE_CLEAR  0x10

#define GEN_RPT		(1<<0)
#define EN_EN		(1<<1)
#define GCP_RPT		(1<<2)
#define GCP_EN		(1<<3)
#define GEN2_RPT	(1<<4)
#define GEN2_EN		(1<<5)
#define CLR_AVMUTE	0x10
#define SET_AVMUTE	0x01
#define SET_AVMUTE	0x01


/* end 2 slave i2c address */
/********************************************/

/* value defined */
#define		ENABLE						1
#define		TRUE						1
#define		DISABLE						0
#define		FALSE						0
#define		INT_ENABLE					(0x1)
#define		BIT_TX_SW_RST					(0x01)
#define		BIT_TX_FIFO_RST					(0x02)
#define		BIT_AVI_REPEAT					(0x01)
#define		BIT_AVI_ENABLE					(0x02)
#define		BIT_SPD_REPEAT					(0x04)
#define		BIT_SPD_ENABLE					(0x08)
#define		BIT_AUD_REPEAT					(0x10)
#define		BIT_AUD_ENABLE					(0x20)
#define		BIT_MPEG_REPEAT					(0x40)
#define		BIT_MPEG_ENABLE					(0x80)
#define		BIT_GENERIC_REPEAT				(0x01)
#define		BIT_GENERIC_ENABLE				(0x02)
#define		BIT_CP_REPEAT					(0x04)
#define		BIT_CP_ENABLE					(0x08)

#define		AUDIO_IFOFRAMES_EN_RPT				(0x30)


/*******************************HDMI Interface *********************/
#define		DO_NOTHING					0
#define		SWITCH_480P					1
#define		SWITCH_720P					2
#define 	SWITCH_1080I					3
#define		HDCP_EXCHANGE					4
#define		SHOW_REGISTER					5
/*******************************WORK QUEUE *************************/
#define		HDCP_ENABLE					6
#define		HDCP_DISABLE					7
#define		HDCP_RI_STATUS					8
#define		EVENT_NOTIFY					9
/******************************* BIT FIELD *********************/
#define		SET_FIFORTS					(1<<1)
#define 	SET_SWRST					(1<<0)
#define		SET_VSYNC					(1<<6)
#define		SET_VEN						(1<<5)
#define		SET_HEN						(1<<4)
#define		SET_BSEL					(1<<2)
#define		SET_EDGE					(1<<1)
#define		SET_PD						(1<<0)
#define		SET_VLOW					(1<<7)
#define		SET_RSEN					(1<<2)
#define		SET_HPD						(1<<1)
#define		P_STABLE					(1<<0)
#define		SET_PLLF_80UA					(0xf<<1)
#define		SET_PLLF_45UA					(0x8<<1)
#define		SET_PLLF_40UA					(0x7<<1)
#define		SET_PLLF_25UA					(0x4<<1)
#define		SET_PLLF_15UA					(0x2<<1)
#define		SET_PLLF_10UA					(0x1<<1)
#define		SET_PLLF_5UA					(0x0)
#define		PFEN_ENABLE					(0x1)
#define		SET_VID_BLANK					(1<<2)
#define		SET_AUD_MUTE					(1<<1)
#define		BKSV_ERR					(1<<5)
#define		RX_RPTR_ENABLE					(1<<4)
#define		TX_ANSTOP					(1<<3)
#define		SET_CP_RESTN					(1<<2)
#define		SET_ENC_EN					(1<<0)
#define		BCAP_ENABLE					(1<<1)
#define		SET_RI_ENABLE					(1<<0)
#define		SET_RI_DBG_TRASH				(1<<7)
#define		SET_RI_DBG_HOLD					(1<<6)
#define		DE_GEN_ENABLE					(1<<6)
#define		SET_VS_POL_NEG					(1<<5)
#define		SET_HS_POL_NEG					(1<<4)
#define		SET_IFPOL_INVERT				(1<<7)
#define		SET_EXTN_12BIT					(1<<5)
#define		SET_EXTN_8BIT					~(1<<5)
#define		CSCSEL_BT709					(1<<4)
#define		CSCSEL_BT601					~(1<<4)
#define		PIXEL_REP_4					(0x3)
#define		PIXEL_REP_1					(0x1)
#define		PIXEL_NO_REP					(0x0)
#define		SET_WIDE_BUS_12BITS				(0x2<<7)
#define		SET_WIDE_BUS_10BITS				(0x1<<7)
#define		SET_WIDE_BUS_0BITS				(0x0)
#define		CLIP_CS_ID_YCBCR				(1<<4)
#define		CLIP_CS_ID_RGB					~(1<<4)
#define		RANGE_CLIP_ENABLE				(1<<3)
#define		RGB2YCBCR_ENABLE				(1<<2)
#define		RANGE_CMPS_ENABLE				(1<<1)
#define		DOWN_SMPL_ENABLE				(1<<0)
#define		DITHER_ENABLE					(1<<5)
#define		RANGE_ENABLE					(1<<4)
#define		CSC_ENABLE					(1<<3)
#define		UPSMP_ENABLE					(1<<2)
#define		DEMUX_ENABLE					(1<<1)
#define		SYNCEXT_ENABLE					(1<<0)
#define		INTR1_SOFT					(1<<7)
#define		INTR1_HPD					(1<<6)
#define		INTR1_RSEN					(1<<5)
#define		INTR1_DROP_SAMPLE				(1<<4)
#define		INTR1_BI_PHASE_ERR				(1<<3)
#define		INTR1_RI_128					(1<<2)
#define		INTR1_OVER_RUN					(1<<1)
#define		INTR1_UNDER_RUN					(1<<0)
#define		INTR2_BCAP_DONE					(1<<7)
#define		INTR2_SPDIF_PAR					(1<<6)
#define		INTR2_ENC_DIS					(1<<5)
#define		INTR2_PREAM_ERR					(1<<4)
#define		INTR2_CTS_CHG					(1<<3)
#define		INTR2_ACR_OVR					(1<<2)
#define		INTR2_TCLK_STBL					(1<<1)
#define		INTR2_VSYNC_REC					(1<<0)
#define		SOFT_INTR_CLEAR					~(1<<3)
#define		SET_SOFT_INTR					(1<<3)
#define		OPEN_DRAIN_ENABLE				(1<<2)
#define		SET_POLARITY_LOW				(1<<1)
#define		SET_TCLKSEL_40					(0x3<<6)
#define		SET_TCLKSEL_20					(0x2<<6)
#define		SET_TCLKSEL_10					(0x1<<6)
#define		SET_TCLKSEL_05					(0x0<<6)
#define		LVBIAS_ENABLE					(1<<2)
#define		STERM_ENABLE					(1<<0)
#define		SD3_ENABLE					(1<<7)
#define		SD2_ENABLE					(1<<6)
#define		SD1_ENABLE					(1<<5)
#define		SD0_ENABLE					(1<<4)
#define		DSD_ENABLE					(1<<3)
#define		SPDIF_ENABLE					(1<<1)
#define		AUD_ENABLE					(1<<0)
#define		PDIDCK_NORMAL					(1<<2)
#define		PDOSC_NORMAL					(1<<1)
#define		PDTOT_NORMAL					(1<<0)
#define		MPEG_ENABLE					(1<<7)
#define		MPEG_RPT_ENABLE					(1<<6)
#define		CEA861_AUD_ENABLE				(1<<5)
#define		AUD_RPT_ENABLE					(1<<4)
#define		SPD_ENABLE					(1<<3)
#define		SPD_RPT_ENABLE					(1<<2)
#define		AVI_ENABLE					(1<<1)
#define		AVI_RPT_ENABLE					(1<<0)
#define		HDMI_LAYOUT0					~(1<<1)
#define		HDMI_LAYOUT1					(1<<1)
#define		HDMI_MODE_ENABLE				(1<<0)
#define		SET_MAN_OVR					(1<<7)
#define		SET_MAN_SDA					(1<<5)
#define		SET_MAN_SCL					(1<<4)
#define		HDCP_ACC					20
#define		SET_CP_RESTN					(1<<2)
#define		NCTSPKT_ENABLE					(1<<1)
#define		CTS_SEL						(1<<0)
#define		BIT_AVI_EN_REPEAT				0x0003
#define		LD_KSV						(1<<5)
#define		DDC_BIT_REPEATER  				(0x40)
#define		DDC_BIT_FIFO_READY				(0x20)
#define		DDC_BIT_FAST_I2C  				(0x10)
#define		DDC_BSTATUS_ADDR  				(0x41) 
#define		DDC_BSTATUS_1_ADDR				(0x41)
#define		DDC_BSTATUS_2_ADDR				(0x42)
#define		DDC_BIT_HDMI_MODE 				(0x10)
#define		DDC_KSV_FIFO_ADDR 				(0x43)

#endif /* NEUROS_SIL9034__H */
