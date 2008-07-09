/*
 * drivers/video/davincifb.c
 *
 * Framebuffer driver for Texas Instruments DaVinci display controller.
 *
 * Copyright (C) 2006 Texas Instruments, Inc.
 * Rishi Bhattacharya <support@ti.com>
 *
 * Leveraged from the framebuffer driver for OMAP24xx
 * written by Andy Lowe (source@mvista.com)
 * Copyright (C) 2004 MontaVista Software, Inc.
 *
 * This file is licensed under the terms of the GNU General Public License
 * version 2. This program is licensed "as is" without any warranty of any
 * kind, whether express or implied.
 */
/* 
 * port into neuros bootloader
 */

#include <common.h>
#include <i2c.h>
#include <asm/arch/hardware.h>

#ifdef IO_ADDRESS
#undef IO_ADDRESS
#define IO_ADDRESS(a) (a)
#endif

#include "davincifb.h"

#define	DBGENTER
#define	DBGEXIT

/* Output Format Selection  */
#define MULTIPLE_BUFFERING	1

#ifdef MULTIPLE_BUFFERING
#define DOUBLE_BUF	2
#define TRIPLE_BUF	3
#else
#define DOUBLE_BUF	1
#define TRIPLE_BUF	1
#endif

#define inl(a)		(*(volatile unsigned int   *)(a))
#define outl(v, a)	(*(volatile unsigned int   *)(a) = (v))

#define KERN_INFO
#define KERN_WARNING

void  davinci_psc_init(void);

/*
 * display controller register I/O routines
 */
static __inline__ u32 dispc_reg_in(u32 offset)
{
	return (inl(offset));
}
static __inline__ u32 dispc_reg_out(u32 offset, u32 val)
{
	outl(val, offset);
	return (val);
}
static __inline__ u32 dispc_reg_merge(u32 offset, u32 val, u32 mask)
{
	u32 addr = offset;
	u32 new_val = (inl(addr) & ~mask) | (val & mask);
	outl(new_val, addr);
	return (new_val);
}

/* There are 4 framebuffers, each represented by an fb_info and
 * a dm_win_info structure */
#define OSD0_FBNAME	"dm_osd0_fb"
#define OSD1_FBNAME	"dm_osd1_fb"
#define VID0_FBNAME	"dm_vid0_fb"
#define VID1_FBNAME	"dm_vid1_fb"

/* usage:	if (is_win(info->fix.id, OSD0)) ... */
#define is_win(name, x) ((strcmp(name, x ## _FBNAME) == 0) ? 1 : 0)

struct dm_win_info {
	/* X and Y position */
	unsigned int x, y;

	/* framebuffer area */
	dma_addr_t fb_base_phys;
	unsigned long fb_base;
	unsigned long fb_size;

	u32 pseudo_palette[17];

	/* flag to identify if framebuffer area is fixed already or not */
	int alloc_fb_mem;
	unsigned long sdram_address;
	struct dm_info *dm;
};

static struct dm_info {
	struct dm_win_info *osd0;
	struct dm_win_info *osd1;
	struct dm_win_info *vid0;
	struct dm_win_info *vid1;

	/* to map the registers */
	dma_addr_t mmio_base_phys;
	unsigned long mmio_base;
	unsigned long mmio_size;

	unsigned long vsync_cnt;
	int timeout;

	/* this is the function that configures the output device (NTSC/PAL/LCD)
	 * for the required output format (composite/s-video/component/rgb)
	 */
	void (*output_device_config) (int on);

	struct device *dev;
} dm_static;
static struct dm_info *dm = &dm_static;

/*JG: In order to support HD resolutions, we should default to the largest resolution
*      supported; this way buffers are large  enough to switch among the different
*      resolutions at run time.  That is, if we want to keep to static buffer methodology
*      implemented thus far  (avoid memory fragmentation ).  For our project, we
*      want to test with 720p, so we will consider this our largest resolution for now.
*
*  TODO: for 1080i support, increase value assigned to following defines appropriately.
*/
// The following apply to NTSC/PAL
#define BASEXVGA  0x0
#define BASEYVGA  0x22
#define DISP_XRESVGA    640
#define DISP_YRESVGA    480
#define BASEXD1      0x80
#define BASEYD1      0x12
#define DISP_XRESD1  720
#define DISP_YRESD1  480
#define DISP_MEMYD1  576

#define BASEX720P 0x50
#define BASEY720P 0x5
#define DISP_XRES720P   1280
#define DISP_YRES720P   720
#define DISP_MEMY720P   720

#define BASEX1080I   0x58
#define BASEY1080I   0x5
#define DISP_XRES1080I   1920
#define DISP_YRES1080I   1088
#define DISP_MEMY1080I   1088

#define BASEXMAX  BASEX1080I
#define BASEYMAX  BASEY1080I
#define DISP_XRESMAX DISP_XRES1080I
#define DISP_YRESMAX DISP_YRES1080I
#define DISP_MEMYMAX DISP_MEMY1080I

