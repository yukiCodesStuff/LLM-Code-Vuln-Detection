/*
 * Cadence MACB/GEM Ethernet Controller driver
 *
 * Copyright (C) 2004-2006 Atmel Corporation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt
#include <linux/clk.h>
#include <linux/crc32.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/circ_buf.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/dma-mapping.h>
#include <linux/platform_data/macb.h>
#include <linux/platform_device.h>
#include <linux/phy.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_mdio.h>
#include <linux/of_net.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include "macb.h"

#define MACB_RX_BUFFER_SIZE	128
#define RX_BUFFER_MULTIPLE	64  /* bytes */

#define DEFAULT_RX_RING_SIZE	512 /* must be power of 2 */
#define MIN_RX_RING_SIZE	64
#define MAX_RX_RING_SIZE	8192
#define RX_RING_BYTES(bp)	(macb_dma_desc_get_size(bp)	\
				 * (bp)->rx_ring_size)

#define DEFAULT_TX_RING_SIZE	512 /* must be power of 2 */
#define MIN_TX_RING_SIZE	64
#define MAX_TX_RING_SIZE	4096
#define TX_RING_BYTES(bp)	(macb_dma_desc_get_size(bp)	\
				 * (bp)->tx_ring_size)

/* level of occupied TX descriptors under which we wake up TX process */
#define MACB_TX_WAKEUP_THRESH(bp)	(3 * (bp)->tx_ring_size / 4)

#define MACB_RX_INT_FLAGS	(MACB_BIT(RCOMP) | MACB_BIT(RXUBR)	\
				 | MACB_BIT(ISR_ROVR))
#define MACB_TX_ERR_FLAGS	(MACB_BIT(ISR_TUND)			\
					| MACB_BIT(ISR_RLE)		\
					| MACB_BIT(TXERR))
#define MACB_TX_INT_FLAGS	(MACB_TX_ERR_FLAGS | MACB_BIT(TCOMP)	\
					| MACB_BIT(TXUBR))

/* Max length of transmit frame must be a multiple of 8 bytes */
#define MACB_TX_LEN_ALIGN	8
#define MACB_MAX_TX_LEN		((unsigned int)((1 << MACB_TX_FRMLEN_SIZE) - 1) & ~((unsigned int)(MACB_TX_LEN_ALIGN - 1)))
#define GEM_MAX_TX_LEN		((unsigned int)((1 << GEM_TX_FRMLEN_SIZE) - 1) & ~((unsigned int)(MACB_TX_LEN_ALIGN - 1)))

#define GEM_MTU_MIN_SIZE	ETH_MIN_MTU
#define MACB_NETIF_LSO		NETIF_F_TSO

#define MACB_WOL_HAS_MAGIC_PACKET	(0x1 << 0)
#define MACB_WOL_ENABLED		(0x1 << 1)

/* Graceful stop timeouts in us. We should allow up to
 * 1 frame time (10 Mbits/s, full-duplex, ignoring collisions)
 */
#define MACB_HALT_TIMEOUT	1230

/* DMA buffer descriptor might be different size
 * depends on hardware configuration:
 *
 * 1. dma address width 32 bits:
 *    word 1: 32 bit address of Data Buffer
 *    word 2: control
 *
 * 2. dma address width 64 bits:
 *    word 1: 32 bit address of Data Buffer
 *    word 2: control
 *    word 3: upper 32 bit address of Data Buffer
 *    word 4: unused
 *
 * 3. dma address width 32 bits with hardware timestamping:
 *    word 1: 32 bit address of Data Buffer
 *    word 2: control
 *    word 3: timestamp word 1
 *    word 4: timestamp word 2
 *
 * 4. dma address width 64 bits with hardware timestamping:
 *    word 1: 32 bit address of Data Buffer
 *    word 2: control
 *    word 3: upper 32 bit address of Data Buffer
 *    word 4: unused
 *    word 5: timestamp word 1
 *    word 6: timestamp word 2
 */
