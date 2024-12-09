#include <linux/list.h>
#include <linux/module.h>
#include <linux/slab.h>

#include <asm/hvcall.h>
#include <asm/hvcserver.h>
#include <asm/io.h>
			= (unsigned int)last_p_partition_ID;

		/* copy the Null-term char too */
		strncpy(&next_partner_info->location_code[0],
			(char *)&pi_buff[2],
			strlen((char *)&pi_buff[2]) + 1);

		list_add_tail(&(next_partner_info->node), head);
		next_partner_info = NULL;
