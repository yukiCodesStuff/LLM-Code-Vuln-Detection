{
	struct dsi_data *dsi = container_of(pll, struct dsi_data, pll);

	dsi_pll_uninit(dsi, true);
}

static int dsi_dump_dsi_clocks(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;
	struct dss_pll_clock_info *cinfo = &dsi->pll.cinfo;
	enum dss_clk_source dispc_clk_src, dsi_clk_src;
	int dsi_module = dsi->module_id;
	struct dss_pll *pll = &dsi->pll;

	dispc_clk_src = dss_get_dispc_clk_source(dsi->dss);
	dsi_clk_src = dss_get_dsi_clk_source(dsi->dss, dsi_module);

	if (dsi_runtime_get(dsi))
		return 0;

	seq_printf(s,	"- DSI%d PLL -\n", dsi_module + 1);

	seq_printf(s,	"dsi pll clkin\t%lu\n", clk_get_rate(pll->clkin));

	seq_printf(s,	"Fint\t\t%-16lun %u\n", cinfo->fint, cinfo->n);

	seq_printf(s,	"CLKIN4DDR\t%-16lum %u\n",
			cinfo->clkdco, cinfo->m);

	seq_printf(s,	"DSI_PLL_HSDIV_DISPC (%s)\t%-16lum_dispc %u\t(%s)\n",
			dss_get_clk_source_name(dsi_module == 0 ?
				DSS_CLK_SRC_PLL1_1 :
				DSS_CLK_SRC_PLL2_1),
			cinfo->clkout[HSDIV_DISPC],
			cinfo->mX[HSDIV_DISPC],
			dispc_clk_src == DSS_CLK_SRC_FCK ?
			"off" : "on");

	seq_printf(s,	"DSI_PLL_HSDIV_DSI (%s)\t%-16lum_dsi %u\t(%s)\n",
			dss_get_clk_source_name(dsi_module == 0 ?
				DSS_CLK_SRC_PLL1_2 :
				DSS_CLK_SRC_PLL2_2),
			cinfo->clkout[HSDIV_DSI],
			cinfo->mX[HSDIV_DSI],
			dsi_clk_src == DSS_CLK_SRC_FCK ?
			"off" : "on");

	seq_printf(s,	"- DSI%d -\n", dsi_module + 1);

	seq_printf(s,	"dsi fclk source = %s\n",
			dss_get_clk_source_name(dsi_clk_src));

	seq_printf(s,	"DSI_FCLK\t%lu\n", dsi_fclk_rate(dsi));

	seq_printf(s,	"DDR_CLK\t\t%lu\n",
			cinfo->clkdco / 4);

	seq_printf(s,	"TxByteClkHS\t%lu\n", dsi_get_txbyteclkhs(dsi));

	seq_printf(s,	"LP_CLK\t\t%lu\n", dsi->current_lp_cinfo.lp_clk);

	dsi_runtime_put(dsi);

	return 0;
}
{
	struct dsi_data *dsi = p;
	struct dss_pll_clock_info *cinfo = &dsi->pll.cinfo;
	enum dss_clk_source dispc_clk_src, dsi_clk_src;
	int dsi_module = dsi->module_id;
	struct dss_pll *pll = &dsi->pll;

	dispc_clk_src = dss_get_dispc_clk_source(dsi->dss);
	dsi_clk_src = dss_get_dsi_clk_source(dsi->dss, dsi_module);

	if (dsi_runtime_get(dsi))
		return 0;

	seq_printf(s,	"- DSI%d PLL -\n", dsi_module + 1);

	seq_printf(s,	"dsi pll clkin\t%lu\n", clk_get_rate(pll->clkin));

	seq_printf(s,	"Fint\t\t%-16lun %u\n", cinfo->fint, cinfo->n);

	seq_printf(s,	"CLKIN4DDR\t%-16lum %u\n",
			cinfo->clkdco, cinfo->m);

	seq_printf(s,	"DSI_PLL_HSDIV_DISPC (%s)\t%-16lum_dispc %u\t(%s)\n",
			dss_get_clk_source_name(dsi_module == 0 ?
				DSS_CLK_SRC_PLL1_1 :
				DSS_CLK_SRC_PLL2_1),
			cinfo->clkout[HSDIV_DISPC],
			cinfo->mX[HSDIV_DISPC],
			dispc_clk_src == DSS_CLK_SRC_FCK ?
			"off" : "on");

	seq_printf(s,	"DSI_PLL_HSDIV_DSI (%s)\t%-16lum_dsi %u\t(%s)\n",
			dss_get_clk_source_name(dsi_module == 0 ?
				DSS_CLK_SRC_PLL1_2 :
				DSS_CLK_SRC_PLL2_2),
			cinfo->clkout[HSDIV_DSI],
			cinfo->mX[HSDIV_DSI],
			dsi_clk_src == DSS_CLK_SRC_FCK ?
			"off" : "on");

	seq_printf(s,	"- DSI%d -\n", dsi_module + 1);

	seq_printf(s,	"dsi fclk source = %s\n",
			dss_get_clk_source_name(dsi_clk_src));

	seq_printf(s,	"DSI_FCLK\t%lu\n", dsi_fclk_rate(dsi));

	seq_printf(s,	"DDR_CLK\t\t%lu\n",
			cinfo->clkdco / 4);

	seq_printf(s,	"TxByteClkHS\t%lu\n", dsi_get_txbyteclkhs(dsi));

	seq_printf(s,	"LP_CLK\t\t%lu\n", dsi->current_lp_cinfo.lp_clk);

	dsi_runtime_put(dsi);

	return 0;
}