static unsigned int macb_dma_desc_get_size(struct macb *bp)
{
#ifdef MACB_EXT_DESC
	unsigned int desc_size;

	switch (bp->hw_dma_cap) {
	case HW_DMA_CAP_64B:
		desc_size = sizeof(struct macb_dma_desc)
			+ sizeof(struct macb_dma_desc_64);
		break;
	case HW_DMA_CAP_PTP:
		desc_size = sizeof(struct macb_dma_desc)
			+ sizeof(struct macb_dma_desc_ptp);
		break;
	case HW_DMA_CAP_64B_PTP:
		desc_size = sizeof(struct macb_dma_desc)
			+ sizeof(struct macb_dma_desc_64)
			+ sizeof(struct macb_dma_desc_ptp);
		break;
	default:
		desc_size = sizeof(struct macb_dma_desc);
	}
	return desc_size;
#endif
	return sizeof(struct macb_dma_desc);
}
		if (status) {
			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, MACB_BIT(RCOMP));
			napi_reschedule(napi);
		} else {
			queue_writel(queue, IER, MACB_RX_INT_FLAGS);
		}
{
	struct macb *bp = (struct macb *)data;
	struct net_device *dev = bp->dev;
	struct macb_queue *queue = bp->queues;
	unsigned int q;
	u32 ctrl;

	for (q = 0, queue = bp->queues; q < bp->num_queues; ++q, ++queue) {
		queue_writel(queue, IDR, MACB_RX_INT_FLAGS |
					 MACB_TX_INT_FLAGS |
					 MACB_BIT(HRESP));
	}
	for (q = 0, queue = bp->queues; q < bp->num_queues; ++q, ++queue) {
		queue_writel(queue, RBQP, lower_32_bits(queue->rx_ring_dma));
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (bp->hw_dma_cap & HW_DMA_CAP_64B)
			queue_writel(queue, RBQPH,
				     upper_32_bits(queue->rx_ring_dma));
#endif
		queue_writel(queue, TBQP, lower_32_bits(queue->tx_ring_dma));
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (bp->hw_dma_cap & HW_DMA_CAP_64B)
			queue_writel(queue, TBQPH,
				     upper_32_bits(queue->tx_ring_dma));
#endif

		/* Enable interrupts */
		queue_writel(queue, IER,
			     MACB_RX_INT_FLAGS |
			     MACB_TX_INT_FLAGS |
			     MACB_BIT(HRESP));
	}

	ctrl |= MACB_BIT(RE) | MACB_BIT(TE);
	macb_writel(bp, NCR, ctrl);

	netif_carrier_on(dev);
	netif_tx_start_all_queues(dev);
}

static void macb_tx_restart(struct macb_queue *queue)
{
	unsigned int head = queue->tx_head;
	unsigned int tail = queue->tx_tail;
	struct macb *bp = queue->bp;

	if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
		queue_writel(queue, ISR, MACB_BIT(TXUBR));

	if (head == tail)
		return;

	macb_writel(bp, NCR, macb_readl(bp, NCR) | MACB_BIT(TSTART));
}

static irqreturn_t macb_interrupt(int irq, void *dev_id)
{
	struct macb_queue *queue = dev_id;
	struct macb *bp = queue->bp;
	struct net_device *dev = bp->dev;
	u32 status, ctrl;

	status = queue_readl(queue, ISR);

	if (unlikely(!status))
		return IRQ_NONE;

	spin_lock(&bp->lock);

	while (status) {
		/* close possible race with dev_close */
		if (unlikely(!netif_running(dev))) {
			queue_writel(queue, IDR, -1);
			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, -1);
			break;
		}
		if (unlikely(!netif_running(dev))) {
			queue_writel(queue, IDR, -1);
			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, -1);
			break;
		}

		netdev_vdbg(bp->dev, "queue = %u, isr = 0x%08lx\n",
			    (unsigned int)(queue - bp->queues),
			    (unsigned long)status);

		if (status & MACB_RX_INT_FLAGS) {
			/* There's no point taking any more interrupts
			 * until we have processed the buffers. The
			 * scheduling call may fail if the poll routine
			 * is already scheduled, so disable interrupts
			 * now.
			 */
			queue_writel(queue, IDR, MACB_RX_INT_FLAGS);
			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, MACB_BIT(RCOMP));

			if (napi_schedule_prep(&queue->napi)) {
				netdev_vdbg(bp->dev, "scheduling RX softirq\n");
				__napi_schedule(&queue->napi);
			}
		}
		if (unlikely(status & (MACB_TX_ERR_FLAGS))) {
			queue_writel(queue, IDR, MACB_TX_INT_FLAGS);
			schedule_work(&queue->tx_error_task);

			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, MACB_TX_ERR_FLAGS);

			break;
		}

		if (status & MACB_BIT(TCOMP))
			macb_tx_interrupt(queue);

		if (status & MACB_BIT(TXUBR))
			macb_tx_restart(queue);

		/* Link change detection isn't possible with RMII, so we'll
		 * add that if/when we get our hands on a full-blown MII PHY.
		 */

		/* There is a hardware issue under heavy load where DMA can
		 * stop, this causes endless "used buffer descriptor read"
		 * interrupts but it can be cleared by re-enabling RX. See
		 * the at91 manual, section 41.3.1 or the Zynq manual
		 * section 16.7.4 for details.
		 */
		if (status & MACB_BIT(RXUBR)) {
			ctrl = macb_readl(bp, NCR);
			macb_writel(bp, NCR, ctrl & ~MACB_BIT(RE));
			wmb();
			macb_writel(bp, NCR, ctrl | MACB_BIT(RE));

			if (bp->caps & MACB_CAPS_ISR_CLEAR_ON_WRITE)
				queue_writel(queue, ISR, MACB_BIT(RXUBR));
		}
	for (q = 0, queue = bp->queues; q < bp->num_queues; ++q, ++queue) {
		queue_writel(queue, RBQP, lower_32_bits(queue->rx_ring_dma));
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (bp->hw_dma_cap & HW_DMA_CAP_64B)
			queue_writel(queue, RBQPH, upper_32_bits(queue->rx_ring_dma));
#endif
		queue_writel(queue, TBQP, lower_32_bits(queue->tx_ring_dma));
#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
		if (bp->hw_dma_cap & HW_DMA_CAP_64B)
			queue_writel(queue, TBQPH, upper_32_bits(queue->tx_ring_dma));
#endif

		/* Enable interrupts */
		queue_writel(queue, IER,
			     MACB_RX_INT_FLAGS |
			     MACB_TX_INT_FLAGS |
			     MACB_BIT(HRESP));
	}

	/* Enable TX and RX */
	macb_writel(bp, NCR, macb_readl(bp, NCR) | MACB_BIT(RE) | MACB_BIT(TE));
}