/* All window widths have to be rounded up to a multiple of 32 bytes */

/* The OSD0 window has to be always within VID0. Plus, since it is in RGB565
 * mode, it _cannot_ overlap with VID1.
 * For defaults, we are setting the OSD0 window to be displayed in the top
 * left quadrant of the screen, and the VID1 in the bottom right quadrant.
 * So the default 'xres' and 'yres' are set to  half of the screen width and
 * height respectively. Note however that the framebuffer size is allocated
 * for the full screen size so the user can change the 'xres' and 'yres' by
 * using the FBIOPUT_VSCREENINFO ioctl within the limits of the screen size.
 */
#define round_32(width) ((((width) + 31) / 32) * 32 )

#define OSD0_XRES round_32((DISP_XRESMAX)*16/8) * 8/16   /* pixels */
#define OSD0_YRES DISP_YRESMAX
#define OSD0_FB_PHY  0
#define OSD0_FB_SIZE (round_32((DISP_XRESMAX)*16/8) * DISP_MEMYMAX * DOUBLE_BUF)

/* Using the full screen for OSD1 by default */
#define OSD1_XRES round_32(DISP_XRESMAX*4/8) * 8/4 /* pixels */
#define OSD1_YRES DISP_YRESMAX
#define OSD1_FB_PHY  0
#define OSD1_FB_SIZE (round_32(DISP_XRESMAX*4/8) * DISP_MEMYMAX * DOUBLE_BUF)

/* Using the full screen for OSD0 by default */
#define VID0_XRES round_32(DISP_XRESMAX*16/8) * 8/16  /* pixels */
#define VID0_YRES DISP_YRESMAX
#define VID0_FB_PHY  0
#define VID0_FB_SIZE (round_32(DISP_XRESMAX*16/8) * DISP_MEMYMAX * TRIPLE_BUF)

/* Using the bottom right quadrant of the screen screen for VID1 by default,
 * but keeping the framebuffer allocated for the full screen, so the user can
 * change the 'xres' and 'yres' later using the FBIOPUT_VSCREENINFO ioctl.
 */
#define VID1_BPP  16 /* Video1 can be in YUV or RGB888 format */
#define VID1_XRES round_32(DISP_XRESMAX*16/8) * 8/16  /* pixels */
#define VID1_YRES DISP_YRESMAX
#define VID1_FB_PHY  0
#define VID1_FB_SIZE (round_32(DISP_XRESMAX*16/8) * DISP_MEMYMAX * TRIPLE_BUF)

#define	x_pos(w)	((w)->x)
#define	y_pos(w)	((w)->y)

static struct dmparams_t {
	u8 output;
	u8 format;
	u8 DAC;
	u8 windows;		/* bitmap flag based on VID0, VID1, OSD0, OSD1
				 * definitions in header file */
	u32 vid0_xres;
	u32 vid0_yres;
	u32 vid0_xpos;
	u32 vid0_ypos;

	u32 vid1_xres;
	u32 vid1_yres;
	u32 vid1_xpos;
	u32 vid1_ypos;

	u32 osd0_xres;
	u32 osd0_yres;
	u32 osd0_xpos;
	u32 osd0_ypos;

	u32 osd1_xres;
	u32 osd1_yres;
	u32 osd1_xpos;
	u32 osd1_ypos;
} dmparams = {
	NTSC,		/* output */
	COMPOSITE,		/* format */
	DAC_AUTO_DETECT,
	(1 << VID0) | (1 << VID1) | (1 << OSD0) | (1 << OSD1),
	/* windows registered */
	720, 480, 0, 0,	/* vid0 size and position */
	720, 480, 0, 0,	/* vid1 size and position */
	720, 480, 0, 0,	/* osd0 size and position */
	720, 480, 0, 0,	/* osd1 size and position */
};

static void davincifb_ntsc_composite_config(int on);
static void davincifb_pal_composite_config(int on);
static void davincifb_720p_config(int on);
static void davincifb_1080i_config(int on);

/*
** For THS8200
*/
static void autodetectVideoDAC(void)
{
	// Set DAC type to auto detect
	dmparams.DAC = DAC_AUTO_DETECT;

	// start trying to detect of any DACs are available
	// add else if to detect additional DACs
	if (ths8200_is_present())
	{
		//No need to autodetect again next time around
		dmparams.DAC = DAC_THS8200;
	}
}

static void configureVideoDAC(int videoFormat)
{
	if (dmparams.DAC == DAC_AUTO_DETECT)
	{
	     autodetectVideoDAC();
	}

	switch (dmparams.DAC)
	{
	case DAC_AUTO_DETECT:
		//if this is still set to AUTO_DETECT, it means that no supported DAC has been detected
		printf("ERROR: Unable to detect DAC hardware\n");
		break;
	case DAC_THS8200:
		if (ths8200_is_present())
		{
			if (videoFormat == LCD_720P)
			{
				ths8200_set_720P();
			}
			else if (videoFormat == LCD_1080I)
			{
				ths8200_set_1080I();
			}
			else
			{
				printf("ERROR: Unsupported video format for THS8200\n");
			}
		}
		else
		{
			printf("ERROR: THS8200 DAC hardware not present");
		}
		break;
	default:
		printf("ERROR: Unsupported digital DAC specified\n");
		break;
	}
}

