struct ia_css_binary_descr {
	int mode;
	bool online;
	bool continuous;
	bool striped;
	bool two_ppc;
	bool enable_yuv_ds;
	bool enable_high_speed;
	bool enable_dvs_6axis;
	bool enable_reduced_pipe;
	bool enable_dz;
	bool enable_xnr;
	bool enable_fractional_ds;
	bool enable_dpc;
#ifdef ISP2401
	bool enable_luma_only;
	bool enable_tnr;
#endif
	bool enable_capture_pp_bli;
	struct ia_css_resolution dvs_env;
	enum ia_css_stream_format stream_format;
	struct ia_css_frame_info *in_info;		/* the info of the input-frame with the
							   ISP required resolution. */
	struct ia_css_frame_info *bds_out_info;
	struct ia_css_frame_info *out_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_frame_info *vf_info;
	unsigned int isp_pipe_version;
	unsigned int required_bds_factor;
	int stream_config_left_padding;
};

struct ia_css_binary {
	const struct ia_css_binary_xinfo *info;
	enum ia_css_stream_format input_format;
	struct ia_css_frame_info in_frame_info;
	struct ia_css_frame_info internal_frame_info;
	struct ia_css_frame_info out_frame_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_resolution effective_in_frame_res;
	struct ia_css_frame_info vf_frame_info;
	int                      input_buf_vectors;
	int                      deci_factor_log2;
	int                      vf_downscale_log2;
	int                      s3atbl_width;
	int                      s3atbl_height;
	int                      s3atbl_isp_width;
	int                      s3atbl_isp_height;
	unsigned int             morph_tbl_width;
	unsigned int             morph_tbl_aligned_width;
	unsigned int             morph_tbl_height;
	int                      sctbl_width_per_color;
	int                      sctbl_aligned_width_per_color;
	int                      sctbl_height;
#ifdef ISP2401
	int                      sctbl_legacy_width_per_color;
	int                      sctbl_legacy_height;
#endif
	struct ia_css_sdis_info	 dis;
	struct ia_css_resolution dvs_envelope;
	bool                     online;
	unsigned int             uds_xc;
	unsigned int             uds_yc;
	unsigned int             left_padding;
	struct sh_css_binary_metrics metrics;
	struct ia_css_isp_param_host_segments mem_params;
	struct ia_css_isp_param_css_segments  css_params;
};

#define IA_CSS_BINARY_DEFAULT_SETTINGS \
(struct ia_css_binary) { \
	.input_format		= IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY, \
	.in_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.internal_frame_info	= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.out_frame_info		= {IA_CSS_BINARY_DEFAULT_FRAME_INFO}, \
	.vf_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
}
struct ia_css_binary {
	const struct ia_css_binary_xinfo *info;
	enum ia_css_stream_format input_format;
	struct ia_css_frame_info in_frame_info;
	struct ia_css_frame_info internal_frame_info;
	struct ia_css_frame_info out_frame_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_resolution effective_in_frame_res;
	struct ia_css_frame_info vf_frame_info;
	int                      input_buf_vectors;
	int                      deci_factor_log2;
	int                      vf_downscale_log2;
	int                      s3atbl_width;
	int                      s3atbl_height;
	int                      s3atbl_isp_width;
	int                      s3atbl_isp_height;
	unsigned int             morph_tbl_width;
	unsigned int             morph_tbl_aligned_width;
	unsigned int             morph_tbl_height;
	int                      sctbl_width_per_color;
	int                      sctbl_aligned_width_per_color;
	int                      sctbl_height;
#ifdef ISP2401
	int                      sctbl_legacy_width_per_color;
	int                      sctbl_legacy_height;
#endif
	struct ia_css_sdis_info	 dis;
	struct ia_css_resolution dvs_envelope;
	bool                     online;
	unsigned int             uds_xc;
	unsigned int             uds_yc;
	unsigned int             left_padding;
	struct sh_css_binary_metrics metrics;
	struct ia_css_isp_param_host_segments mem_params;
	struct ia_css_isp_param_css_segments  css_params;
};