#ifdef CONFIG_OMAP2_DSS_COLLECT_IRQ_STATS
static int dsi_dump_dsi_irqs(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;
	unsigned long flags;
	struct dsi_irq_stats stats;

	spin_lock_irqsave(&dsi->irq_stats_lock, flags);

	stats = dsi->irq_stats;
	memset(&dsi->irq_stats, 0, sizeof(dsi->irq_stats));
	dsi->irq_stats.last_reset = jiffies;

	spin_unlock_irqrestore(&dsi->irq_stats_lock, flags);

	seq_printf(s, "period %u ms\n",
			jiffies_to_msecs(jiffies - stats.last_reset));

	seq_printf(s, "irqs %d\n", stats.irq_count);
#define PIS(x) \
	seq_printf(s, "%-20s %10d\n", #x, stats.dsi_irqs[ffs(DSI_IRQ_##x)-1]);

	seq_printf(s, "-- DSI%d interrupts --\n", dsi->module_id + 1);
	PIS(VC0);
	PIS(VC1);
	PIS(VC2);
	PIS(VC3);
	PIS(WAKEUP);
	PIS(RESYNC);
	PIS(PLL_LOCK);
	PIS(PLL_UNLOCK);
	PIS(PLL_RECALL);
	PIS(COMPLEXIO_ERR);
	PIS(HS_TX_TIMEOUT);
	PIS(LP_RX_TIMEOUT);
	PIS(TE_TRIGGER);
	PIS(ACK_TRIGGER);
	PIS(SYNC_LOST);
	PIS(LDO_POWER_GOOD);
	PIS(TA_TIMEOUT);
#undef PIS

#define PIS(x) \
	seq_printf(s, "%-20s %10d %10d %10d %10d\n", #x, \
			stats.vc_irqs[0][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[1][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[2][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[3][ffs(DSI_VC_IRQ_##x)-1]);

	seq_printf(s, "-- VC interrupts --\n");
	PIS(CS);
	PIS(ECC_CORR);
	PIS(PACKET_SENT);
	PIS(FIFO_TX_OVF);
	PIS(FIFO_RX_OVF);
	PIS(BTA);
	PIS(ECC_NO_CORR);
	PIS(FIFO_TX_UDF);
	PIS(PP_BUSY_CHANGE);
#undef PIS

#define PIS(x) \
	seq_printf(s, "%-20s %10d\n", #x, \
			stats.cio_irqs[ffs(DSI_CIO_IRQ_##x)-1]);

	seq_printf(s, "-- CIO interrupts --\n");
	PIS(ERRSYNCESC1);
	PIS(ERRSYNCESC2);
	PIS(ERRSYNCESC3);
	PIS(ERRESC1);
	PIS(ERRESC2);
	PIS(ERRESC3);
	PIS(ERRCONTROL1);
	PIS(ERRCONTROL2);
	PIS(ERRCONTROL3);
	PIS(STATEULPS1);
	PIS(STATEULPS2);
	PIS(STATEULPS3);
	PIS(ERRCONTENTIONLP0_1);
	PIS(ERRCONTENTIONLP1_1);
	PIS(ERRCONTENTIONLP0_2);
	PIS(ERRCONTENTIONLP1_2);
	PIS(ERRCONTENTIONLP0_3);
	PIS(ERRCONTENTIONLP1_3);
	PIS(ULPSACTIVENOT_ALL0);
	PIS(ULPSACTIVENOT_ALL1);
#undef PIS

	return 0;
}
{
	struct dsi_data *dsi = p;
	unsigned long flags;
	struct dsi_irq_stats stats;

	spin_lock_irqsave(&dsi->irq_stats_lock, flags);

	stats = dsi->irq_stats;
	memset(&dsi->irq_stats, 0, sizeof(dsi->irq_stats));
	dsi->irq_stats.last_reset = jiffies;

	spin_unlock_irqrestore(&dsi->irq_stats_lock, flags);

	seq_printf(s, "period %u ms\n",
			jiffies_to_msecs(jiffies - stats.last_reset));

	seq_printf(s, "irqs %d\n", stats.irq_count);
#define PIS(x) \
	seq_printf(s, "%-20s %10d\n", #x, stats.dsi_irqs[ffs(DSI_IRQ_##x)-1]);

	seq_printf(s, "-- DSI%d interrupts --\n", dsi->module_id + 1);
	PIS(VC0);
	PIS(VC1);
	PIS(VC2);
	PIS(VC3);
	PIS(WAKEUP);
	PIS(RESYNC);
	PIS(PLL_LOCK);
	PIS(PLL_UNLOCK);
	PIS(PLL_RECALL);
	PIS(COMPLEXIO_ERR);
	PIS(HS_TX_TIMEOUT);
	PIS(LP_RX_TIMEOUT);
	PIS(TE_TRIGGER);
	PIS(ACK_TRIGGER);
	PIS(SYNC_LOST);
	PIS(LDO_POWER_GOOD);
	PIS(TA_TIMEOUT);
#undef PIS

#define PIS(x) \
	seq_printf(s, "%-20s %10d %10d %10d %10d\n", #x, \
			stats.vc_irqs[0][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[1][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[2][ffs(DSI_VC_IRQ_##x)-1], \
			stats.vc_irqs[3][ffs(DSI_VC_IRQ_##x)-1]);

	seq_printf(s, "-- VC interrupts --\n");
	PIS(CS);
	PIS(ECC_CORR);
	PIS(PACKET_SENT);
	PIS(FIFO_TX_OVF);
	PIS(FIFO_RX_OVF);
	PIS(BTA);
	PIS(ECC_NO_CORR);
	PIS(FIFO_TX_UDF);
	PIS(PP_BUSY_CHANGE);
#undef PIS

#define PIS(x) \
	seq_printf(s, "%-20s %10d\n", #x, \
			stats.cio_irqs[ffs(DSI_CIO_IRQ_##x)-1]);

	seq_printf(s, "-- CIO interrupts --\n");
	PIS(ERRSYNCESC1);
	PIS(ERRSYNCESC2);
	PIS(ERRSYNCESC3);
	PIS(ERRESC1);
	PIS(ERRESC2);
	PIS(ERRESC3);
	PIS(ERRCONTROL1);
	PIS(ERRCONTROL2);
	PIS(ERRCONTROL3);
	PIS(STATEULPS1);
	PIS(STATEULPS2);
	PIS(STATEULPS3);
	PIS(ERRCONTENTIONLP0_1);
	PIS(ERRCONTENTIONLP1_1);
	PIS(ERRCONTENTIONLP0_2);
	PIS(ERRCONTENTIONLP1_2);
	PIS(ERRCONTENTIONLP0_3);
	PIS(ERRCONTENTIONLP1_3);
	PIS(ULPSACTIVENOT_ALL0);
	PIS(ULPSACTIVENOT_ALL1);
#undef PIS

	return 0;
}
#endif

static int dsi_dump_dsi_regs(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;

	if (dsi_runtime_get(dsi))
		return 0;
	dsi_enable_scp_clk(dsi);

#define DUMPREG(r) seq_printf(s, "%-35s %08x\n", #r, dsi_read_reg(dsi, r))
	DUMPREG(DSI_REVISION);
	DUMPREG(DSI_SYSCONFIG);
	DUMPREG(DSI_SYSSTATUS);
	DUMPREG(DSI_IRQSTATUS);
	DUMPREG(DSI_IRQENABLE);
	DUMPREG(DSI_CTRL);
	DUMPREG(DSI_COMPLEXIO_CFG1);
	DUMPREG(DSI_COMPLEXIO_IRQ_STATUS);
	DUMPREG(DSI_COMPLEXIO_IRQ_ENABLE);
	DUMPREG(DSI_CLK_CTRL);
	DUMPREG(DSI_TIMING1);
	DUMPREG(DSI_TIMING2);
	DUMPREG(DSI_VM_TIMING1);
	DUMPREG(DSI_VM_TIMING2);
	DUMPREG(DSI_VM_TIMING3);
	DUMPREG(DSI_CLK_TIMING);
	DUMPREG(DSI_TX_FIFO_VC_SIZE);
	DUMPREG(DSI_RX_FIFO_VC_SIZE);
	DUMPREG(DSI_COMPLEXIO_CFG2);
	DUMPREG(DSI_RX_FIFO_VC_FULLNESS);
	DUMPREG(DSI_VM_TIMING4);
	DUMPREG(DSI_TX_FIFO_VC_EMPTINESS);
	DUMPREG(DSI_VM_TIMING5);
	DUMPREG(DSI_VM_TIMING6);
	DUMPREG(DSI_VM_TIMING7);
	DUMPREG(DSI_STOPCLK_TIMING);

	DUMPREG(DSI_VC_CTRL(0));
	DUMPREG(DSI_VC_TE(0));
	DUMPREG(DSI_VC_LONG_PACKET_HEADER(0));
	DUMPREG(DSI_VC_LONG_PACKET_PAYLOAD(0));
	DUMPREG(DSI_VC_SHORT_PACKET_HEADER(0));
	DUMPREG(DSI_VC_IRQSTATUS(0));
	DUMPREG(DSI_VC_IRQENABLE(0));

	DUMPREG(DSI_VC_CTRL(1));
	DUMPREG(DSI_VC_TE(1));
	DUMPREG(DSI_VC_LONG_PACKET_HEADER(1));
	DUMPREG(DSI_VC_LONG_PACKET_PAYLOAD(1));
	DUMPREG(DSI_VC_SHORT_PACKET_HEADER(1));
	DUMPREG(DSI_VC_IRQSTATUS(1));
	DUMPREG(DSI_VC_IRQENABLE(1));

	DUMPREG(DSI_VC_CTRL(2));
	DUMPREG(DSI_VC_TE(2));
	DUMPREG(DSI_VC_LONG_PACKET_HEADER(2));
	DUMPREG(DSI_VC_LONG_PACKET_PAYLOAD(2));
	DUMPREG(DSI_VC_SHORT_PACKET_HEADER(2));
	DUMPREG(DSI_VC_IRQSTATUS(2));
	DUMPREG(DSI_VC_IRQENABLE(2));

	DUMPREG(DSI_VC_CTRL(3));
	DUMPREG(DSI_VC_TE(3));
	DUMPREG(DSI_VC_LONG_PACKET_HEADER(3));
	DUMPREG(DSI_VC_LONG_PACKET_PAYLOAD(3));
	DUMPREG(DSI_VC_SHORT_PACKET_HEADER(3));
	DUMPREG(DSI_VC_IRQSTATUS(3));
	DUMPREG(DSI_VC_IRQENABLE(3));

	DUMPREG(DSI_DSIPHY_CFG0);
	DUMPREG(DSI_DSIPHY_CFG1);
	DUMPREG(DSI_DSIPHY_CFG2);
	DUMPREG(DSI_DSIPHY_CFG5);

	DUMPREG(DSI_PLL_CONTROL);
	DUMPREG(DSI_PLL_STATUS);
	DUMPREG(DSI_PLL_GO);
	DUMPREG(DSI_PLL_CONFIGURATION1);
	DUMPREG(DSI_PLL_CONFIGURATION2);
#undef DUMPREG

	dsi_disable_scp_clk(dsi);
	dsi_runtime_put(dsi);

	return 0;
}
	if (r) {
		DSSERR("failed to find suitable DSI LP clock settings\n");
		goto err;
	}

	dsi->user_dsi_cinfo = ctx.dsi_cinfo;
	dsi->user_dispc_cinfo = ctx.dispc_cinfo;

	dsi->vm = ctx.vm;

	/*
	 * override interlace, logic level and edge related parameters in
	 * videomode with default values
	 */
	dsi->vm.flags &= ~DISPLAY_FLAGS_INTERLACED;
	dsi->vm.flags &= ~DISPLAY_FLAGS_HSYNC_LOW;
	dsi->vm.flags |= DISPLAY_FLAGS_HSYNC_HIGH;
	dsi->vm.flags &= ~DISPLAY_FLAGS_VSYNC_LOW;
	dsi->vm.flags |= DISPLAY_FLAGS_VSYNC_HIGH;

	dss_mgr_set_timings(&dsi->output, &dsi->vm);

	dsi->vm_timings = ctx.dsi_vm;

	mutex_unlock(&dsi->lock);

	return 0;
err:
	mutex_unlock(&dsi->lock);

	return r;
}

/*
 * Return a hardcoded channel for the DSI output. This should work for
 * current use cases, but this can be later expanded to either resolve
 * the channel in some more dynamic manner, or get the channel as a user
 * parameter.
 */
static enum omap_channel dsi_get_channel(struct dsi_data *dsi)
{
	switch (dsi->data->model) {
	case DSI_MODEL_OMAP3:
		return OMAP_DSS_CHANNEL_LCD;

	case DSI_MODEL_OMAP4:
		switch (dsi->module_id) {
		case 0:
			return OMAP_DSS_CHANNEL_LCD;
		case 1:
			return OMAP_DSS_CHANNEL_LCD2;
		default:
			DSSWARN("unsupported module id\n");
			return OMAP_DSS_CHANNEL_LCD;
		}

	case DSI_MODEL_OMAP5:
		switch (dsi->module_id) {
		case 0:
			return OMAP_DSS_CHANNEL_LCD;
		case 1:
			return OMAP_DSS_CHANNEL_LCD3;
		default:
			DSSWARN("unsupported module id\n");
			return OMAP_DSS_CHANNEL_LCD;
		}

	default:
		DSSWARN("unsupported DSS version\n");
		return OMAP_DSS_CHANNEL_LCD;
	}
{
	struct dss_device *dss = dss_get_device(master);
	struct dsi_data *dsi = dev_get_drvdata(dev);
	char name[10];
	u32 rev;
	int r;

	dsi->dss = dss;

	dsi_init_pll_data(dss, dsi);

	r = dsi_runtime_get(dsi);
	if (r)
		return r;

	rev = dsi_read_reg(dsi, DSI_REVISION);
	dev_dbg(dev, "OMAP DSI rev %d.%d\n",
	       FLD_GET(rev, 7, 4), FLD_GET(rev, 3, 0));

	dsi->line_buffer_size = dsi_get_line_buf_size(dsi);

	dsi_runtime_put(dsi);

	snprintf(name, sizeof(name), "dsi%u_regs", dsi->module_id + 1);
	dsi->debugfs.regs = dss_debugfs_create_file(dss, name,
						    dsi_dump_dsi_regs, &dsi);
#ifdef CONFIG_OMAP2_DSS_COLLECT_IRQ_STATS
	snprintf(name, sizeof(name), "dsi%u_irqs", dsi->module_id + 1);
	dsi->debugfs.irqs = dss_debugfs_create_file(dss, name,
						    dsi_dump_dsi_irqs, &dsi);
#endif
	snprintf(name, sizeof(name), "dsi%u_clks", dsi->module_id + 1);
	dsi->debugfs.clks = dss_debugfs_create_file(dss, name,
						    dsi_dump_dsi_clocks, &dsi);

	return 0;
}

static void dsi_unbind(struct device *dev, struct device *master, void *data)
{
	struct dsi_data *dsi = dev_get_drvdata(dev);

	dss_debugfs_remove_file(dsi->debugfs.clks);
	dss_debugfs_remove_file(dsi->debugfs.irqs);
	dss_debugfs_remove_file(dsi->debugfs.regs);

	of_platform_depopulate(dev);

	WARN_ON(dsi->scp_clk_refcount > 0);

	dss_pll_unregister(&dsi->pll);
}

static const struct component_ops dsi_component_ops = {
	.bind	= dsi_bind,
	.unbind	= dsi_unbind,
};

/* -----------------------------------------------------------------------------
 * Probe & Remove, Suspend & Resume
 */

static int dsi_init_output(struct dsi_data *dsi)
{
	struct omap_dss_device *out = &dsi->output;
	int r;

	out->dev = dsi->dev;
	out->id = dsi->module_id == 0 ?
			OMAP_DSS_OUTPUT_DSI1 : OMAP_DSS_OUTPUT_DSI2;

	out->output_type = OMAP_DISPLAY_TYPE_DSI;
	out->name = dsi->module_id == 0 ? "dsi.0" : "dsi.1";
	out->dispc_channel = dsi_get_channel(dsi);
	out->ops = &dsi_ops;
	out->owner = THIS_MODULE;
	out->of_ports = BIT(0);
	out->bus_flags = DRM_BUS_FLAG_PIXDATA_POSEDGE
		       | DRM_BUS_FLAG_DE_HIGH
		       | DRM_BUS_FLAG_SYNC_NEGEDGE;

	out->next = omapdss_of_find_connected_device(out->dev->of_node, 0);
	if (IS_ERR(out->next)) {
		if (PTR_ERR(out->next) != -EPROBE_DEFER)
			dev_err(out->dev, "failed to find video sink\n");
		return PTR_ERR(out->next);
	}

	r = omapdss_output_validate(out);
	if (r) {
		omapdss_device_put(out->next);
		out->next = NULL;
		return r;
	}

	omapdss_device_register(out);

	return 0;
}
{
	struct dsi_data *dsi = dev_get_drvdata(dev);

	dss_debugfs_remove_file(dsi->debugfs.clks);
	dss_debugfs_remove_file(dsi->debugfs.irqs);
	dss_debugfs_remove_file(dsi->debugfs.regs);

	of_platform_depopulate(dev);

	WARN_ON(dsi->scp_clk_refcount > 0);

	dss_pll_unregister(&dsi->pll);
}

static const struct component_ops dsi_component_ops = {
	.bind	= dsi_bind,
	.unbind	= dsi_unbind,
};

/* -----------------------------------------------------------------------------
 * Probe & Remove, Suspend & Resume
 */

static int dsi_init_output(struct dsi_data *dsi)
{
	struct omap_dss_device *out = &dsi->output;
	int r;

	out->dev = dsi->dev;
	out->id = dsi->module_id == 0 ?
			OMAP_DSS_OUTPUT_DSI1 : OMAP_DSS_OUTPUT_DSI2;

	out->output_type = OMAP_DISPLAY_TYPE_DSI;
	out->name = dsi->module_id == 0 ? "dsi.0" : "dsi.1";
	out->dispc_channel = dsi_get_channel(dsi);
	out->ops = &dsi_ops;
	out->owner = THIS_MODULE;
	out->of_ports = BIT(0);
	out->bus_flags = DRM_BUS_FLAG_PIXDATA_POSEDGE
		       | DRM_BUS_FLAG_DE_HIGH
		       | DRM_BUS_FLAG_SYNC_NEGEDGE;

	out->next = omapdss_of_find_connected_device(out->dev->of_node, 0);
	if (IS_ERR(out->next)) {
		if (PTR_ERR(out->next) != -EPROBE_DEFER)
			dev_err(out->dev, "failed to find video sink\n");
		return PTR_ERR(out->next);
	}

	r = omapdss_output_validate(out);
	if (r) {
		omapdss_device_put(out->next);
		out->next = NULL;
		return r;
	}

	omapdss_device_register(out);

	return 0;
}
{
	struct dsi_data *dsi = platform_get_drvdata(pdev);

	component_del(&pdev->dev, &dsi_component_ops);

	dsi_uninit_output(dsi);

	pm_runtime_disable(&pdev->dev);

	if (dsi->vdds_dsi_reg != NULL && dsi->vdds_dsi_enabled) {
		regulator_disable(dsi->vdds_dsi_reg);
		dsi->vdds_dsi_enabled = false;
	}