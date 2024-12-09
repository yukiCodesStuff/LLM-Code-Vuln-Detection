#ifdef ISP2401
static bool
binary_supports_input_format(const struct ia_css_binary_xinfo *info,
			     enum ia_css_stream_format format)
{

	assert(info != NULL);
	(void)format;
ia_css_binary_fill_info(const struct ia_css_binary_xinfo *xinfo,
		 bool online,
		 bool two_ppc,
		 enum ia_css_stream_format stream_format,
		 const struct ia_css_frame_info *in_info, /* can be NULL */
		 const struct ia_css_frame_info *bds_out_info, /* can be NULL */
		 const struct ia_css_frame_info *out_info[], /* can be NULL */
		 const struct ia_css_frame_info *vf_info, /* can be NULL */
	int mode;
	bool online;
	bool two_ppc;
	enum ia_css_stream_format stream_format;
	const struct ia_css_frame_info *req_in_info,
				       *req_bds_out_info,
				       *req_out_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS],
				       *req_bin_out_info = NULL,