/* The hash address register is 64 bits long and takes up two
 * locations in the memory map.  The least significant bits are stored
 * in EMAC_HSL and the most significant bits in EMAC_HSH.
 *
 * The unicast hash enable and the multicast hash enable bits in the
 * network configuration register enable the reception of hash matched
 * frames. The destination address is reduced to a 6 bit index into
 * the 64 bit hash register using the following hash function.  The
 * hash function is an exclusive or of every sixth bit of the
 * destination address.
 *
 * hi[5] = da[5] ^ da[11] ^ da[17] ^ da[23] ^ da[29] ^ da[35] ^ da[41] ^ da[47]
 * hi[4] = da[4] ^ da[10] ^ da[16] ^ da[22] ^ da[28] ^ da[34] ^ da[40] ^ da[46]
 * hi[3] = da[3] ^ da[09] ^ da[15] ^ da[21] ^ da[27] ^ da[33] ^ da[39] ^ da[45]
 * hi[2] = da[2] ^ da[08] ^ da[14] ^ da[20] ^ da[26] ^ da[32] ^ da[38] ^ da[44]
 * hi[1] = da[1] ^ da[07] ^ da[13] ^ da[19] ^ da[25] ^ da[31] ^ da[37] ^ da[43]
 * hi[0] = da[0] ^ da[06] ^ da[12] ^ da[18] ^ da[24] ^ da[30] ^ da[36] ^ da[42]
 *
 * da[0] represents the least significant bit of the first byte
 * received, that is, the multicast/unicast indicator, and da[47]
 * represents the most significant bit of the last byte received.  If
 * the hash index, hi[n], points to a bit that is set in the hash
 * register then the frame will be matched according to whether the
 * frame is multicast or unicast.  A multicast match will be signalled
 * if the multicast hash enable bit is set, da[0] is 1 and the hash
 * index points to a bit set in the hash register.  A unicast match
 * will be signalled if the unicast hash enable bit is set, da[0] is 0
 * and the hash index points to a bit set in the hash register.  To
 * receive all multicast frames, the hash register should be set with
 * all ones and the multicast hash enable bit should be set in the
 * network configuration register.
 */

static inline int hash_bit_value(int bitnr, __u8 *addr)
{
	if (addr[bitnr / 8] & (1 << (bitnr % 8)))
		return 1;
	return 0;
}

/* Return the hash index value for the specified address. */
static int hash_get_index(__u8 *addr)
{
	int i, j, bitval;
	int hash_index = 0;

	for (j = 0; j < 6; j++) {
		for (i = 0, bitval = 0; i < 8; i++)
			bitval ^= hash_bit_value(i * 6 + j, addr);

		hash_index |= (bitval << j);
	}

	return hash_index;
}

/* Add multicast addresses to the internal multicast-hash table. */
static void macb_sethashtable(struct net_device *dev)
{
	struct netdev_hw_addr *ha;
	unsigned long mc_filter[2];
	unsigned int bitnr;
	struct macb *bp = netdev_priv(dev);

	mc_filter[0] = 0;
	mc_filter[1] = 0;

	netdev_for_each_mc_addr(ha, dev) {
		bitnr = hash_get_index(ha->addr);
		mc_filter[bitnr >> 5] |= 1 << (bitnr & 31);
	}

	macb_or_gem_writel(bp, HRB, mc_filter[0]);
	macb_or_gem_writel(bp, HRT, mc_filter[1]);
}

