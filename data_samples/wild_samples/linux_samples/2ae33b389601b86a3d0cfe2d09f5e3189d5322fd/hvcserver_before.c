/*
 * hvcserver.c
 * Copyright (C) 2004 Ryan S Arnold, IBM Corporation
 *
 * PPC64 virtual I/O console server support.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <asm/hvcall.h>
#include <asm/hvcserver.h>
#include <asm/io.h>

#define HVCS_ARCH_VERSION "1.0.0"

MODULE_AUTHOR("Ryan S. Arnold <rsa@us.ibm.com>");
MODULE_DESCRIPTION("IBM hvcs ppc64 API");
MODULE_LICENSE("GPL");
MODULE_VERSION(HVCS_ARCH_VERSION);

/*
 * Convert arch specific return codes into relevant errnos.  The hvcs
 * functions aren't performance sensitive, so this conversion isn't an
 * issue.
 */
static int hvcs_convert(long to_convert)
{
	switch (to_convert) {
		case H_SUCCESS:
			return 0;
		case H_PARAMETER:
			return -EINVAL;
		case H_HARDWARE:
			return -EIO;
		case H_BUSY:
		case H_LONG_BUSY_ORDER_1_MSEC:
		case H_LONG_BUSY_ORDER_10_MSEC:
		case H_LONG_BUSY_ORDER_100_MSEC:
		case H_LONG_BUSY_ORDER_1_SEC:
		case H_LONG_BUSY_ORDER_10_SEC:
		case H_LONG_BUSY_ORDER_100_SEC:
			return -EBUSY;
		case H_FUNCTION: /* fall through */
		default:
			return -EPERM;
	}
}
		if (!next_partner_info) {
			printk(KERN_WARNING "HVCONSOLE: kmalloc() failed to"
				" allocate partner info struct.\n");
			hvcs_free_partner_info(head);
			return -ENOMEM;
		}

		next_partner_info->unit_address
			= (unsigned int)last_p_unit_address;
		next_partner_info->partition_ID
			= (unsigned int)last_p_partition_ID;

		/* copy the Null-term char too */
		strncpy(&next_partner_info->location_code[0],
			(char *)&pi_buff[2],
			strlen((char *)&pi_buff[2]) + 1);

		list_add_tail(&(next_partner_info->node), head);
		next_partner_info = NULL;

	} while (more);

	return 0;
}
EXPORT_SYMBOL(hvcs_get_partner_info);

/**
 * hvcs_register_connection - establish a connection between this vty-server and
 *	a vty.
 * @unit_address: The unit address of the vty-server adapter that is to be
 *	establish a connection.
 * @p_partition_ID: The partition ID of the vty adapter that is to be connected.
 * @p_unit_address: The unit address of the vty adapter to which the vty-server
 *	is to be connected.
 *
 * If this function is called once and -EINVAL is returned it may
 * indicate that the partner info needs to be refreshed for the
 * target unit address at which point the caller must invoke
 * hvcs_get_partner_info() and then call this function again.  If,
 * for a second time, -EINVAL is returned then it indicates that
 * there is probably already a partner connection registered to a
 * different vty-server adapter.  It is also possible that a second
 * -EINVAL may indicate that one of the parms is not valid, for
 * instance if the link was removed between the vty-server adapter
 * and the vty adapter that you are trying to open.  Don't shoot the
 * messenger.  Firmware implemented it this way.
 */
int hvcs_register_connection( uint32_t unit_address,
		uint32_t p_partition_ID, uint32_t p_unit_address)
{
	long retval;
	retval = plpar_hcall_norets(H_REGISTER_VTERM, unit_address,
				p_partition_ID, p_unit_address);
	return hvcs_convert(retval);
}
EXPORT_SYMBOL(hvcs_register_connection);

/**
 * hvcs_free_connection - free the connection between a vty-server and vty
 * @unit_address: The unit address of the vty-server that is to have its
 *	connection severed.
 *
 * This function is used to free the partner connection between a vty-server
 * adapter and a vty adapter.
 *
 * If -EBUSY is returned continue to call this function until 0 is returned.
 */
int hvcs_free_connection(uint32_t unit_address)
{
	long retval;
	retval = plpar_hcall_norets(H_FREE_VTERM, unit_address);
	return hvcs_convert(retval);
}
EXPORT_SYMBOL(hvcs_free_connection);