#define IA_CSS_BINARY_DEFAULT_SETTINGS \
(struct ia_css_binary) { \
	.input_format		= IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY, \
	.in_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.internal_frame_info	= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.out_frame_info		= {IA_CSS_BINARY_DEFAULT_FRAME_INFO}, \
	.vf_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
}
struct ia_css_binary {
	const struct ia_css_binary_xinfo *info;
	enum ia_css_stream_format input_format;
	struct ia_css_frame_info in_frame_info;
	struct ia_css_frame_info internal_frame_info;
	struct ia_css_frame_info out_frame_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_resolution effective_in_frame_res;
	struct ia_css_frame_info vf_frame_info;
	int                      input_buf_vectors;
	int                      deci_factor_log2;
	int                      vf_downscale_log2;
	int                      s3atbl_width;
	int                      s3atbl_height;
	int                      s3atbl_isp_width;
	int                      s3atbl_isp_height;
	unsigned int             morph_tbl_width;
	unsigned int             morph_tbl_aligned_width;
	unsigned int             morph_tbl_height;
	int                      sctbl_width_per_color;
	int                      sctbl_aligned_width_per_color;
	int                      sctbl_height;
#ifdef ISP2401
	int                      sctbl_legacy_width_per_color;
	int                      sctbl_legacy_height;
#endif
	struct ia_css_sdis_info	 dis;
	struct ia_css_resolution dvs_envelope;
	bool                     online;
	unsigned int             uds_xc;
	unsigned int             uds_yc;
	unsigned int             left_padding;
	struct sh_css_binary_metrics metrics;
	struct ia_css_isp_param_host_segments mem_params;
	struct ia_css_isp_param_css_segments  css_params;
};

#define IA_CSS_BINARY_DEFAULT_SETTINGS \
(struct ia_css_binary) { \
	.input_format		= IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY, \
	.in_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.internal_frame_info	= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.out_frame_info		= {IA_CSS_BINARY_DEFAULT_FRAME_INFO}, \
	.vf_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
}

enum ia_css_err
ia_css_binary_init_infos(void);

enum ia_css_err
ia_css_binary_uninit(void);

enum ia_css_err
ia_css_binary_fill_info(const struct ia_css_binary_xinfo *xinfo,
		 bool online,
		 bool two_ppc,
		 enum ia_css_stream_format stream_format,
		 const struct ia_css_frame_info *in_info,
		 const struct ia_css_frame_info *bds_out_info,
		 const struct ia_css_frame_info *out_info[],
		 const struct ia_css_frame_info *vf_info,
		 struct ia_css_binary *binary,
		 struct ia_css_resolution *dvs_env,
		 int stream_config_left_padding,
		 bool accelerator);

enum ia_css_err
ia_css_binary_find(struct ia_css_binary_descr *descr,
		   struct ia_css_binary *binary);

/* @brief Get the shading information of the specified shading correction type.
 *
 * @param[in] binary: The isp binary which has the shading correction.
 * @param[in] type: The shading correction type.
 * @param[in] required_bds_factor: The bayer downscaling factor required in the pipe.
 * @param[in] stream_config: The stream configuration.
#ifndef ISP2401
 * @param[out] info: The shading information.
#else
 * @param[out] shading_info: The shading information.
 *		The shading information necessary as API is stored in the shading_info.
#endif
 *		The driver needs to get this information to generate
#ifndef ISP2401
 *		the shading table directly required in the isp.
#else
 *		the shading table directly required from ISP.
 * @param[out] pipe_config: The pipe configuration.
 *		The shading information related to ISP (but, not necessary as API) is stored in the pipe_config.
#endif
 * @return	IA_CSS_SUCCESS or error code upon error.
 *
 */
enum ia_css_err
ia_css_binary_get_shading_info(const struct ia_css_binary *binary,
			enum ia_css_shading_correction_type type,
			unsigned int required_bds_factor,
			const struct ia_css_stream_config *stream_config,
#ifndef ISP2401
			struct ia_css_shading_info *info);
#else
			struct ia_css_shading_info *shading_info,
			struct ia_css_pipe_config *pipe_config);
#endif

enum ia_css_err
ia_css_binary_3a_grid_info(const struct ia_css_binary *binary,
			   struct ia_css_grid_info *info,
			   struct ia_css_pipe *pipe);

void
ia_css_binary_dvs_grid_info(const struct ia_css_binary *binary,
			    struct ia_css_grid_info *info,
			    struct ia_css_pipe *pipe);

void
ia_css_binary_dvs_stat_grid_info(
	const struct ia_css_binary *binary,
	struct ia_css_grid_info *info,
	struct ia_css_pipe *pipe);

unsigned
ia_css_binary_max_vf_width(void);

void
ia_css_binary_destroy_isp_parameters(struct ia_css_binary *binary);

void
ia_css_binary_get_isp_binaries(struct ia_css_binary_xinfo **binaries,
	uint32_t *num_isp_binaries);