/* Enable/Disable promiscuous and multicast modes. */
static void macb_set_rx_mode(struct net_device *dev)
{
	unsigned long cfg;
	struct macb *bp = netdev_priv(dev);

	cfg = macb_readl(bp, NCFGR);

	if (dev->flags & IFF_PROMISC) {
		/* Enable promiscuous mode */
		cfg |= MACB_BIT(CAF);

		/* Disable RX checksum offload */
		if (macb_is_gem(bp))
			cfg &= ~GEM_BIT(RXCOEN);
	} else {
		/* Disable promiscuous mode */
		cfg &= ~MACB_BIT(CAF);

		/* Enable RX checksum offload only if requested */
		if (macb_is_gem(bp) && dev->features & NETIF_F_RXCSUM)
			cfg |= GEM_BIT(RXCOEN);
	}
static const struct macb_config sama5d4_config = {
	.caps = MACB_CAPS_USRIO_DEFAULT_IS_MII_GMII,
	.dma_burst_length = 4,
	.clk_init = macb_clk_init,
	.init = macb_init,
};

static const struct macb_config emac_config = {
	.clk_init = at91ether_clk_init,
	.init = at91ether_init,
};

static const struct macb_config np4_config = {
	.caps = MACB_CAPS_USRIO_DISABLED,
	.clk_init = macb_clk_init,
	.init = macb_init,
};

static const struct macb_config zynqmp_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE |
			MACB_CAPS_JUMBO |
			MACB_CAPS_GEM_HAS_PTP | MACB_CAPS_BD_RD_PREFETCH,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
	.jumbo_max_len = 10240,
};

static const struct macb_config zynq_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE | MACB_CAPS_NO_GIGABIT_HALF,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
};

static const struct of_device_id macb_dt_ids[] = {
	{ .compatible = "cdns,at32ap7000-macb" },
	{ .compatible = "cdns,at91sam9260-macb", .data = &at91sam9260_config },
	{ .compatible = "cdns,macb" },
	{ .compatible = "cdns,np4-macb", .data = &np4_config },
	{ .compatible = "cdns,pc302-gem", .data = &pc302gem_config },
	{ .compatible = "cdns,gem", .data = &pc302gem_config },
	{ .compatible = "atmel,sama5d2-gem", .data = &sama5d2_config },
	{ .compatible = "atmel,sama5d3-gem", .data = &sama5d3_config },
	{ .compatible = "atmel,sama5d3-macb", .data = &sama5d3macb_config },
	{ .compatible = "atmel,sama5d4-gem", .data = &sama5d4_config },
	{ .compatible = "cdns,at91rm9200-emac", .data = &emac_config },
	{ .compatible = "cdns,emac", .data = &emac_config },
	{ .compatible = "cdns,zynqmp-gem", .data = &zynqmp_config},
	{ .compatible = "cdns,zynq-gem", .data = &zynq_config },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, macb_dt_ids);
#endif /* CONFIG_OF */

static const struct macb_config default_gem_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE |
			MACB_CAPS_JUMBO |
			MACB_CAPS_GEM_HAS_PTP,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
	.jumbo_max_len = 10240,
};

static int macb_probe(struct platform_device *pdev)
{
	const struct macb_config *macb_config = &default_gem_config;
	int (*clk_init)(struct platform_device *, struct clk **,
			struct clk **, struct clk **,  struct clk **)
					      = macb_config->clk_init;
	int (*init)(struct platform_device *) = macb_config->init;
	struct device_node *np = pdev->dev.of_node;
	struct clk *pclk, *hclk = NULL, *tx_clk = NULL, *rx_clk = NULL;
	unsigned int queue_mask, num_queues;
	struct macb_platform_data *pdata;
	bool native_io;
	struct phy_device *phydev;
	struct net_device *dev;
	struct resource *regs;
	void __iomem *mem;
	const char *mac;
	struct macb *bp;
	int err, val;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mem = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(mem))
		return PTR_ERR(mem);

	if (np) {
		const struct of_device_id *match;

		match = of_match_node(macb_dt_ids, np);
		if (match && match->data) {
			macb_config = match->data;
			clk_init = macb_config->clk_init;
			init = macb_config->init;
		}
	}

	err = clk_init(pdev, &pclk, &hclk, &tx_clk, &rx_clk);
	if (err)
		return err;

	native_io = hw_is_native_io(mem);

	macb_probe_queues(mem, native_io, &queue_mask, &num_queues);
	dev = alloc_etherdev_mq(sizeof(*bp), num_queues);
	if (!dev) {
		err = -ENOMEM;
		goto err_disable_clocks;
	}

	dev->base_addr = regs->start;

	SET_NETDEV_DEV(dev, &pdev->dev);

	bp = netdev_priv(dev);
	bp->pdev = pdev;
	bp->dev = dev;
	bp->regs = mem;
	bp->native_io = native_io;
	if (native_io) {
		bp->macb_reg_readl = hw_readl_native;
		bp->macb_reg_writel = hw_writel_native;
	} else {
		bp->macb_reg_readl = hw_readl;
		bp->macb_reg_writel = hw_writel;
	}
	bp->num_queues = num_queues;
	bp->queue_mask = queue_mask;
	if (macb_config)
		bp->dma_burst_length = macb_config->dma_burst_length;
	bp->pclk = pclk;
	bp->hclk = hclk;
	bp->tx_clk = tx_clk;
	bp->rx_clk = rx_clk;
	if (macb_config)
		bp->jumbo_max_len = macb_config->jumbo_max_len;

	bp->wol = 0;
	if (of_get_property(np, "magic-packet", NULL))
		bp->wol |= MACB_WOL_HAS_MAGIC_PACKET;
	device_init_wakeup(&pdev->dev, bp->wol & MACB_WOL_HAS_MAGIC_PACKET);

	spin_lock_init(&bp->lock);

	/* setup capabilities */
	macb_configure_caps(bp, macb_config);

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	if (GEM_BFEXT(DAW64, gem_readl(bp, DCFG6))) {
		dma_set_mask(&pdev->dev, DMA_BIT_MASK(44));
		bp->hw_dma_cap |= HW_DMA_CAP_64B;
	}
