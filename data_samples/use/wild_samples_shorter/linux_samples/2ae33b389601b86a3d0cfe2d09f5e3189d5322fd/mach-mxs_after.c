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