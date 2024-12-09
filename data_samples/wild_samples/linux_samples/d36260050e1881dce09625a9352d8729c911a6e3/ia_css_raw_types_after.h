struct ia_css_raw_configuration {
	const struct sh_css_sp_pipeline *pipe;
	const struct ia_css_frame_info  *in_info;
	const struct ia_css_frame_info  *internal_info;
	bool two_ppc;
	enum atomisp_input_format stream_format;
	bool deinterleaved;
	uint8_t enable_left_padding;
};

#endif /* __IA_CSS_RAW_TYPES_H */