#endif
	platform_set_drvdata(pdev, dev);

	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0) {
		err = dev->irq;
		goto err_out_free_netdev;
	}

	/* MTU range: 68 - 1500 or 10240 */
	dev->min_mtu = GEM_MTU_MIN_SIZE;
	if (bp->caps & MACB_CAPS_JUMBO)
		dev->max_mtu = gem_readl(bp, JML) - ETH_HLEN - ETH_FCS_LEN;
	else
		dev->max_mtu = ETH_DATA_LEN;

	if (bp->caps & MACB_CAPS_BD_RD_PREFETCH) {
		val = GEM_BFEXT(RXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->rx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);

		val = GEM_BFEXT(TXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->tx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);
	}

	mac = of_get_mac_address(np);
	if (mac) {
		ether_addr_copy(bp->dev->dev_addr, mac);
	} else {
		err = nvmem_get_mac_address(&pdev->dev, bp->dev->dev_addr);
		if (err) {
			if (err == -EPROBE_DEFER)
				goto err_out_free_netdev;
			macb_get_hwaddr(bp);
		}
	}

	err = of_get_phy_mode(np);
	if (err < 0) {
		pdata = dev_get_platdata(&pdev->dev);
		if (pdata && pdata->is_rmii)
			bp->phy_interface = PHY_INTERFACE_MODE_RMII;
		else
			bp->phy_interface = PHY_INTERFACE_MODE_MII;
	} else {
		bp->phy_interface = err;
	}

	/* IP specific init */
	err = init(pdev);
	if (err)
		goto err_out_free_netdev;

	err = macb_mii_init(bp);
	if (err)
		goto err_out_free_netdev;

	phydev = dev->phydev;

	netif_carrier_off(dev);

	err = register_netdev(dev);
	if (err) {
		dev_err(&pdev->dev, "Cannot register net device, aborting.\n");
		goto err_out_unregister_mdio;
	}

	tasklet_init(&bp->hresp_err_tasklet, macb_hresp_error_task,
		     (unsigned long)bp);

	phy_attached_info(phydev);

	netdev_info(dev, "Cadence %s rev 0x%08x at 0x%08lx irq %d (%pM)\n",
		    macb_is_gem(bp) ? "GEM" : "MACB", macb_readl(bp, MID),
		    dev->base_addr, dev->irq, dev->dev_addr);

	return 0;

err_out_unregister_mdio:
	phy_disconnect(dev->phydev);
	mdiobus_unregister(bp->mii_bus);
	of_node_put(bp->phy_node);
	if (np && of_phy_is_fixed_link(np))
		of_phy_deregister_fixed_link(np);
	mdiobus_free(bp->mii_bus);

err_out_free_netdev:
	free_netdev(dev);

err_disable_clocks:
	clk_disable_unprepare(tx_clk);
	clk_disable_unprepare(hclk);
	clk_disable_unprepare(pclk);
	clk_disable_unprepare(rx_clk);

	return err;
}

