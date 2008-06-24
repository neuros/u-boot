/*
 * (C) Copyright 2001
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

/*
 * Misc functions
 */
#include <common.h>
#include <command.h>

extern void	start_davincifb(char *p);


int do_initfb (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
char *options;

	if (argc > 2)
	{
		printf("Usage: initfb [options]\n");
		return 0;
	}
	else if (argc == 1)
		options = "output=ntsc";
	else
		options = argv[1];

	start_davincifb(options);

	return 0;
}

U_BOOT_CMD(
	initfb,    2,    1,     do_initfb,
	"initfb  - Initialize the framebuffer\n",
	NULL
);
