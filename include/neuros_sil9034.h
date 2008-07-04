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
 * 3) Reorganize the bit field. --------------------------- 2008-07-01 JChen
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

/* HDCP control register */
#define HDCP_CTRL_ADDR						(0x0F)

/* Data control register */
#define DCTL_ADDR						(0x0D)

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
#define GEN_RPT		0x1
#define EN_EN		0x2
#define GCP_RPT		0x4
#define GCP_EN		0x8
#define CLR_AVMUTE	0x10
#define SET_AVMUTE	0x01


/* end 2 slave i2c address */
/********************************************/

/* value defined */
#define		ENABLE						1
#define		DISABLE						0
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
#define		SET_P_STABLE					(1<<0)
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
#define		RX_RPTR_ENABLE					(1<<4)
#define		TX_ANSTOP_ENABLE				(1<<3)
#define		SET_CP_RESTN					(1<<2)
#define		SET_ENC_EN					(1<<0)
#define		BCAP_ENABLE					(1<<1)
#define		RI_ENABLE					(1<<0)
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
#define		AUD_ENABLE					(1<<5)
#define		AUD_RPT_ENABLE					(1<<4)
#define		SPD_ENABLE					(1<<3)
#define		SPD_RPT_ENABLE					(1<<2)
#define		AVI_ENABLE					(1<<1)
#define		AVI_RPT_ENABLE					(1<<0)
#define		HDMI_LAYOUT0					~(1<<1)
#define		HDMI_LAYOUT1					(1<<1)
#define		HDMI_MODE_ENABLE				(1<<0)
#endif /* NEUROS_SIL9034__H */