static int macb_remove(struct platform_device *pdev)
{
	struct net_device *dev;
	struct macb *bp;
	struct device_node *np = pdev->dev.of_node;

	dev = platform_get_drvdata(pdev);

	if (dev) {
		bp = netdev_priv(dev);
		if (dev->phydev)
			phy_disconnect(dev->phydev);
		mdiobus_unregister(bp->mii_bus);
		if (np && of_phy_is_fixed_link(np))
			of_phy_deregister_fixed_link(np);
		dev->phydev = NULL;
		mdiobus_free(bp->mii_bus);

		unregister_netdev(dev);
		clk_disable_unprepare(bp->tx_clk);
		clk_disable_unprepare(bp->hclk);
		clk_disable_unprepare(bp->pclk);
		clk_disable_unprepare(bp->rx_clk);
		of_node_put(bp->phy_node);
		free_netdev(dev);
	}

	return 0;
}

static int __maybe_unused macb_suspend(struct device *dev)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct macb *bp = netdev_priv(netdev);

	netif_carrier_off(netdev);
	netif_device_detach(netdev);

	if (bp->wol & MACB_WOL_ENABLED) {
		macb_writel(bp, IER, MACB_BIT(WOL));
		macb_writel(bp, WOL, MACB_BIT(MAG));
		enable_irq_wake(bp->queues[0].irq);
	} else {
		clk_disable_unprepare(bp->tx_clk);
		clk_disable_unprepare(bp->hclk);
		clk_disable_unprepare(bp->pclk);
		clk_disable_unprepare(bp->rx_clk);
	}

	return 0;
}

static int __maybe_unused macb_resume(struct device *dev)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct macb *bp = netdev_priv(netdev);

	if (bp->wol & MACB_WOL_ENABLED) {
		macb_writel(bp, IDR, MACB_BIT(WOL));
		macb_writel(bp, WOL, 0);
		disable_irq_wake(bp->queues[0].irq);
	} else {
		clk_prepare_enable(bp->pclk);
		clk_prepare_enable(bp->hclk);
		clk_prepare_enable(bp->tx_clk);
		clk_prepare_enable(bp->rx_clk);
	}

	netif_device_attach(netdev);

	return 0;
}

static SIMPLE_DEV_PM_OPS(macb_pm_ops, macb_suspend, macb_resume);

static struct platform_driver macb_driver = {
	.probe		= macb_probe,
	.remove		= macb_remove,
	.driver		= {
		.name		= "macb",
		.of_match_table	= of_match_ptr(macb_dt_ids),
		.pm	= &macb_pm_ops,
	},
};

module_platform_driver(macb_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cadence MACB/GEM Ethernet driver");
MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_ALIAS("platform:macb");
static const struct macb_config zynqmp_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE |
			MACB_CAPS_JUMBO |
			MACB_CAPS_GEM_HAS_PTP | MACB_CAPS_BD_RD_PREFETCH,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
	.jumbo_max_len = 10240,
};

static const struct macb_config zynq_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE | MACB_CAPS_NO_GIGABIT_HALF,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
};

static const struct of_device_id macb_dt_ids[] = {
	{ .compatible = "cdns,at32ap7000-macb" },
	{ .compatible = "cdns,at91sam9260-macb", .data = &at91sam9260_config },
	{ .compatible = "cdns,macb" },
	{ .compatible = "cdns,np4-macb", .data = &np4_config },
	{ .compatible = "cdns,pc302-gem", .data = &pc302gem_config },
	{ .compatible = "cdns,gem", .data = &pc302gem_config },
	{ .compatible = "atmel,sama5d2-gem", .data = &sama5d2_config },
	{ .compatible = "atmel,sama5d3-gem", .data = &sama5d3_config },
	{ .compatible = "atmel,sama5d3-macb", .data = &sama5d3macb_config },
	{ .compatible = "atmel,sama5d4-gem", .data = &sama5d4_config },
	{ .compatible = "cdns,at91rm9200-emac", .data = &emac_config },
	{ .compatible = "cdns,emac", .data = &emac_config },
	{ .compatible = "cdns,zynqmp-gem", .data = &zynqmp_config},
	{ .compatible = "cdns,zynq-gem", .data = &zynq_config },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, macb_dt_ids);
#endif /* CONFIG_OF */

static const struct macb_config default_gem_config = {
	.caps = MACB_CAPS_GIGABIT_MODE_AVAILABLE |
			MACB_CAPS_JUMBO |
			MACB_CAPS_GEM_HAS_PTP,
	.dma_burst_length = 16,
	.clk_init = macb_clk_init,
	.init = macb_init,
	.jumbo_max_len = 10240,
};