static int parse_win_params(char *wp,
			    int *xres, int *yres, int *xpos, int *ypos)
{
	char *s;

	if ((s = strsep(&wp, "x")) == NULL)
		return -1;
	*xres = simple_strtoul(s, NULL, 0);

	if ((s = strsep(&wp, "@")) == NULL)
		return -1;
	*yres = simple_strtoul(s, NULL, 0);

	if ((s = strsep(&wp, ",")) == NULL)
		return -1;
	*xpos = simple_strtoul(s, NULL, 0);

	if ((s = strsep(&wp, ":")) == NULL)
		return -1;
	*ypos = simple_strtoul(s, NULL, 0);

	return 0;
}

/*
 * Pass boot-time options by adding the following string to the boot params:
 * 	video=davincifb:[option[:option]]
 * Valid options:
 * 	output=[lcd|ntsc|pal]
 * 	format=[composite|s-video|component|rgb]
 * 	vid0=[off|MxN@X,Y]
 * 	vid1=[off|MxN@X,Y]
 * 	osd0=[off|MxN@X,Y]
 * 	osd1=[off|MxN@X,Y]
 * 		MxN specify the window resolution (displayed size)
 * 		X,Y specify the window position
 * 		M, N, X, Y are integers
 * 		M, X should be multiples of 16
 */

