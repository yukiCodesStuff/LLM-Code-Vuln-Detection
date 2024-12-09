
	err = configure_kernel(info, out_info, vf_info, downscale_log2, &config);
	configure_dma(&config, vf_info);
	if (binary) {
		if (vf_info)
			vf_info->raw_bit_depth = info->dma.vfdec_bits_per_pixel;
		ia_css_configure_vf (binary, &config);
	}
	return IA_CSS_SUCCESS;
}
