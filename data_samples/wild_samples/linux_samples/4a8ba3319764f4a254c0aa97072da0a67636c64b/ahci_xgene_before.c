{
	struct ahci_host_priv *hpriv = ap->host->private_data;

	ahci_stop_engine(ap);
	ahci_start_fis_rx(ap);
	hpriv->start_engine(ap);

	return 0;
}

/**
 * xgene_ahci_qc_issue - Issue commands to the device
 * @qc: Command to issue
 *
 * Due to Hardware errata for IDENTIFY DEVICE command, the controller cannot
 * clear the BSY bit after receiving the PIO setup FIS. This results in the dma
 * state machine goes into the CMFatalErrorUpdate state and locks up. By
 * restarting the dma engine, it removes the controller out of lock up state.
 */
static unsigned int xgene_ahci_qc_issue(struct ata_queued_cmd *qc)
{
	struct ata_port *ap = qc->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct xgene_ahci_context *ctx = hpriv->plat_data;
	int rc = 0;

	if (unlikely(ctx->last_cmd[ap->port_no] == ATA_CMD_ID_ATA))
		xgene_ahci_restart_engine(ap);

	rc = ahci_qc_issue(qc);

	/* Save the last command issued */
	ctx->last_cmd[ap->port_no] = qc->tf.command;

	return rc;
}
{
	struct ata_port *ap = qc->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct xgene_ahci_context *ctx = hpriv->plat_data;
	int rc = 0;

	if (unlikely(ctx->last_cmd[ap->port_no] == ATA_CMD_ID_ATA))
		xgene_ahci_restart_engine(ap);

	rc = ahci_qc_issue(qc);

	/* Save the last command issued */
	ctx->last_cmd[ap->port_no] = qc->tf.command;

	return rc;
}

static bool xgene_ahci_is_memram_inited(struct xgene_ahci_context *ctx)
{
	void __iomem *diagcsr = ctx->csr_diag;

	return (readl(diagcsr + CFG_MEM_RAM_SHUTDOWN) == 0 &&
	        readl(diagcsr + BLOCK_MEM_RDY) == 0xFFFFFFFF);
}

/**
 * xgene_ahci_read_id - Read ID data from the specified device
 * @dev: device
 * @tf: proposed taskfile
 * @id: data buffer
 *
 * This custom read ID function is required due to the fact that the HW
 * does not support DEVSLP.
 */
static unsigned int xgene_ahci_read_id(struct ata_device *dev,
				       struct ata_taskfile *tf, u16 *id)
{
	u32 err_mask;

	err_mask = ata_do_dev_read_id(dev, tf, id);
	if (err_mask)
		return err_mask;

	/*
	 * Mask reserved area. Word78 spec of Link Power Management
	 * bit15-8: reserved
	 * bit7: NCQ autosence
	 * bit6: Software settings preservation supported
	 * bit5: reserved
	 * bit4: In-order sata delivery supported
	 * bit3: DIPM requests supported
	 * bit2: DMA Setup FIS Auto-Activate optimization supported
	 * bit1: DMA Setup FIX non-Zero buffer offsets supported
	 * bit0: Reserved
	 *
	 * Clear reserved bit 8 (DEVSLP bit) as we don't support DEVSLP
	 */
	id[ATA_ID_FEATURE_SUPP] &= ~(1 << 8);

	return 0;
}

