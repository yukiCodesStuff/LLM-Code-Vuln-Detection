{
	enum ia_css_err err;
	struct ia_css_vf_configuration config;
	const struct ia_css_binary_info *info = &binary->info->sp;

	err = configure_kernel(info, out_info, vf_info, downscale_log2, &config);
	configure_dma(&config, vf_info);

	if (vf_info)
		vf_info->raw_bit_depth = info->dma.vfdec_bits_per_pixel;
	ia_css_configure_vf (binary, &config);

	return IA_CSS_SUCCESS;
}
