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

#if defined(CONFIG_CMD_UPDATE_UBOOT)

#include <command.h>

extern int actual_update_uboot(ulong mem_offset, ulong offset_uboot, ulong size_uboot, ulong uboot_body_size);

int do_update_uboot(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	int ret;
	ulong mem_offset, offset, size, body_size;

	if (argc != 5)
	{
		printf("update uboot with error arg\n");
		return -1;
	}
	mem_offset = simple_strtoul(argv[1], NULL, 16);
	offset = simple_strtoul(argv[2], NULL, 16);
	size = simple_strtoul(argv[3], NULL, 16);
	body_size = simple_strtoul(argv[4], NULL, 16);
	ret = actual_update_uboot(mem_offset, offset, size, body_size);
	return ret;
}

U_BOOT_CMD(
		  update_uboot,   5,  1,  do_update_uboot,
		  "for update uboot\n",
		  NULL
		  );

#endif