static int macb_probe(struct platform_device *pdev)
{
	const struct macb_config *macb_config = &default_gem_config;
	int (*clk_init)(struct platform_device *, struct clk **,
			struct clk **, struct clk **,  struct clk **)
					      = macb_config->clk_init;
	int (*init)(struct platform_device *) = macb_config->init;
	struct device_node *np = pdev->dev.of_node;
	struct clk *pclk, *hclk = NULL, *tx_clk = NULL, *rx_clk = NULL;
	unsigned int queue_mask, num_queues;
	struct macb_platform_data *pdata;
	bool native_io;
	struct phy_device *phydev;
	struct net_device *dev;
	struct resource *regs;
	void __iomem *mem;
	const char *mac;
	struct macb *bp;
	int err, val;

	regs = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	mem = devm_ioremap_resource(&pdev->dev, regs);
	if (IS_ERR(mem))
		return PTR_ERR(mem);

	if (np) {
		const struct of_device_id *match;

		match = of_match_node(macb_dt_ids, np);
		if (match && match->data) {
			macb_config = match->data;
			clk_init = macb_config->clk_init;
			init = macb_config->init;
		}
	}

	err = clk_init(pdev, &pclk, &hclk, &tx_clk, &rx_clk);
	if (err)
		return err;

	native_io = hw_is_native_io(mem);

	macb_probe_queues(mem, native_io, &queue_mask, &num_queues);
	dev = alloc_etherdev_mq(sizeof(*bp), num_queues);
	if (!dev) {
		err = -ENOMEM;
		goto err_disable_clocks;
	}

	dev->base_addr = regs->start;

	SET_NETDEV_DEV(dev, &pdev->dev);

	bp = netdev_priv(dev);
	bp->pdev = pdev;
	bp->dev = dev;
	bp->regs = mem;
	bp->native_io = native_io;
	if (native_io) {
		bp->macb_reg_readl = hw_readl_native;
		bp->macb_reg_writel = hw_writel_native;
	} else {
		bp->macb_reg_readl = hw_readl;
		bp->macb_reg_writel = hw_writel;
	}
	bp->num_queues = num_queues;
	bp->queue_mask = queue_mask;
	if (macb_config)
		bp->dma_burst_length = macb_config->dma_burst_length;
	bp->pclk = pclk;
	bp->hclk = hclk;
	bp->tx_clk = tx_clk;
	bp->rx_clk = rx_clk;
	if (macb_config)
		bp->jumbo_max_len = macb_config->jumbo_max_len;

	bp->wol = 0;
	if (of_get_property(np, "magic-packet", NULL))
		bp->wol |= MACB_WOL_HAS_MAGIC_PACKET;
	device_init_wakeup(&pdev->dev, bp->wol & MACB_WOL_HAS_MAGIC_PACKET);

	spin_lock_init(&bp->lock);

	/* setup capabilities */
	macb_configure_caps(bp, macb_config);

#ifdef CONFIG_ARCH_DMA_ADDR_T_64BIT
	if (GEM_BFEXT(DAW64, gem_readl(bp, DCFG6))) {
		dma_set_mask(&pdev->dev, DMA_BIT_MASK(44));
		bp->hw_dma_cap |= HW_DMA_CAP_64B;
	}
#endif
	platform_set_drvdata(pdev, dev);

	dev->irq = platform_get_irq(pdev, 0);
	if (dev->irq < 0) {
		err = dev->irq;
		goto err_out_free_netdev;
	}

	/* MTU range: 68 - 1500 or 10240 */
	dev->min_mtu = GEM_MTU_MIN_SIZE;
	if (bp->caps & MACB_CAPS_JUMBO)
		dev->max_mtu = gem_readl(bp, JML) - ETH_HLEN - ETH_FCS_LEN;
	else
		dev->max_mtu = ETH_DATA_LEN;

	if (bp->caps & MACB_CAPS_BD_RD_PREFETCH) {
		val = GEM_BFEXT(RXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->rx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);

		val = GEM_BFEXT(TXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->tx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);
	}

	mac = of_get_mac_address(np);
	if (mac) {
		ether_addr_copy(bp->dev->dev_addr, mac);
	} else {
		err = nvmem_get_mac_address(&pdev->dev, bp->dev->dev_addr);
		if (err) {
			if (err == -EPROBE_DEFER)
				goto err_out_free_netdev;
			macb_get_hwaddr(bp);
		}
	}

	err = of_get_phy_mode(np);
	if (err < 0) {
		pdata = dev_get_platdata(&pdev->dev);
		if (pdata && pdata->is_rmii)
			bp->phy_interface = PHY_INTERFACE_MODE_RMII;
		else
			bp->phy_interface = PHY_INTERFACE_MODE_MII;
	} else {
		bp->phy_interface = err;
	}

	/* IP specific init */
	err = init(pdev);
	if (err)
		goto err_out_free_netdev;

	err = macb_mii_init(bp);
	if (err)
		goto err_out_free_netdev;

	phydev = dev->phydev;

	netif_carrier_off(dev);

	err = register_netdev(dev);
	if (err) {
		dev_err(&pdev->dev, "Cannot register net device, aborting.\n");
		goto err_out_unregister_mdio;
	}

	tasklet_init(&bp->hresp_err_tasklet, macb_hresp_error_task,
		     (unsigned long)bp);

	phy_attached_info(phydev);

	netdev_info(dev, "Cadence %s rev 0x%08x at 0x%08lx irq %d (%pM)\n",
		    macb_is_gem(bp) ? "GEM" : "MACB", macb_readl(bp, MID),
		    dev->base_addr, dev->irq, dev->dev_addr);

	return 0;

err_out_unregister_mdio:
	phy_disconnect(dev->phydev);
	mdiobus_unregister(bp->mii_bus);
	of_node_put(bp->phy_node);
	if (np && of_phy_is_fixed_link(np))
		of_phy_deregister_fixed_link(np);
	mdiobus_free(bp->mii_bus);

err_out_free_netdev:
	free_netdev(dev);

err_disable_clocks:
	clk_disable_unprepare(tx_clk);
	clk_disable_unprepare(hclk);
	clk_disable_unprepare(pclk);
	clk_disable_unprepare(rx_clk);

	return err;
}