#ifndef MODULE
int /* __init */ davincifb_setup(char *options)
{
	char *this_opt;
	u32 xres, yres, xpos, ypos;
	int format_yres = 480;

	printf("davincifb: Options \"%s\"\n", options);

	if (!options || !*options)
		return 0;

	dmparams.DAC = DAC_AUTO_DETECT;

	while ((this_opt = strsep(&options, ":")) != NULL) {

		if (!*this_opt)
			continue;

		if (!strncmp(this_opt, "output=", 7)) {
			if (!strncmp(this_opt + 7, "lcd", 3)) {
				dmparams.output = LCD;
				dmparams.format = 0;
			} else if (!strncmp(this_opt + 7, "ntsc", 4))
				dmparams.output = NTSC;
			else if (!strncmp(this_opt + 7, "pal", 3))
				dmparams.output = PAL;
			else if (!strncmp(this_opt + 7, "720p", 4)) {
				dmparams.output = LCD_720P;
				dmparams.format = 0;
			} else if (!strncmp(this_opt + 7, "1080i", 5)) {
				dmparams.output = LCD_1080I;
				dmparams.format = 0;
			}
		} else if (!strncmp(this_opt, "format=", 7)) {
			if (!strncmp(this_opt + 7, "composite", 9))
				dmparams.format = COMPOSITE;
			else if (!strncmp(this_opt + 7, "s-video", 7))
				dmparams.format = SVIDEO;
			else if (!strncmp(this_opt + 7, "component", 9))
				dmparams.format = COMPONENT;
			else if (!strncmp(this_opt + 7, "rgb", 3))
				dmparams.format = RGB;
		} else if (!strncmp(this_opt, "vid0=", 5)) {
			if (!strncmp(this_opt + 5, "off", 3))
				dmparams.windows &= ~(1 << VID0);
			else if (!parse_win_params(this_opt + 5,
						   &xres, &yres, &xpos,
						   &ypos)) {
				dmparams.vid0_xres = xres;
				dmparams.vid0_yres = yres;
				dmparams.vid0_xpos = xpos;
				dmparams.vid0_ypos = ypos;
			}
		} else if (!strncmp(this_opt, "vid1=", 5)) {
			if (!strncmp(this_opt + 5, "off", 3))
				dmparams.windows &= ~(1 << VID1);
			else if (!parse_win_params(this_opt + 5,
						   &xres, &yres, &xpos,
						   &ypos)) {
				dmparams.vid1_xres = xres;
				dmparams.vid1_yres = yres;
				dmparams.vid1_xpos = xpos;
				dmparams.vid1_ypos = ypos;
			}
		} else if (!strncmp(this_opt, "osd0=", 5)) {
			if (!strncmp(this_opt + 5, "off", 3))
				dmparams.windows &= ~(1 << OSD0);
			else if (!parse_win_params(this_opt + 5,
						   &xres, &yres, &xpos,
						   &ypos)) {
				dmparams.osd0_xres = xres;
				dmparams.osd0_yres = yres;
				dmparams.osd0_xpos = xpos;
				dmparams.osd0_ypos = ypos;
			}
		} else if (!strncmp(this_opt, "osd1=", 5)) {
			if (!strncmp(this_opt + 5, "off", 3))
				dmparams.windows &= ~(1 << OSD1);
			else if (!parse_win_params(this_opt + 5,
						   &xres, &yres, &xpos,
						   &ypos)) {
				dmparams.osd1_xres = xres;
				dmparams.osd1_yres = yres;
				dmparams.osd1_xpos = xpos;
				dmparams.osd1_ypos = ypos;
			}
		}
	}
	printf(KERN_INFO "DaVinci: "
	       "Output on %s%s, Enabled windows: %s %s %s %s\n",
	       (dmparams.output == LCD) ? "LCD" :
	       (dmparams.output == NTSC) ? "NTSC" :
	       (dmparams.output == PAL) ? "PAL" :
	       (dmparams.output == LCD_720P) ? "720p" :
	       (dmparams.output == LCD_1080I) ? "1080i" : "unknown device!",
	       (dmparams.format == 0) ? "" :
	       (dmparams.format == COMPOSITE) ? " in COMPOSITE format" :
	       (dmparams.format == SVIDEO) ? " in SVIDEO format" :
	       (dmparams.format == COMPONENT) ? " in COMPONENT format" :
	       (dmparams.format == RGB) ? " in RGB format" : "",
	       (dmparams.windows & (1 << VID0)) ? "Video0" : "",
	       (dmparams.windows & (1 << VID1)) ? "Video1" : "",
	       (dmparams.windows & (1 << OSD0)) ? "OSD0" : "",
	       (dmparams.windows & (1 << OSD1)) ? "OSD1" : "");
	if (dmparams.output == NTSC) {
		format_yres = 480;
	} else if (dmparams.output == PAL) {
		format_yres = 576;
	} else if (dmparams.output == LCD_720P) {
		format_yres = 720;
	} else if (dmparams.output == LCD_1080I) {
		format_yres = 1080;
	} else {
		printf(KERN_INFO
		       "DaVinci:invalid format..defaulting width to 480\n");
	}
	dmparams.osd0_yres = /*osd0_default_var.yres =*/ format_yres;
	dmparams.osd1_yres = /*osd1_default_var.yres =*/ format_yres;
	dmparams.vid0_yres = /*vid0_default_var.yres =*/ format_yres;
	dmparams.vid1_yres = /*vid1_default_var.yres =*/ format_yres;

	if (dmparams.windows & (1 << VID0))
		printf(KERN_INFO "Setting Video0 size %dx%d, "
		       "position (%d,%d)\n",
		       dmparams.vid0_xres, dmparams.vid0_yres,
		       dmparams.vid0_xpos, dmparams.vid0_ypos);
	if (dmparams.windows & (1 << VID1))
		printf(KERN_INFO "Setting Video1 size %dx%d, "
		       "position (%d,%d)\n",
		       dmparams.vid1_xres, dmparams.vid1_yres,
		       dmparams.vid1_xpos, dmparams.vid1_ypos);
	if (dmparams.windows & (1 << OSD0))
		printf(KERN_INFO "Setting OSD0 size %dx%d, "
		       "position (%d,%d)\n",
		       dmparams.osd0_xres, dmparams.osd0_yres,
		       dmparams.osd0_xpos, dmparams.osd0_ypos);
	if (dmparams.windows & (1 << OSD1))
		printf(KERN_INFO "Setting OSD1 size %dx%d, "
		       "position (%d,%d)\n",
		       dmparams.osd1_xres, dmparams.osd1_yres,
		       dmparams.osd1_xpos, dmparams.osd1_ypos);
	return (0);
}
#endif

