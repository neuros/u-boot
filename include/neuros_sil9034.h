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


/*******************************HDMI Interface *********************/


#endif /* NEUROS_SIL9034__H */