static int macb_remove(struct platform_device *pdev)
{
	struct net_device *dev;
	struct macb *bp;
	struct device_node *np = pdev->dev.of_node;

	dev = platform_get_drvdata(pdev);

	if (dev) {
		bp = netdev_priv(dev);
		if (dev->phydev)
			phy_disconnect(dev->phydev);
		mdiobus_unregister(bp->mii_bus);
		if (np && of_phy_is_fixed_link(np))
			of_phy_deregister_fixed_link(np);
		dev->phydev = NULL;
		mdiobus_free(bp->mii_bus);

		unregister_netdev(dev);
		clk_disable_unprepare(bp->tx_clk);
		clk_disable_unprepare(bp->hclk);
		clk_disable_unprepare(bp->pclk);
		clk_disable_unprepare(bp->rx_clk);
		of_node_put(bp->phy_node);
		free_netdev(dev);
	}

	return 0;
}

static int __maybe_unused macb_suspend(struct device *dev)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct macb *bp = netdev_priv(netdev);

	netif_carrier_off(netdev);
	netif_device_detach(netdev);

	if (bp->wol & MACB_WOL_ENABLED) {
		macb_writel(bp, IER, MACB_BIT(WOL));
		macb_writel(bp, WOL, MACB_BIT(MAG));
		enable_irq_wake(bp->queues[0].irq);
	} else {
		clk_disable_unprepare(bp->tx_clk);
		clk_disable_unprepare(bp->hclk);
		clk_disable_unprepare(bp->pclk);
		clk_disable_unprepare(bp->rx_clk);
	}

	return 0;
}

static int __maybe_unused macb_resume(struct device *dev)
{
	struct net_device *netdev = dev_get_drvdata(dev);
	struct macb *bp = netdev_priv(netdev);

	if (bp->wol & MACB_WOL_ENABLED) {
		macb_writel(bp, IDR, MACB_BIT(WOL));
		macb_writel(bp, WOL, 0);
		disable_irq_wake(bp->queues[0].irq);
	} else {
		clk_prepare_enable(bp->pclk);
		clk_prepare_enable(bp->hclk);
		clk_prepare_enable(bp->tx_clk);
		clk_prepare_enable(bp->rx_clk);
	}

	netif_device_attach(netdev);

	return 0;
}

static SIMPLE_DEV_PM_OPS(macb_pm_ops, macb_suspend, macb_resume);

static struct platform_driver macb_driver = {
	.probe		= macb_probe,
	.remove		= macb_remove,
	.driver		= {
		.name		= "macb",
		.of_match_table	= of_match_ptr(macb_dt_ids),
		.pm	= &macb_pm_ops,
	},
};

module_platform_driver(macb_driver);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Cadence MACB/GEM Ethernet driver");
MODULE_AUTHOR("Haavard Skinnemoen (Atmel)");
MODULE_ALIAS("platform:macb");
	if (bp->caps & MACB_CAPS_BD_RD_PREFETCH) {
		val = GEM_BFEXT(RXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->rx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);

		val = GEM_BFEXT(TXBD_RDBUFF, gem_readl(bp, DCFG10));
		if (val)
			bp->tx_bd_rd_prefetch = (2 << (val - 1)) *
						macb_dma_desc_get_size(bp);
	}

	mac = of_get_mac_address(np);
	if (mac) {
		ether_addr_copy(bp->dev->dev_addr, mac);
	} else {
		err = nvmem_get_mac_address(&pdev->dev, bp->dev->dev_addr);
		if (err) {
			if (err == -EPROBE_DEFER)
				goto err_out_free_netdev;
			macb_get_hwaddr(bp);
		}
	}