static void enableDigitalOutput(int bEnable)
{
	if (bEnable)
	{
		// Set PINMUX0 reg to enable LCD (all other settings are kept per u-boot)
		dispc_reg_merge(PINMUX0, PINMUX0_LOEEN, PINMUX0_LOEEN);

		// Set PCR register for FULL clock
		dispc_reg_out(VPBE_PCR, 0);

		// Enable video clock output and non-inverse clock polarity
		dispc_reg_out(VENC_VIDCTL, VENC_VIDCTL_VLCKE);

		// Enabling SYNC pulse width processing, and turning on horizontal and vertical syncs
		//    dispc_reg_out(VENC_SYNCCTL, (VENC_SYNCCTL_SYSW|VENC_SYNCCTL_SYEV|VENC_SYNCCTL_SYEH) );

		// Setting DRGB Matrix registers back to default values
		dispc_reg_out(VENC_DRGBX0, 0x00000400);
		dispc_reg_out(VENC_DRGBX1, 0x00000576);
		dispc_reg_out(VENC_DRGBX2, 0x00000159);
		dispc_reg_out(VENC_DRGBX3, 0x000002cb);
		dispc_reg_out(VENC_DRGBX4, 0x000006ee);


		// Enable DCLOCK
		dispc_reg_out(VENC_DCLKCTL, VENC_DCKCTL_DCKEC);
		// Set DCLOCK pattern
		dispc_reg_out(VENC_DCLKPTN0, 1);
		dispc_reg_out(VENC_DCLKPTN1, 0);
		dispc_reg_out(VENC_DCLKPTN2, 0);
		dispc_reg_out(VENC_DCLKPTN3, 0);
		dispc_reg_out(VENC_DCLKPTN0A, 2);
		dispc_reg_out(VENC_DCLKPTN1A, 0);
		dispc_reg_out(VENC_DCLKPTN2A, 0);
		dispc_reg_out(VENC_DCLKPTN3A, 0);
		dispc_reg_out(VENC_DCLKHS, 0);
		dispc_reg_out(VENC_DCLKHSA, 1);
		dispc_reg_out(VENC_DCLKHR, 0);
		dispc_reg_out(VENC_DCLKVS, 0);
		dispc_reg_out(VENC_DCLKVR, 0);


		// Enable LCD output control (accepting default polarity)
		dispc_reg_out(VENC_LCDOUT, 0x1);

		// Set brightness start position and pulse width to zero
		dispc_reg_out(VENC_BRTS, 0);
		dispc_reg_out(VENC_BRTW, 0);

		// Set LCD AC toggle interval and horizontal position to zero
		dispc_reg_out(VENC_ACCTL, 0);

		// Set PWM period and width to zero
		dispc_reg_out(VENC_PWMP, 0);
		dispc_reg_out(VENC_PWMW, 0);

		// Clear component and composite mode registers (applicable to Analog DACS)
		dispc_reg_out(VENC_CVBS, 0);
		dispc_reg_out(VENC_CMPNT, 0);

		// turning on horizontal and vertical syncs
		dispc_reg_out(VENC_SYNCCTL, (VENC_SYNCCTL_SYEV|VENC_SYNCCTL_SYEH) );

		// Set OSD clock and OSD Sync Adavance registers
		dispc_reg_out(VENC_OSDCLK0, 0);
		dispc_reg_out(VENC_OSDCLK1, 1);
		dispc_reg_out(VENC_OSDHAD, 0);

		// Enable Video Window 0 / disable video window 1
		dispc_reg_out(OSD_VIDWINMD, OSD_VIDWINMD_ACT0);

		// Clear OSD Field Inversion for VID0 Use
		dispc_reg_out(OSD_MODE, 0);

		// Disable OSD0 Window
		dispc_reg_out(OSD_OSDWIN0MD, 0x00002000);

		// Disable OSD1 Window
		dispc_reg_out(OSD_OSDWIN1MD, 0x00008000);

		// set VPSS clock
		dispc_reg_out(VPSS_CLKCTL, 0x0a);

	}
	else
	{
		/* Initialize the VPSS Clock Control register */
		dispc_reg_out(VPSS_CLKCTL, 0x18);

		// Set PINMUX0 reg to enable LCD (all other settings are kept per u-boot)
		dispc_reg_merge(PINMUX0, 0, PINMUX0_LOEEN);
		dispc_reg_merge(PINMUX0, 0, PINMUX0_LFLDEN);

		/* disable VCLK output pin enable */
		dispc_reg_out(VENC_VIDCTL, 0x1101);

		// Disable output sync pins
		dispc_reg_out(VENC_SYNCCTL, 0 );

		// Disable DCLOCK
		dispc_reg_out(VENC_DCLKCTL, 0);
		dispc_reg_out(VENC_DRGBX1, 0x0000057C);

		// Disable LCD output control (accepting default polarity)
		dispc_reg_out(VENC_LCDOUT, 0);
		dispc_reg_out(VENC_CMPNT, 0x100);

		// Enable Video Window 1 / disable video window 0
		dispc_reg_out(OSD_VIDWINMD, 0x302);

		// Enable OSD Field Inversion for VID1 Use
		dispc_reg_out(OSD_MODE, 0x200);

		// Disable OSD0 Window
		dispc_reg_out(OSD_OSDWIN0MD, 0x00002003);

		// Disable OSD1 Window
		dispc_reg_out(OSD_OSDWIN1MD, 0x00008002);

		// Set VID0 window  origin and size
		dispc_reg_out(OSD_VIDWIN0XP, 0);
		dispc_reg_out(OSD_VIDWIN0YP, 0);
		dispc_reg_out(OSD_VIDWIN0XL, 0x2d0);
		dispc_reg_out(OSD_VIDWIN0YL, 0xf0);

		// Set VID1 window  origin and size
		dispc_reg_out(OSD_VIDWIN1XP, 0);
		dispc_reg_out(OSD_VIDWIN1YP, 0);
		dispc_reg_out(OSD_VIDWIN1XL, 0x2d0);
		dispc_reg_out(OSD_VIDWIN1YL, 0xf0);

		// Set OSD0 window  origin and size
		dispc_reg_out(OSD_OSDWIN0XP, 0);
		dispc_reg_out(OSD_OSDWIN0YP, 0);
		dispc_reg_out(OSD_OSDWIN0XL, 0x2d0);
		dispc_reg_out(OSD_OSDWIN0YL, 0xf0);

		// Set OSD1 window  origin and size
		dispc_reg_out(OSD_OSDWIN1XP, 0);
		dispc_reg_out(OSD_OSDWIN1YP, 0);
		dispc_reg_out(OSD_OSDWIN1XL, 0x2d0);
		dispc_reg_out(OSD_OSDWIN1YL, 0xf0);

		// Set OSD1 window  origin and size
		dispc_reg_out(OSD_CURXP, 0);
		dispc_reg_out(OSD_CURYP, 0);
		dispc_reg_out(OSD_CURXL, 0x2d0);
		dispc_reg_out(OSD_CURYL, 0xf0);


		dispc_reg_out(VENC_HSPLS, 0);
		dispc_reg_out(VENC_VSPLS, 0);
		dispc_reg_out(VENC_HINT, 0);
		dispc_reg_out(VENC_HSTART, 0);
		dispc_reg_out(VENC_HVALID, 0);
		dispc_reg_out(VENC_VINT, 0);
		dispc_reg_out(VENC_VSTART, 0);
		dispc_reg_out(VENC_VVALID, 0);
		dispc_reg_out(VENC_HSDLY, 0);
		dispc_reg_out(VENC_VSDLY, 0);
		dispc_reg_out(VENC_YCCCTL, 0);
		dispc_reg_out(VENC_VSTARTA, 0);

		// Set OSD clock and OSD Sync Adavance registers
		dispc_reg_out(VENC_OSDCLK0, 1);
		dispc_reg_out(VENC_OSDCLK1, 2);
	}
}

