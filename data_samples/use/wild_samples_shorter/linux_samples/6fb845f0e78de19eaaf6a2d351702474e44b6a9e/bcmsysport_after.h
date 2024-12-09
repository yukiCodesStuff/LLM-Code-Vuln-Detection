#define __BCM_SYSPORT_H

#include <linux/bitmap.h>
#include <linux/ethtool.h>
#include <linux/if_vlan.h>
#include <linux/net_dim.h>

/* Receive/transmit descriptor format */
	unsigned int		crc_fwd:1;
	u16			rev;
	u32			wolopts;
	u8			sopass[SOPASS_MAX];
	unsigned int		wol_irq_disabled:1;

	/* MIB related fields */
	struct bcm_sysport_mib	mib;