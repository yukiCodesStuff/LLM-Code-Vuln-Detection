	(void)size;
	ia_css_dma_configure_from_info(&to->port_b, from->info);
	to->width_a_over_b = elems_a / to->port_b.elems;
	to->height = from->info->res.height;
	to->enable = from->info != NULL;
	ia_css_frame_info_to_frame_sp_info(&to->info, from->info);

	/* Assume divisiblity here, may need to generalize to fixed point. */