static void davincifb_720p_config(int on)
{
	DBGENTER;

	/* Reset video encoder module */
	dispc_reg_out(VENC_VMOD, 0);

	// Set new baseX and baseY
	dispc_reg_out(OSD_BASEPX, BASEX720P);
	dispc_reg_out(OSD_BASEPY, BASEY720P);

	enableDigitalOutput(1);
	dispc_reg_merge(PINMUX0, 0, PINMUX0_LFLDEN);

	// Enable OSD0 Window
	dispc_reg_out(OSD_OSDWIN0MD, 0x00002001);

	// Enable OSD1 Window
	dispc_reg_out(OSD_OSDWIN1MD, 0x00008000);

	// Set Timing parameters for 720P frame (must match what THS8200 expects)
	dispc_reg_out(VENC_HSPLS, BASEX720P);
	dispc_reg_out(VENC_VSPLS, BASEY720P);
	dispc_reg_out(VENC_HINT, 1649);
	dispc_reg_out(VENC_HSTART, 300);
	dispc_reg_out(VENC_HVALID, DISP_XRES720P);
	dispc_reg_out(VENC_VINT, 749);
	dispc_reg_out(VENC_VSTART, 26);
	dispc_reg_out(VENC_VVALID, DISP_YRES720P);
	dispc_reg_out(VENC_HSDLY, 0);
	dispc_reg_out(VENC_VSDLY, 0);
	dispc_reg_out(VENC_YCCCTL, 0);
	dispc_reg_out(VENC_VSTARTA, 0);

	// Set VID0 window  origin and size
	dispc_reg_out(OSD_VIDWIN0XP, 220);
	dispc_reg_out(OSD_VIDWIN0YP, 25);
	dispc_reg_out(OSD_VIDWIN0XL, DISP_XRES720P);
	dispc_reg_out(OSD_VIDWIN0YL, DISP_YRES720P);

	// Set VID1 window  origin and size
	dispc_reg_out(OSD_VIDWIN1XP, 220);
	dispc_reg_out(OSD_VIDWIN1YP, 25);
	dispc_reg_out(OSD_VIDWIN1XL, DISP_XRES720P);
	dispc_reg_out(OSD_VIDWIN1YL, DISP_YRES720P);

	// Set OSD0 window  origin and size
	dispc_reg_out(OSD_OSDWIN0XP, 220);
	dispc_reg_out(OSD_OSDWIN0YP, 25);
	dispc_reg_out(OSD_OSDWIN0XL, DISP_XRES720P);
	dispc_reg_out(OSD_OSDWIN0YL, DISP_YRES720P);

	// Set OSD1 window  origin and size
	dispc_reg_out(OSD_OSDWIN1XP, 220);
	dispc_reg_out(OSD_OSDWIN1YP, 25);
	dispc_reg_out(OSD_OSDWIN1XL, DISP_XRES720P);
	dispc_reg_out(OSD_OSDWIN1YL, DISP_YRES720P);

	// Set OSD1 window  origin and size
	dispc_reg_out(OSD_CURXP, 220);
	dispc_reg_out(OSD_CURYP, 25);
	dispc_reg_out(OSD_CURXL, DISP_XRES720P);
	dispc_reg_out(OSD_CURYL, DISP_YRES720P);

	// Enable all VENC, non-standard timing mode, master timing, HD, progressive
	dispc_reg_out(VENC_VMOD, (VENC_VMOD_VENC|VENC_VMOD_VMD | VENC_VMOD_HDMD) );

	configureVideoDAC(LCD_720P);

	DBGEXIT;
}

