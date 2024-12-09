#endif
	bool enable_capture_pp_bli;
	struct ia_css_resolution dvs_env;
	enum ia_css_stream_format stream_format;
	struct ia_css_frame_info *in_info;		/* the info of the input-frame with the
							   ISP required resolution. */
	struct ia_css_frame_info *bds_out_info;
	struct ia_css_frame_info *out_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];

struct ia_css_binary {
	const struct ia_css_binary_xinfo *info;
	enum ia_css_stream_format input_format;
	struct ia_css_frame_info in_frame_info;
	struct ia_css_frame_info internal_frame_info;
	struct ia_css_frame_info out_frame_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_resolution effective_in_frame_res;

#define IA_CSS_BINARY_DEFAULT_SETTINGS \
(struct ia_css_binary) { \
	.input_format		= IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY, \
	.in_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.internal_frame_info	= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.out_frame_info		= {IA_CSS_BINARY_DEFAULT_FRAME_INFO}, \
	.vf_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
ia_css_binary_fill_info(const struct ia_css_binary_xinfo *xinfo,
		 bool online,
		 bool two_ppc,
		 enum ia_css_stream_format stream_format,
		 const struct ia_css_frame_info *in_info,
		 const struct ia_css_frame_info *bds_out_info,
		 const struct ia_css_frame_info *out_info[],
		 const struct ia_css_frame_info *vf_info,