static void xgene_ahci_set_phy_cfg(struct xgene_ahci_context *ctx, int channel)
{
	void __iomem *mmio = ctx->hpriv->mmio;
	u32 val;

	dev_dbg(ctx->dev, "port configure mmio 0x%p channel %d\n",
		mmio, channel);
	val = readl(mmio + PORTCFG);
	val = PORTADDR_SET(val, channel == 0 ? 2 : 3);
	writel(val, mmio + PORTCFG);
	readl(mmio + PORTCFG);  /* Force a barrier */
	/* Disable fix rate */
	writel(0x0001fffe, mmio + PORTPHY1CFG);
	readl(mmio + PORTPHY1CFG); /* Force a barrier */
	writel(0x28183219, mmio + PORTPHY2CFG);
	readl(mmio + PORTPHY2CFG); /* Force a barrier */
	writel(0x13081008, mmio + PORTPHY3CFG);
	readl(mmio + PORTPHY3CFG); /* Force a barrier */
	writel(0x00480815, mmio + PORTPHY4CFG);
	readl(mmio + PORTPHY4CFG); /* Force a barrier */
	/* Set window negotiation */
	val = readl(mmio + PORTPHY5CFG);
	val = PORTPHY5CFG_RTCHG_SET(val, 0x300);
	writel(val, mmio + PORTPHY5CFG);
	readl(mmio + PORTPHY5CFG); /* Force a barrier */
	val = readl(mmio + PORTAXICFG);
	val = PORTAXICFG_EN_CONTEXT_SET(val, 0x1); /* Enable context mgmt */
	val = PORTAXICFG_OUTTRANS_SET(val, 0xe); /* Set outstanding */
	writel(val, mmio + PORTAXICFG);
	readl(mmio + PORTAXICFG); /* Force a barrier */
	/* Set the watermark threshold of the receive FIFO */
	val = readl(mmio + PORTRANSCFG);
	val = PORTRANSCFG_RXWM_SET(val, 0x30);
	writel(val, mmio + PORTRANSCFG);
}

/**
 * xgene_ahci_do_hardreset - Issue the actual COMRESET
 * @link: link to reset
 * @deadline: deadline jiffies for the operation
 * @online: Return value to indicate if device online
 *
 * Due to the limitation of the hardware PHY, a difference set of setting is
 * required for each supported disk speed - Gen3 (6.0Gbps), Gen2 (3.0Gbps),
 * and Gen1 (1.5Gbps). Otherwise during long IO stress test, the PHY will
 * report disparity error and etc. In addition, during COMRESET, there can
 * be error reported in the register PORT_SCR_ERR. For SERR_DISPARITY and
 * SERR_10B_8B_ERR, the PHY receiver line must be reseted. Also during long
 * reboot cycle regression, sometimes the PHY reports link down even if the
 * device is present because of speed negotiation failure. so need to retry
 * the COMRESET to get the link up. The following algorithm is followed to
 * proper configure the hardware PHY during COMRESET:
 *
 * Alg Part 1:
 * 1. Start the PHY at Gen3 speed (default setting)
 * 2. Issue the COMRESET
 * 3. If no link, go to Alg Part 3
 * 4. If link up, determine if the negotiated speed matches the PHY
 *    configured speed
 * 5. If they matched, go to Alg Part 2
 * 6. If they do not matched and first time, configure the PHY for the linked
 *    up disk speed and repeat step 2
 * 7. Go to Alg Part 2
 *
 * Alg Part 2:
 * 1. On link up, if there are any SERR_DISPARITY and SERR_10B_8B_ERR error
 *    reported in the register PORT_SCR_ERR, then reset the PHY receiver line
 * 2. Go to Alg Part 4
 *
 * Alg Part 3:
 * 1. Check the PORT_SCR_STAT to see whether device presence detected but PHY
 *    communication establishment failed and maximum link down attempts are
 *    less than Max attempts 3 then goto Alg Part 1.
 * 2. Go to Alg Part 4.
 *
 * Alg Part 4:
 * 1. Clear any pending from register PORT_SCR_ERR.
 *
 * NOTE: For the initial version, we will NOT support Gen1/Gen2. In addition
 *       and until the underlying PHY supports an method to reset the receiver
 *       line, on detection of SERR_DISPARITY or SERR_10B_8B_ERR errors,
 *       an warning message will be printed.
 */
