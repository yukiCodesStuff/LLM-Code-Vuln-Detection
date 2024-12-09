			const struct ia_css_metadata_config *md_config,
			const struct ia_css_metadata_info *md_info,
#if !defined(HAS_NO_INPUT_SYSTEM)
			const mipi_port_ID_t port_id
#endif
#ifdef ISP2401
			,
			const struct ia_css_coordinate *internal_frame_origin_bqs_on_sctbl, /* Origin of internal frame