{
	enable_clk_enet_out();
	update_fec_mac_prop(OUI_CRYSTALFONTZ);

	mxsfb_pdata.mode_list = cfa10049_video_modes;
	mxsfb_pdata.mode_count = ARRAY_SIZE(cfa10049_video_modes);
	mxsfb_pdata.default_bpp = 32;
	mxsfb_pdata.ld_intf_width = STMLCDIF_18BIT;
}

static void __init cfa10037_init(void)
{
	enable_clk_enet_out();
	update_fec_mac_prop(OUI_CRYSTALFONTZ);
}

static void __init apf28_init(void)
{
	enable_clk_enet_out();

	mxsfb_pdata.mode_list = apf28dev_video_modes;
	mxsfb_pdata.mode_count = ARRAY_SIZE(apf28dev_video_modes);
	mxsfb_pdata.default_bpp = 16;
	mxsfb_pdata.ld_intf_width = STMLCDIF_16BIT;
}

static void __init mxs_machine_init(void)
{
	if (of_machine_is_compatible("fsl,imx28-evk"))
		imx28_evk_init();
	else if (of_machine_is_compatible("fsl,imx23-evk"))
		imx23_evk_init();
	else if (of_machine_is_compatible("denx,m28evk"))
		m28evk_init();
	else if (of_machine_is_compatible("bluegiga,apx4devkit"))
		apx4devkit_init();
	else if (of_machine_is_compatible("crystalfontz,cfa10037"))
		cfa10037_init();
	else if (of_machine_is_compatible("crystalfontz,cfa10049"))
		cfa10049_init();
	else if (of_machine_is_compatible("armadeus,imx28-apf28"))
		apf28_init();
	else if (of_machine_is_compatible("schulercontrol,imx28-sps1"))
		sc_sps1_init();

	of_platform_populate(NULL, of_default_bus_match_table,
			     mxs_auxdata_lookup, NULL);

	if (of_machine_is_compatible("karo,tx28"))
		tx28_post_init();

	if (of_machine_is_compatible("fsl,imx28-evk"))
		imx28_evk_post_init();
}

static const char *imx23_dt_compat[] __initdata = {
	"fsl,imx23",
	NULL,
};

static const char *imx28_dt_compat[] __initdata = {
	"fsl,imx28",
	NULL,
};

DT_MACHINE_START(IMX23, "Freescale i.MX23 (Device Tree)")
	.map_io		= mx23_map_io,
	.init_irq	= icoll_init_irq,
	.handle_irq	= icoll_handle_irq,
	.init_time	= imx23_timer_init,
	.init_machine	= mxs_machine_init,
	.dt_compat	= imx23_dt_compat,
	.restart	= mxs_restart,
MACHINE_END

DT_MACHINE_START(IMX28, "Freescale i.MX28 (Device Tree)")
	.map_io		= mx28_map_io,
	.init_irq	= icoll_init_irq,
	.handle_irq	= icoll_handle_irq,
	.init_time	= imx28_timer_init,
	.init_machine	= mxs_machine_init,
	.dt_compat	= imx28_dt_compat,
	.restart	= mxs_restart,
MACHINE_END