struct sh_css_sp_group		sh_css_sp_group;
struct sh_css_sp_stage		sh_css_sp_stage;
struct sh_css_isp_stage		sh_css_isp_stage;
struct sh_css_sp_output		sh_css_sp_output;
static struct sh_css_sp_per_frame_data per_frame_data;

/* true if SP supports frame loop and host2sp_commands */
/* For the moment there is only code that sets this bool to true */
	*/
	sh_css_sp_stage.enable.sdis = sh_css_isp_stage.binary_info.enable.dis;
	sh_css_sp_stage.enable.s3a = sh_css_isp_stage.binary_info.enable.s3a;
#ifdef ISP2401	
	sh_css_sp_stage.enable.lace_stats = sh_css_isp_stage.binary_info.enable.lace_stats;
#endif	
}

void
store_sp_stage_data(enum ia_css_pipe_id id, unsigned int pipe_num, unsigned stage)

static void
sh_css_sp_init_group(bool two_ppc,
		     enum ia_css_stream_format input_format,
		     bool no_isp_sync,
		     uint8_t if_config_index)
{
#if !defined(HAS_NO_INPUT_FORMATTER)
	bool two_ppc,
	bool deinterleaved)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
#ifdef ISP2401
	struct ia_css_pipe *pipe = find_pipe_by_num(pipeline->pipe_num);
	const struct ia_css_resolution *res;

	ia_css_ref_configure(binary, (const struct ia_css_frame **)args->delay_frames, pipeline->dvs_frame_delay);
	ia_css_tnr_configure(binary, (const struct ia_css_frame **)args->tnr_frames);
	ia_css_bayer_io_config(binary, args);
	return err;
}

static void
initialize_isp_states(const struct ia_css_binary *binary)
			out_infos[0] = &args->out_frame[0]->info;
		info = &firmware->info.isp;
		ia_css_binary_fill_info(info, false, false,
			    IA_CSS_STREAM_FORMAT_RAW_10,
			    args->in_frame  ? &args->in_frame->info  : NULL,
			    NULL,
				out_infos,
			    args->out_vf_frame ? &args->out_vf_frame->info
			const struct ia_css_metadata_config *md_config,
			const struct ia_css_metadata_info *md_info,
#if !defined(HAS_NO_INPUT_SYSTEM)
			const mipi_port_ID_t port_id
#endif
#ifdef ISP2401
			,
			const struct ia_css_coordinate *internal_frame_origin_bqs_on_sctbl, /* Origin of internal frame
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	assert(frame_num < NUM_CONTINUOUS_FRAMES);

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int o;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {