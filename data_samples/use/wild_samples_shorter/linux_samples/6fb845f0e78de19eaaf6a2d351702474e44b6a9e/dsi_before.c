
static int dsi_dump_dsi_clocks(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;
	struct dss_pll_clock_info *cinfo = &dsi->pll.cinfo;
	enum dss_clk_source dispc_clk_src, dsi_clk_src;
	int dsi_module = dsi->module_id;
	struct dss_pll *pll = &dsi->pll;
#ifdef CONFIG_OMAP2_DSS_COLLECT_IRQ_STATS
static int dsi_dump_dsi_irqs(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;
	unsigned long flags;
	struct dsi_irq_stats stats;

	spin_lock_irqsave(&dsi->irq_stats_lock, flags);

static int dsi_dump_dsi_regs(struct seq_file *s, void *p)
{
	struct dsi_data *dsi = p;

	if (dsi_runtime_get(dsi))
		return 0;
	dsi_enable_scp_clk(dsi);
	dsi->vm.flags |= DISPLAY_FLAGS_HSYNC_HIGH;
	dsi->vm.flags &= ~DISPLAY_FLAGS_VSYNC_LOW;
	dsi->vm.flags |= DISPLAY_FLAGS_VSYNC_HIGH;

	dss_mgr_set_timings(&dsi->output, &dsi->vm);

	dsi->vm_timings = ctx.dsi_vm;

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

	dss_debugfs_remove_file(dsi->debugfs.irqs);
	dss_debugfs_remove_file(dsi->debugfs.regs);

	of_platform_depopulate(dev);

	WARN_ON(dsi->scp_clk_refcount > 0);

	dss_pll_unregister(&dsi->pll);
}

	dsi_uninit_output(dsi);

	pm_runtime_disable(&pdev->dev);

	if (dsi->vdds_dsi_reg != NULL && dsi->vdds_dsi_enabled) {
		regulator_disable(dsi->vdds_dsi_reg);