static int xgene_ahci_do_hardreset(struct ata_link *link,
				   unsigned long deadline, bool *online)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct xgene_ahci_context *ctx = hpriv->plat_data;
	struct ahci_port_priv *pp = ap->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ata_taskfile tf;
	int link_down_retry = 0;
	int rc;
	u32 val, sstatus;

	do {
		/* clear D2H reception area to properly wait for D2H FIS */
		ata_tf_init(link->device, &tf);
		tf.command = ATA_BUSY;
		ata_tf_to_fis(&tf, 0, 0, d2h_fis);
		rc = sata_link_hardreset(link, timing, deadline, online,
				 ahci_check_ready);
		if (*online) {
			val = readl(port_mmio + PORT_SCR_ERR);
			if (val & (SERR_DISPARITY | SERR_10B_8B_ERR))
				dev_warn(ctx->dev, "link has error\n");
			break;
		}

		sata_scr_read(link, SCR_STATUS, &sstatus);
	} while (link_down_retry++ < MAX_LINK_DOWN_RETRY &&
		 (sstatus & 0xff) == 0x1);

	/* clear all errors if any pending */
	val = readl(port_mmio + PORT_SCR_ERR);
	writel(val, port_mmio + PORT_SCR_ERR);

	return rc;
}
{
	u32 err_mask;

	err_mask = ata_do_dev_read_id(dev, tf, id);
	if (err_mask)
		return err_mask;

	/*
	 * Mask reserved area. Word78 spec of Link Power Management
	 * bit15-8: reserved
	 * bit7: NCQ autosence
	 * bit6: Software settings preservation supported
	 * bit5: reserved
	 * bit4: In-order sata delivery supported
	 * bit3: DIPM requests supported
	 * bit2: DMA Setup FIS Auto-Activate optimization supported
	 * bit1: DMA Setup FIX non-Zero buffer offsets supported
	 * bit0: Reserved
	 *
	 * Clear reserved bit 8 (DEVSLP bit) as we don't support DEVSLP
	 */
	id[ATA_ID_FEATURE_SUPP] &= ~(1 << 8);

	return 0;
}

static void xgene_ahci_set_phy_cfg(struct xgene_ahci_context *ctx, int channel)
{
	void __iomem *mmio = ctx->hpriv->mmio;
	u32 val;

	dev_dbg(ctx->dev, "port configure mmio 0x%p channel %d\n",
		mmio, channel);
	val = readl(mmio + PORTCFG);
	val = PORTADDR_SET(val, channel == 0 ? 2 : 3);
	writel(val, mmio + PORTCFG);
	readl(mmio + PORTCFG);  /* Force a barrier */
	/* Disable fix rate */
	writel(0x0001fffe, mmio + PORTPHY1CFG);
	readl(mmio + PORTPHY1CFG); /* Force a barrier */
	writel(0x28183219, mmio + PORTPHY2CFG);
	readl(mmio + PORTPHY2CFG); /* Force a barrier */
	writel(0x13081008, mmio + PORTPHY3CFG);
	readl(mmio + PORTPHY3CFG); /* Force a barrier */
	writel(0x00480815, mmio + PORTPHY4CFG);
	readl(mmio + PORTPHY4CFG); /* Force a barrier */
	/* Set window negotiation */
	val = readl(mmio + PORTPHY5CFG);
	val = PORTPHY5CFG_RTCHG_SET(val, 0x300);
	writel(val, mmio + PORTPHY5CFG);
	readl(mmio + PORTPHY5CFG); /* Force a barrier */
	val = readl(mmio + PORTAXICFG);
	val = PORTAXICFG_EN_CONTEXT_SET(val, 0x1); /* Enable context mgmt */
	val = PORTAXICFG_OUTTRANS_SET(val, 0xe); /* Set outstanding */
	writel(val, mmio + PORTAXICFG);
	readl(mmio + PORTAXICFG); /* Force a barrier */
	/* Set the watermark threshold of the receive FIFO */
	val = readl(mmio + PORTRANSCFG);
	val = PORTRANSCFG_RXWM_SET(val, 0x30);
	writel(val, mmio + PORTRANSCFG);
}