#endif /* _IA_CSS_BINARY_H_ */
struct ia_css_binary {
	const struct ia_css_binary_xinfo *info;
	enum ia_css_stream_format input_format;
	struct ia_css_frame_info in_frame_info;
	struct ia_css_frame_info internal_frame_info;
	struct ia_css_frame_info out_frame_info[IA_CSS_BINARY_MAX_OUTPUT_PORTS];
	struct ia_css_resolution effective_in_frame_res;
	struct ia_css_frame_info vf_frame_info;
	int                      input_buf_vectors;
	int                      deci_factor_log2;
	int                      vf_downscale_log2;
	int                      s3atbl_width;
	int                      s3atbl_height;
	int                      s3atbl_isp_width;
	int                      s3atbl_isp_height;
	unsigned int             morph_tbl_width;
	unsigned int             morph_tbl_aligned_width;
	unsigned int             morph_tbl_height;
	int                      sctbl_width_per_color;
	int                      sctbl_aligned_width_per_color;
	int                      sctbl_height;
#ifdef ISP2401
	int                      sctbl_legacy_width_per_color;
	int                      sctbl_legacy_height;
#endif
	struct ia_css_sdis_info	 dis;
	struct ia_css_resolution dvs_envelope;
	bool                     online;
	unsigned int             uds_xc;
	unsigned int             uds_yc;
	unsigned int             left_padding;
	struct sh_css_binary_metrics metrics;
	struct ia_css_isp_param_host_segments mem_params;
	struct ia_css_isp_param_css_segments  css_params;
};

#define IA_CSS_BINARY_DEFAULT_SETTINGS \
(struct ia_css_binary) { \
	.input_format		= IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY, \
	.in_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.internal_frame_info	= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
	.out_frame_info		= {IA_CSS_BINARY_DEFAULT_FRAME_INFO}, \
	.vf_frame_info		= IA_CSS_BINARY_DEFAULT_FRAME_INFO, \
}

enum ia_css_err
ia_css_binary_init_infos(void);

enum ia_css_err
ia_css_binary_uninit(void);

enum ia_css_err
ia_css_binary_fill_info(const struct ia_css_binary_xinfo *xinfo,
		 bool online,
		 bool two_ppc,
		 enum ia_css_stream_format stream_format,
		 const struct ia_css_frame_info *in_info,
		 const struct ia_css_frame_info *bds_out_info,
		 const struct ia_css_frame_info *out_info[],
		 const struct ia_css_frame_info *vf_info,
		 struct ia_css_binary *binary,
		 struct ia_css_resolution *dvs_env,
		 int stream_config_left_padding,
		 bool accelerator);

enum ia_css_err
ia_css_binary_find(struct ia_css_binary_descr *descr,
		   struct ia_css_binary *binary);

/* @brief Get the shading information of the specified shading correction type.
 *
 * @param[in] binary: The isp binary which has the shading correction.
 * @param[in] type: The shading correction type.
 * @param[in] required_bds_factor: The bayer downscaling factor required in the pipe.
 * @param[in] stream_config: The stream configuration.
#ifndef ISP2401
 * @param[out] info: The shading information.
#else
 * @param[out] shading_info: The shading information.
 *		The shading information necessary as API is stored in the shading_info.
#endif
 *		The driver needs to get this information to generate
#ifndef ISP2401
 *		the shading table directly required in the isp.
#else
 *		the shading table directly required from ISP.
 * @param[out] pipe_config: The pipe configuration.
 *		The shading information related to ISP (but, not necessary as API) is stored in the pipe_config.
#endif
 * @return	IA_CSS_SUCCESS or error code upon error.
 *
 */
enum ia_css_err
ia_css_binary_get_shading_info(const struct ia_css_binary *binary,
			enum ia_css_shading_correction_type type,
			unsigned int required_bds_factor,
			const struct ia_css_stream_config *stream_config,
#ifndef ISP2401
			struct ia_css_shading_info *info);
#else
			struct ia_css_shading_info *shading_info,
			struct ia_css_pipe_config *pipe_config);
#endif

enum ia_css_err
ia_css_binary_3a_grid_info(const struct ia_css_binary *binary,
			   struct ia_css_grid_info *info,
			   struct ia_css_pipe *pipe);

void
ia_css_binary_dvs_grid_info(const struct ia_css_binary *binary,
			    struct ia_css_grid_info *info,
			    struct ia_css_pipe *pipe);

void
ia_css_binary_dvs_stat_grid_info(
	const struct ia_css_binary *binary,
	struct ia_css_grid_info *info,
	struct ia_css_pipe *pipe);

unsigned
ia_css_binary_max_vf_width(void);

void
ia_css_binary_destroy_isp_parameters(struct ia_css_binary *binary);

void
ia_css_binary_get_isp_binaries(struct ia_css_binary_xinfo **binaries,
	uint32_t *num_isp_binaries);

#endif /* _IA_CSS_BINARY_H_ */