//JG : TODO:  Need to set appropriate video timing and window sizes for 1080i
static void davincifb_1080i_config(int on)
{
	DBGENTER;

	/* Reset video encoder module */
	dispc_reg_out(VENC_VMOD, 0);

	// Set new baseX and baseY
	dispc_reg_out(OSD_BASEPX, BASEX1080I);
	dispc_reg_out(OSD_BASEPY, BASEY1080I);

	enableDigitalOutput(1);
	dispc_reg_merge(PINMUX0, PINMUX0_LFLDEN, PINMUX0_LFLDEN);

	// Enable OSD0 Window
	dispc_reg_out(OSD_OSDWIN0MD, 0x00002003);

	// Enable OSD1 Window
	dispc_reg_out(OSD_OSDWIN1MD, 0x00008002);

	// Set Timing parameters for 720P frame (must match what THS8200 expects)
	dispc_reg_out(VENC_HSPLS, BASEX1080I);
	dispc_reg_out(VENC_VSPLS, BASEY1080I);
	dispc_reg_out(VENC_HINT, 2200-1);
	dispc_reg_out(VENC_HSTART, 200);
	dispc_reg_out(VENC_HVALID, DISP_XRES1080I);
	dispc_reg_out(VENC_VINT, 1125-1);
	dispc_reg_out(VENC_VSTART, 13);
	dispc_reg_out(VENC_VVALID, DISP_YRES1080I/2);
	dispc_reg_out(VENC_HSDLY, 0);
	dispc_reg_out(VENC_VSDLY, 0);
	dispc_reg_out(VENC_YCCCTL, 0);
	dispc_reg_out(VENC_VSTARTA, 13);

	dispc_reg_out(OSD_VIDWINMD, 0x203);

	// Set VID0 window  origin and size
	dispc_reg_out(OSD_VIDWIN0XP, 200 - BASEX1080I);
	dispc_reg_out(OSD_VIDWIN0YP, 13);
	dispc_reg_out(OSD_VIDWIN0XL, DISP_XRES1080I);
	dispc_reg_out(OSD_VIDWIN0YL, DISP_YRES1080I/2);

	// Set VID1 window  origin and size
	dispc_reg_out(OSD_VIDWIN1XP, 200 - BASEX1080I);
	dispc_reg_out(OSD_VIDWIN1YP, 13);
	dispc_reg_out(OSD_VIDWIN1XL, DISP_XRES1080I);
	dispc_reg_out(OSD_VIDWIN1YL, DISP_YRES1080I/2);

	// Set OSD0 window  origin and size
	dispc_reg_out(OSD_OSDWIN0XP, 200 - BASEX1080I);
	dispc_reg_out(OSD_OSDWIN0YP, 13);
	dispc_reg_out(OSD_OSDWIN0XL, DISP_XRES1080I);
	dispc_reg_out(OSD_OSDWIN0YL, DISP_YRES1080I/2);

	// Set OSD1 window  origin and size
	dispc_reg_out(OSD_OSDWIN1XP, 200 - BASEX1080I);
	dispc_reg_out(OSD_OSDWIN1YP, 13);
	dispc_reg_out(OSD_OSDWIN1XL, DISP_XRES1080I);
	dispc_reg_out(OSD_OSDWIN1YL, DISP_YRES1080I/2);

	// Set OSD1 window  origin and size
	dispc_reg_out(OSD_CURXP, 200 - BASEX1080I);
	dispc_reg_out(OSD_CURYP, 13);
	dispc_reg_out(OSD_CURXL, DISP_XRES1080I);
	dispc_reg_out(OSD_CURYL, DISP_YRES1080I/2);

	// Enable all VENC, non-standard timing mode, master timing, HD, interlaced
	dispc_reg_out(VENC_VMOD, (VENC_VMOD_VENC|VENC_VMOD_VMD | VENC_VMOD_HDMD | VENC_VMOD_NSIT) );

	configureVideoDAC(LCD_1080I);

	DBGEXIT;
}

static void davincifb_ntsc_composite_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and NTSC  */
		dispc_reg_out(VENC_VMOD, 0x1003);

		/* Configure DACs for composite video */
		dispc_reg_out(VENC_DACSEL, 0);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

static void davincifb_ntsc_svideo_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and NTSC  */
		dispc_reg_out(VENC_VMOD, 0x1003);

		/* Enable S-Video Output; DAC B: S-Video Y, DAC C: S-Video C  */
		dispc_reg_out(VENC_DACSEL, 0x210);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