/**
 * xgene_ahci_do_hardreset - Issue the actual COMRESET
 * @link: link to reset
 * @deadline: deadline jiffies for the operation
 * @online: Return value to indicate if device online
 *
 * Due to the limitation of the hardware PHY, a difference set of setting is
 * required for each supported disk speed - Gen3 (6.0Gbps), Gen2 (3.0Gbps),
 * and Gen1 (1.5Gbps). Otherwise during long IO stress test, the PHY will
 * report disparity error and etc. In addition, during COMRESET, there can
 * be error reported in the register PORT_SCR_ERR. For SERR_DISPARITY and
 * SERR_10B_8B_ERR, the PHY receiver line must be reseted. Also during long
 * reboot cycle regression, sometimes the PHY reports link down even if the
 * device is present because of speed negotiation failure. so need to retry
 * the COMRESET to get the link up. The following algorithm is followed to
 * proper configure the hardware PHY during COMRESET:
 *
 * Alg Part 1:
 * 1. Start the PHY at Gen3 speed (default setting)
 * 2. Issue the COMRESET
 * 3. If no link, go to Alg Part 3
 * 4. If link up, determine if the negotiated speed matches the PHY
 *    configured speed
 * 5. If they matched, go to Alg Part 2
 * 6. If they do not matched and first time, configure the PHY for the linked
 *    up disk speed and repeat step 2
 * 7. Go to Alg Part 2
 *
 * Alg Part 2:
 * 1. On link up, if there are any SERR_DISPARITY and SERR_10B_8B_ERR error
 *    reported in the register PORT_SCR_ERR, then reset the PHY receiver line
 * 2. Go to Alg Part 4
 *
 * Alg Part 3:
 * 1. Check the PORT_SCR_STAT to see whether device presence detected but PHY
 *    communication establishment failed and maximum link down attempts are
 *    less than Max attempts 3 then goto Alg Part 1.
 * 2. Go to Alg Part 4.
 *
 * Alg Part 4:
 * 1. Clear any pending from register PORT_SCR_ERR.
 *
 * NOTE: For the initial version, we will NOT support Gen1/Gen2. In addition
 *       and until the underlying PHY supports an method to reset the receiver
 *       line, on detection of SERR_DISPARITY or SERR_10B_8B_ERR errors,
 *       an warning message will be printed.
 */
static int xgene_ahci_do_hardreset(struct ata_link *link,
				   unsigned long deadline, bool *online)
{
	const unsigned long *timing = sata_ehc_deb_timing(&link->eh_context);
	struct ata_port *ap = link->ap;
	struct ahci_host_priv *hpriv = ap->host->private_data;
	struct xgene_ahci_context *ctx = hpriv->plat_data;
	struct ahci_port_priv *pp = ap->private_data;
	u8 *d2h_fis = pp->rx_fis + RX_FIS_D2H_REG;
	void __iomem *port_mmio = ahci_port_base(ap);
	struct ata_taskfile tf;
	int link_down_retry = 0;
	int rc;
	u32 val, sstatus;

	do {
		/* clear D2H reception area to properly wait for D2H FIS */
		ata_tf_init(link->device, &tf);
		tf.command = ATA_BUSY;
		ata_tf_to_fis(&tf, 0, 0, d2h_fis);
		rc = sata_link_hardreset(link, timing, deadline, online,
				 ahci_check_ready);
		if (*online) {
			val = readl(port_mmio + PORT_SCR_ERR);
			if (val & (SERR_DISPARITY | SERR_10B_8B_ERR))
				dev_warn(ctx->dev, "link has error\n");
			break;
		}

		sata_scr_read(link, SCR_STATUS, &sstatus);
	} while (link_down_retry++ < MAX_LINK_DOWN_RETRY &&
		 (sstatus & 0xff) == 0x1);

	/* clear all errors if any pending */
	val = readl(port_mmio + PORT_SCR_ERR);
	writel(val, port_mmio + PORT_SCR_ERR);

	return rc;
}