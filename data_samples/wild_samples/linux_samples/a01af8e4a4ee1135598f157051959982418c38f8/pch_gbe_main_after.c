/*
 * Copyright (C) 1999 - 2010 Intel Corporation.
 * Copyright (C) 2010 OKI SEMICONDUCTOR CO., LTD.
 *
 * This code was derived from the Intel e1000e Linux driver.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 2 of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA.
 */

#include "pch_gbe.h"
#include "pch_gbe_api.h"

#define DRV_VERSION     "1.00"
const char pch_driver_version[] = DRV_VERSION;

#define PCI_DEVICE_ID_INTEL_IOH1_GBE	0x8802		/* Pci device ID */
#define PCH_GBE_MAR_ENTRIES		16
#define PCH_GBE_SHORT_PKT		64
#define DSC_INIT16			0xC000
#define PCH_GBE_DMA_ALIGN		0
#define PCH_GBE_WATCHDOG_PERIOD		(1 * HZ)	/* watchdog time */
#define PCH_GBE_COPYBREAK_DEFAULT	256
#define PCH_GBE_PCI_BAR			1

#define PCH_GBE_TX_WEIGHT         64
#define PCH_GBE_RX_WEIGHT         64
#define PCH_GBE_RX_BUFFER_WRITE   16

/* Initialize the wake-on-LAN settings */
#define PCH_GBE_WL_INIT_SETTING    (PCH_GBE_WLC_MP)

#define PCH_GBE_MAC_RGMII_CTRL_SETTING ( \
	PCH_GBE_CHIP_TYPE_INTERNAL | \
	PCH_GBE_RGMII_MODE_RGMII   | \
	PCH_GBE_CRS_SEL              \
	)

/* Ethertype field values */
#define PCH_GBE_MAX_JUMBO_FRAME_SIZE    10318
#define PCH_GBE_FRAME_SIZE_2048         2048
#define PCH_GBE_FRAME_SIZE_4096         4096
#define PCH_GBE_FRAME_SIZE_8192         8192

#define PCH_GBE_GET_DESC(R, i, type)    (&(((struct type *)((R).desc))[i]))
#define PCH_GBE_RX_DESC(R, i)           PCH_GBE_GET_DESC(R, i, pch_gbe_rx_desc)
#define PCH_GBE_TX_DESC(R, i)           PCH_GBE_GET_DESC(R, i, pch_gbe_tx_desc)
#define PCH_GBE_DESC_UNUSED(R) \
	((((R)->next_to_clean > (R)->next_to_use) ? 0 : (R)->count) + \
	(R)->next_to_clean - (R)->next_to_use - 1)

/* Pause packet value */
#define	PCH_GBE_PAUSE_PKT1_VALUE    0x00C28001
#define	PCH_GBE_PAUSE_PKT2_VALUE    0x00000100
#define	PCH_GBE_PAUSE_PKT4_VALUE    0x01000888
#define	PCH_GBE_PAUSE_PKT5_VALUE    0x0000FFFF

#define PCH_GBE_ETH_ALEN            6

/* This defines the bits that are set in the Interrupt Mask
 * Set/Read Register.  Each bit is documented below:
 *   o RXT0   = Receiver Timer Interrupt (ring 0)
 *   o TXDW   = Transmit Descriptor Written Back
 *   o RXDMT0 = Receive Descriptor Minimum Threshold hit (ring 0)
 *   o RXSEQ  = Receive Sequence Error
 *   o LSC    = Link Status Change
 */
#define PCH_GBE_INT_ENABLE_MASK ( \
	PCH_GBE_INT_RX_DMA_CMPLT |    \
	PCH_GBE_INT_RX_DSC_EMP   |    \
	PCH_GBE_INT_WOL_DET      |    \
	PCH_GBE_INT_TX_CMPLT          \
	)


static unsigned int copybreak __read_mostly = PCH_GBE_COPYBREAK_DEFAULT;

static int pch_gbe_mdio_read(struct net_device *netdev, int addr, int reg);
static void pch_gbe_mdio_write(struct net_device *netdev, int addr, int reg,
			       int data);
/**
 * pch_gbe_mac_read_mac_addr - Read MAC address
 * @hw:	            Pointer to the HW structure
 * Returns
 *	0:			Successful.
 */
s32 pch_gbe_mac_read_mac_addr(struct pch_gbe_hw *hw)
{
	u32  adr1a, adr1b;

	adr1a = ioread32(&hw->reg->mac_adr[0].high);
	adr1b = ioread32(&hw->reg->mac_adr[0].low);

	hw->mac.addr[0] = (u8)(adr1a & 0xFF);
	hw->mac.addr[1] = (u8)((adr1a >> 8) & 0xFF);
	hw->mac.addr[2] = (u8)((adr1a >> 16) & 0xFF);
	hw->mac.addr[3] = (u8)((adr1a >> 24) & 0xFF);
	hw->mac.addr[4] = (u8)(adr1b & 0xFF);
	hw->mac.addr[5] = (u8)((adr1b >> 8) & 0xFF);

	pr_debug("hw->mac.addr : %pM\n", hw->mac.addr);
	return 0;
}
{
	pci_unregister_driver(&pch_gbe_pcidev);
}

module_init(pch_gbe_init_module);
module_exit(pch_gbe_exit_module);

MODULE_DESCRIPTION("EG20T PCH Gigabit ethernet Driver");
MODULE_AUTHOR("OKI SEMICONDUCTOR, <toshiharu-linux@dsn.okisemi.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION(DRV_VERSION);
MODULE_DEVICE_TABLE(pci, pch_gbe_pcidev_id);

module_param(copybreak, uint, 0644);
MODULE_PARM_DESC(copybreak,
	"Maximum size of packet that is copied to a new buffer on receive");

/* pch_gbe_main.c */