static void davincifb_ntsc_component_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and NTSC  */
		dispc_reg_out(VENC_VMOD, 0x1003);

		/* Enable Component output; DAC A: Y, DAC B: Pb, DAC C: Pr  */
		dispc_reg_out(VENC_DACSEL, 0x543);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

static void davincifb_pal_composite_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and PAL  */
		dispc_reg_out(VENC_VMOD, 0x1043);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

static void davincifb_pal_svideo_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and PAL  */
		dispc_reg_out(VENC_VMOD, 0x1043);

		/* Enable S-Video Output; DAC B: S-Video Y, DAC C: S-Video C  */
		dispc_reg_out(VENC_DACSEL, 0x210);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

static void davincifb_pal_component_config(int on)
{
	if (on) {

		/* Set Base Pixel X and Base Pixel Y */
		dispc_reg_out(OSD_BASEPX, BASEXD1);
		dispc_reg_out(OSD_BASEPY, BASEYD1);

		enableDigitalOutput(0);

		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);

		/* Enable Composite output and start video encoder */
		dispc_reg_out(VENC_VMOD, (VENC_VMOD_VIE | VENC_VMOD_VENC));

		/* Set REC656 Mode */
		dispc_reg_out(VENC_YCCCTL, 0x1);

		/* Enable output mode and PAL  */
		dispc_reg_out(VENC_VMOD, 0x1043);

		/* Enable Component output; DAC A: Y, DAC B: Pb, DAC C: Pr  */
		dispc_reg_out(VENC_DACSEL, 0x543);

		/* Enable all DACs  */
		dispc_reg_out(VENC_DACTST, 0);
	} else {
		/* Reset video encoder module */
		dispc_reg_out(VENC_VMOD, 0);
	}
}

/*
 *  Initialization
 */
static int davincifb_probe(/*struct platform_device *pdev*/ void)
{
	if (dmparams.windows == 0)
		return 0;	/* user disabled all windows through bootargs */

	dm->mmio_base_phys = OSD_REG_BASE;
	dm->mmio_size = OSD_REG_SIZE;

	if ((dmparams.output == NTSC) && (dmparams.format == COMPOSITE))
		dm->output_device_config = davincifb_ntsc_composite_config;
	else if ((dmparams.output == NTSC) && (dmparams.format == SVIDEO))
		dm->output_device_config = davincifb_ntsc_svideo_config;
	else if ((dmparams.output == NTSC) && (dmparams.format == COMPONENT))
		dm->output_device_config = davincifb_ntsc_component_config;
	else if ((dmparams.output == PAL) && (dmparams.format == COMPOSITE))
		dm->output_device_config = davincifb_pal_composite_config;
	else if ((dmparams.output == PAL) && (dmparams.format == SVIDEO))
		dm->output_device_config = davincifb_pal_svideo_config;
	else if ((dmparams.output == PAL) && (dmparams.format == COMPONENT))
		dm->output_device_config = davincifb_pal_component_config;
	else if (dmparams.output == LCD_720P)
		dm->output_device_config = davincifb_720p_config;
	else if (dmparams.output == LCD_1080I)
		dm->output_device_config = davincifb_1080i_config;
	/* Add support for other displays here */
	else {
		printf(KERN_WARNING "Unsupported output device!\n");
		dm->output_device_config = NULL;
	}

	printf("Setting Up Clocks for DM420 OSD\n");

	/* Reset OSD registers to default. */
	dispc_reg_out(OSD_MODE, 0);
	dispc_reg_out(OSD_OSDWIN0MD, 0);

	/* Field Inversion Workaround */
	dispc_reg_out(OSD_MODE, 0x200);

	/* --------------V Bruno: enable color bars V--------------- */
	dispc_reg_out(VENC_VDPRO, 0x100);
	/* --------------^ Bruno: enable color bars V--------------- */

	/* Setup VID0 framebuffer */
	if (!(dmparams.windows & (1 << VID0))) {
		printf(KERN_WARNING "No video/osd windows will be enabled "
		       "because Video0 is disabled\n");
		return 0;	/* background will still be shown */
	}

	/* --------------V Bruno V--------------- */
	dispc_reg_out(OSD_VIDWIN1ADR, 0x82000000);		/* SDARM address */
	dispc_reg_out(OSD_VIDWIN1OFST, 45);			/* line len */
	dispc_reg_merge(OSD_OSDWIN0MD, 0, OSD_OSDWIN0MD_OACT0);	/* Disable OSD0 win */
	dispc_reg_merge(OSD_VIDWINMD, ~0, OSD_VIDWINMD_ACT1);	/* Enable Vid win 1 */
	/* --------------^ Bruno ^--------------- */

	/* Turn ON the output device */
	dm->output_device_config(1);	// Removed for THS8200?

	return (0);
}

void start_davincifb(char *options)
{
	davinci_psc_init();
	davincifb_setup(options);
	davincifb_probe();
}
