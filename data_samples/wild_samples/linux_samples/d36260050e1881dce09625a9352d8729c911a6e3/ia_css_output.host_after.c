{
	unsigned elems_a = ISP_VEC_NELEMS;

	(void)size;
	ia_css_dma_configure_from_info(&to->port_b, from->info);
	to->width_a_over_b = elems_a / to->port_b.elems;
	to->height = from->info ? from->info->res.height : 0;
	to->enable = from->info != NULL;
	ia_css_frame_info_to_frame_sp_info(&to->info, from->info);

	/* Assume divisiblity here, may need to generalize to fixed point. */
	assert (elems_a % to->port_b.elems == 0);
}

void
ia_css_output0_config(
	struct sh_css_isp_output_isp_config       *to,
	const struct ia_css_output0_configuration *from,
	unsigned size)
{
	ia_css_output_config (
		to, (const struct ia_css_output_configuration *)from, size);
}

void
ia_css_output1_config(
	struct sh_css_isp_output_isp_config       *to,
	const struct ia_css_output1_configuration *from,
	unsigned size)
{
	ia_css_output_config (
		to, (const struct ia_css_output_configuration *)from, size);
}

void
ia_css_output_configure(
	const struct ia_css_binary     *binary,
	const struct ia_css_frame_info *info)
{
	if (NULL != info) {
		struct ia_css_output_configuration config =
				default_output_configuration;

		config.info = info;

		ia_css_configure_output(binary, &config);
	}