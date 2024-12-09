/*
 * Support for Intel Camera Imaging ISP subsystem.
 * Copyright (c) 2015, Intel Corporation.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 */

#include "sh_css_sp.h"

#if !defined(HAS_NO_INPUT_FORMATTER)
#include "input_formatter.h"
#endif

#include "dma.h"	/* N_DMA_CHANNEL_ID */

#include "ia_css_buffer.h"
#include "ia_css_binary.h"
#include "sh_css_hrt.h"
#include "sh_css_defs.h"
#include "sh_css_internal.h"
#include "ia_css_control.h"
#include "ia_css_debug.h"
#include "ia_css_debug_pipe.h"
#include "ia_css_event_public.h"
#include "ia_css_mmu.h"
#include "ia_css_stream.h"
#include "ia_css_isp_param.h"
#include "sh_css_params.h"
#include "sh_css_legacy.h"
#include "ia_css_frame_comm.h"
#if !defined(HAS_NO_INPUT_SYSTEM)
#include "ia_css_isys.h"
#endif

#include "gdc_device.h"				/* HRT_GDC_N */

/*#include "sp.h"*/	/* host2sp_enqueue_frame_data() */

#include "memory_access.h"

#include "assert_support.h"
#include "platform_support.h"	/* hrt_sleep() */

#include "sw_event_global.h"			/* Event IDs.*/
#include "ia_css_event.h"
#include "mmu_device.h"
#include "ia_css_spctrl.h"

#ifndef offsetof
#define offsetof(T, x) ((unsigned)&(((T *)0)->x))
#endif

#define IA_CSS_INCLUDE_CONFIGURATIONS
#include "ia_css_isp_configs.h"
#define IA_CSS_INCLUDE_STATES
#include "ia_css_isp_states.h"

#ifndef ISP2401
#include "isp/kernels/io_ls/bayer_io_ls/ia_css_bayer_io.host.h"
#else
#include "isp/kernels/ipu2_io_ls/bayer_io_ls/ia_css_bayer_io.host.h"
#endif

struct sh_css_sp_group		sh_css_sp_group;
struct sh_css_sp_stage		sh_css_sp_stage;
struct sh_css_isp_stage		sh_css_isp_stage;
struct sh_css_sp_output		sh_css_sp_output;
static struct sh_css_sp_per_frame_data per_frame_data;

/* true if SP supports frame loop and host2sp_commands */
/* For the moment there is only code that sets this bool to true */
/* TODO: add code that sets this bool to false */
static bool sp_running;

static enum ia_css_err
set_output_frame_buffer(const struct ia_css_frame *frame,
			unsigned idx);

static void
sh_css_copy_buffer_attr_to_spbuffer(struct ia_css_buffer_sp *dest_buf,
				const enum sh_css_queue_id queue_id,
				const hrt_vaddress xmem_addr,
				const enum ia_css_buffer_type buf_type);

static void
initialize_frame_buffer_attribute(struct ia_css_buffer_sp *buf_attr);

static void
initialize_stage_frames(struct ia_css_frames_sp *frames);

/* This data is stored every frame */
void
store_sp_group_data(void)
{
	per_frame_data.sp_group_addr = sh_css_store_sp_group_to_ddr();
}
{
	/* [WW07.5]type casting will cause potential issues */
	sh_css_sp_stage.num_stripes = (uint8_t) sh_css_isp_stage.binary_info.iterator.num_stripes;
	sh_css_sp_stage.row_stripes_height = (uint16_t) sh_css_isp_stage.binary_info.iterator.row_stripes_height;
	sh_css_sp_stage.row_stripes_overlap_lines = (uint16_t) sh_css_isp_stage.binary_info.iterator.row_stripes_overlap_lines;
	sh_css_sp_stage.top_cropping = (uint16_t) sh_css_isp_stage.binary_info.pipeline.top_cropping;
	/* moved to sh_css_sp_init_stage
	   sh_css_sp_stage.enable.vf_output =
	   sh_css_isp_stage.binary_info.enable.vf_veceven ||
	   sh_css_isp_stage.binary_info.num_output_pins > 1;
	*/
	sh_css_sp_stage.enable.sdis = sh_css_isp_stage.binary_info.enable.dis;
	sh_css_sp_stage.enable.s3a = sh_css_isp_stage.binary_info.enable.s3a;
#ifdef ISP2401	
	sh_css_sp_stage.enable.lace_stats = sh_css_isp_stage.binary_info.enable.lace_stats;
#endif	
}

void
store_sp_stage_data(enum ia_css_pipe_id id, unsigned int pipe_num, unsigned stage)
{
	unsigned int thread_id;
	ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);
	copy_isp_stage_to_sp_stage();
	if (id != IA_CSS_PIPE_ID_COPY)
		sh_css_sp_stage.isp_stage_addr =
			sh_css_store_isp_stage_to_ddr(pipe_num, stage);
	sh_css_sp_group.pipe[thread_id].sp_stage_addr[stage] =
		sh_css_store_sp_stage_to_ddr(pipe_num, stage);

	/* Clear for next frame */
	sh_css_sp_stage.program_input_circuit = false;
}

static void
store_sp_per_frame_data(const struct ia_css_fw_info *fw)
{
	unsigned int HIVE_ADDR_sp_per_frame_data = 0;

	assert(fw != NULL);

	switch (fw->type) {
	case ia_css_sp_firmware:
		HIVE_ADDR_sp_per_frame_data = fw->info.sp.per_frame_data;
		break;
	case ia_css_acc_firmware:
		HIVE_ADDR_sp_per_frame_data = fw->info.acc.per_frame_data;
		break;
	case ia_css_isp_firmware:
		return;
	}
	for (i = 0; i < IA_CSS_BINARY_MAX_OUTPUT_PORTS; i++) {
		if (err == IA_CSS_SUCCESS && args->out_frame[i])
			err = set_output_frame_buffer(args->out_frame[i], i);
	}

	/* we don't pass this error back to the upper layer, so we add a assert here
	   because we actually hit the error here but it still works by accident... */
	if (err != IA_CSS_SUCCESS) assert(false);
	return err;
}

static void
sh_css_sp_init_group(bool two_ppc,
		     enum ia_css_stream_format input_format,
		     bool no_isp_sync,
		     uint8_t if_config_index)
{
#if !defined(HAS_NO_INPUT_FORMATTER)
	sh_css_sp_group.config.input_formatter.isp_2ppc = two_ppc;
#else
	(void)two_ppc;
#endif

	sh_css_sp_group.config.no_isp_sync = (uint8_t)no_isp_sync;
	/* decide whether the frame is processed online or offline */
	if (if_config_index == SH_CSS_IF_CONFIG_NOT_NEEDED) return;
#if !defined(HAS_NO_INPUT_FORMATTER)
	assert(if_config_index < SH_CSS_MAX_IF_CONFIGS);
	sh_css_sp_group.config.input_formatter.set[if_config_index].stream_format = input_format;
#else
	(void)input_format;
#endif
}
{
	assert(stage != NULL);
	return stage->sp_func != IA_CSS_PIPELINE_NO_FUNC;
}

static enum ia_css_err
configure_isp_from_args(
	const struct sh_css_sp_pipeline *pipeline,
	const struct ia_css_binary      *binary,
	const struct sh_css_binary_args *args,
	bool two_ppc,
	bool deinterleaved)
{
	enum ia_css_err err = IA_CSS_SUCCESS;
#ifdef ISP2401
	struct ia_css_pipe *pipe = find_pipe_by_num(pipeline->pipe_num);
	const struct ia_css_resolution *res;

#endif
	ia_css_fpn_configure(binary,  &binary->in_frame_info);
	ia_css_crop_configure(binary, &args->delay_frames[0]->info);
	ia_css_qplane_configure(pipeline, binary, &binary->in_frame_info);
	ia_css_output0_configure(binary, &args->out_frame[0]->info);
	ia_css_output1_configure(binary, &args->out_vf_frame->info);
	ia_css_copy_output_configure(binary, args->copy_output);
	ia_css_output0_configure(binary, &args->out_frame[0]->info);
#ifdef ISP2401
	ia_css_sc_configure(binary, pipeline->shading.internal_frame_origin_x_bqs_on_sctbl,
				    pipeline->shading.internal_frame_origin_y_bqs_on_sctbl);
#endif
	ia_css_iterator_configure(binary, &args->in_frame->info);
	ia_css_dvs_configure(binary, &args->out_frame[0]->info);
	ia_css_output_configure(binary, &args->out_frame[0]->info);
	ia_css_raw_configure(pipeline, binary, &args->in_frame->info, &binary->in_frame_info, two_ppc, deinterleaved);
	ia_css_ref_configure(binary, (const struct ia_css_frame **)args->delay_frames, pipeline->dvs_frame_delay);
	ia_css_tnr_configure(binary, (const struct ia_css_frame **)args->tnr_frames);
	ia_css_bayer_io_config(binary, args);
	return err;
}
{
	enum ia_css_err err = IA_CSS_SUCCESS;
#ifdef ISP2401
	struct ia_css_pipe *pipe = find_pipe_by_num(pipeline->pipe_num);
	const struct ia_css_resolution *res;

#endif
	ia_css_fpn_configure(binary,  &binary->in_frame_info);
	ia_css_crop_configure(binary, &args->delay_frames[0]->info);
	ia_css_qplane_configure(pipeline, binary, &binary->in_frame_info);
	ia_css_output0_configure(binary, &args->out_frame[0]->info);
	ia_css_output1_configure(binary, &args->out_vf_frame->info);
	ia_css_copy_output_configure(binary, args->copy_output);
	ia_css_output0_configure(binary, &args->out_frame[0]->info);
#ifdef ISP2401
	ia_css_sc_configure(binary, pipeline->shading.internal_frame_origin_x_bqs_on_sctbl,
				    pipeline->shading.internal_frame_origin_y_bqs_on_sctbl);
#endif
	ia_css_iterator_configure(binary, &args->in_frame->info);
	ia_css_dvs_configure(binary, &args->out_frame[0]->info);
	ia_css_output_configure(binary, &args->out_frame[0]->info);
	ia_css_raw_configure(pipeline, binary, &args->in_frame->info, &binary->in_frame_info, two_ppc, deinterleaved);
	ia_css_ref_configure(binary, (const struct ia_css_frame **)args->delay_frames, pipeline->dvs_frame_delay);
	ia_css_tnr_configure(binary, (const struct ia_css_frame **)args->tnr_frames);
	ia_css_bayer_io_config(binary, args);
	return err;
}

static void
initialize_isp_states(const struct ia_css_binary *binary)
{
	unsigned int i;

	if (!binary->info->mem_offsets.offsets.state)
		return;
	for (i = 0; i < IA_CSS_NUM_STATE_IDS; i++) {
		ia_css_kernel_init_state[i](binary);
	}
	} else if (firmware) {
		const struct ia_css_frame_info *out_infos[IA_CSS_BINARY_MAX_OUTPUT_PORTS] = {NULL};
		if (args->out_frame[0])
			out_infos[0] = &args->out_frame[0]->info;
		info = &firmware->info.isp;
		ia_css_binary_fill_info(info, false, false,
			    IA_CSS_STREAM_FORMAT_RAW_10,
			    args->in_frame  ? &args->in_frame->info  : NULL,
			    NULL,
				out_infos,
			    args->out_vf_frame ? &args->out_vf_frame->info
						: NULL,
			    &tmp_binary,
			    NULL,
			    -1, true);
		binary = &tmp_binary;
		binary->info = info;
		binary_name = IA_CSS_EXT_ISP_PROG_NAME(firmware);
		blob_info = &firmware->blob;
		mem_if = (struct ia_css_isp_param_css_segments *)&firmware->mem_initializers;
	} else {
	    /* SP stage */
	    assert(stage->sp_func != IA_CSS_PIPELINE_NO_FUNC);
		/* binary and blob_info are now NULL.
		   These will be passed to sh_css_sp_init_stage
		   and dereferenced there, so passing a NULL
		   pointer is no good. return an error */
		return IA_CSS_ERR_INTERNAL_ERROR;
	}
	switch (stage->sp_func) {
	case IA_CSS_PIPELINE_RAW_COPY:
		sh_css_sp_start_raw_copy(args->out_frame[0],
				pipe_num, two_ppc,
				stage->max_input_width,
				copy_ovrd, if_config_index);
		break;
	case IA_CSS_PIPELINE_BIN_COPY:
		assert(false); /* TBI */
	case IA_CSS_PIPELINE_ISYS_COPY:
		sh_css_sp_start_isys_copy(args->out_frame[0],
				pipe_num, stage->max_input_width, if_config_index);
		break;
	case IA_CSS_PIPELINE_NO_FUNC:
		assert(false);
	}
}

void
sh_css_sp_init_pipeline(struct ia_css_pipeline *me,
			enum ia_css_pipe_id id,
			uint8_t pipe_num,
			bool xnr,
			bool two_ppc,
			bool continuous,
			bool offline,
			unsigned int required_bds_factor,
			enum sh_css_pipe_config_override copy_ovrd,
			enum ia_css_input_mode input_mode,
			const struct ia_css_metadata_config *md_config,
			const struct ia_css_metadata_info *md_info,
#if !defined(HAS_NO_INPUT_SYSTEM)
			const mipi_port_ID_t port_id
#endif
#ifdef ISP2401
			,
			const struct ia_css_coordinate *internal_frame_origin_bqs_on_sctbl, /* Origin of internal frame
							positioned on shading table at shading correction in ISP. */
			const struct ia_css_isp_parameters *params
#endif
	)
{
	/* Get first stage */
	struct ia_css_pipeline_stage *stage        = NULL;
	struct ia_css_binary	     *first_binary = NULL;
	struct ia_css_pipe *pipe = NULL;
	unsigned num;

	enum ia_css_pipe_id pipe_id = id;
	unsigned int thread_id;
	uint8_t if_config_index, tmp_if_config_index;

	assert(me != NULL);

#if !defined(HAS_NO_INPUT_SYSTEM)
	assert(me->stages != NULL);

	first_binary = me->stages->binary;

	if (input_mode == IA_CSS_INPUT_MODE_SENSOR ||
	    input_mode == IA_CSS_INPUT_MODE_BUFFERED_SENSOR) {
		assert(port_id < N_MIPI_PORT_ID);
		if (port_id >= N_MIPI_PORT_ID) /* should not happen but KW does not know */
			return; /* we should be able to return an error */
		if_config_index  = (uint8_t) (port_id - MIPI_PORT0_ID);
	} else if (input_mode == IA_CSS_INPUT_MODE_MEMORY) {
		if_config_index = SH_CSS_IF_CONFIG_NOT_NEEDED;
	} else {
		if_config_index = 0x0;
	}
#else
	(void)input_mode;
	if_config_index = SH_CSS_IF_CONFIG_NOT_NEEDED;
#endif

	ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);
	memset(&sh_css_sp_group.pipe[thread_id], 0, sizeof(struct sh_css_sp_pipeline));

	/* Count stages */
	for (stage = me->stages, num = 0; stage; stage = stage->next, num++) {
		stage->stage_num = num;
		ia_css_debug_pipe_graph_dump_stage(stage, id);
	}
	me->num_stages = num;

	if (first_binary != NULL) {
		/* Init pipeline data */
		sh_css_sp_init_group(two_ppc, first_binary->input_format,
				     offline, if_config_index);
	} /* if (first_binary != NULL) */

#if defined(USE_INPUT_SYSTEM_VERSION_2401) || defined(USE_INPUT_SYSTEM_VERSION_2)
	/* Signal the host immediately after start for SP_ISYS_COPY only */
	if ((me->num_stages == 1) && me->stages &&
	    (me->stages->sp_func == IA_CSS_PIPELINE_ISYS_COPY))
		sh_css_sp_group.config.no_isp_sync = true;
#endif

	/* Init stage data */
	sh_css_init_host2sp_frame_data();

	sh_css_sp_group.pipe[thread_id].num_stages = 0;
	sh_css_sp_group.pipe[thread_id].pipe_id = pipe_id;
	sh_css_sp_group.pipe[thread_id].thread_id = thread_id;
	sh_css_sp_group.pipe[thread_id].pipe_num = pipe_num;
	sh_css_sp_group.pipe[thread_id].num_execs = me->num_execs;
	sh_css_sp_group.pipe[thread_id].pipe_qos_config = me->pipe_qos_config;
	sh_css_sp_group.pipe[thread_id].required_bds_factor = required_bds_factor;
#if !defined(HAS_NO_INPUT_SYSTEM)
	sh_css_sp_group.pipe[thread_id].input_system_mode
						= (uint32_t)input_mode;
	sh_css_sp_group.pipe[thread_id].port_id = port_id;
#endif
	sh_css_sp_group.pipe[thread_id].dvs_frame_delay = (uint32_t)me->dvs_frame_delay;

	/* TODO: next indicates from which queues parameters need to be
		 sampled, needs checking/improvement */
	if (ia_css_pipeline_uses_params(me)) {
		sh_css_sp_group.pipe[thread_id].pipe_config =
			SH_CSS_PIPE_CONFIG_SAMPLE_PARAMS << thread_id;
	}

	/* For continuous use-cases, SP copy is responsible for sampling the
	 * parameters */
	if (continuous)
		sh_css_sp_group.pipe[thread_id].pipe_config = 0;

	sh_css_sp_group.pipe[thread_id].inout_port_config = me->inout_port_config;

	pipe = find_pipe_by_num(pipe_num);
	assert(pipe != NULL);
	if (pipe == NULL) {
		return;
	}
	sh_css_sp_group.pipe[thread_id].scaler_pp_lut = sh_css_pipe_get_pp_gdc_lut(pipe);

#if defined(SH_CSS_ENABLE_METADATA)
	if (md_info != NULL && md_info->size > 0) {
		sh_css_sp_group.pipe[thread_id].metadata.width  = md_info->resolution.width;
		sh_css_sp_group.pipe[thread_id].metadata.height = md_info->resolution.height;
		sh_css_sp_group.pipe[thread_id].metadata.stride = md_info->stride;
		sh_css_sp_group.pipe[thread_id].metadata.size   = md_info->size;
		ia_css_isys_convert_stream_format_to_mipi_format(
				md_config->data_type, MIPI_PREDICTOR_NONE,
				&sh_css_sp_group.pipe[thread_id].metadata.format);
	}
#else
	(void)md_config;
	(void)md_info;
#endif

#if defined(SH_CSS_ENABLE_PER_FRAME_PARAMS)
	sh_css_sp_group.pipe[thread_id].output_frame_queue_id = (uint32_t)SH_CSS_INVALID_QUEUE_ID;
	if (IA_CSS_PIPE_ID_COPY != pipe_id) {
		ia_css_query_internal_queue_id(IA_CSS_BUFFER_TYPE_OUTPUT_FRAME, thread_id, (enum sh_css_queue_id *)(&sh_css_sp_group.pipe[thread_id].output_frame_queue_id));
	}
#endif

#ifdef ISP2401
	/* For the shading correction type 1 (the legacy shading table conversion in css is not used),
	 * the parameters are passed to the isp for the shading table centering.
	 */
	if (internal_frame_origin_bqs_on_sctbl != NULL &&
			params != NULL && params->shading_settings.enable_shading_table_conversion == 0) {
		sh_css_sp_group.pipe[thread_id].shading.internal_frame_origin_x_bqs_on_sctbl
								= (uint32_t)internal_frame_origin_bqs_on_sctbl->x;
		sh_css_sp_group.pipe[thread_id].shading.internal_frame_origin_y_bqs_on_sctbl
								= (uint32_t)internal_frame_origin_bqs_on_sctbl->y;
	} else {
		sh_css_sp_group.pipe[thread_id].shading.internal_frame_origin_x_bqs_on_sctbl = 0;
		sh_css_sp_group.pipe[thread_id].shading.internal_frame_origin_y_bqs_on_sctbl = 0;
	}

#endif
	IA_CSS_LOG("pipe_id %d port_config %08x",
		   pipe_id, sh_css_sp_group.pipe[thread_id].inout_port_config);

	for (stage = me->stages, num = 0; stage; stage = stage->next, num++) {
		sh_css_sp_group.pipe[thread_id].num_stages++;
		if (is_sp_stage(stage)) {
			sp_init_sp_stage(stage, pipe_num, two_ppc,
				copy_ovrd, if_config_index);
		} else {
			if ((stage->stage_num != 0) || SH_CSS_PIPE_PORT_CONFIG_IS_CONTINUOUS(me->inout_port_config))
				tmp_if_config_index = SH_CSS_IF_CONFIG_NOT_NEEDED;
			else
				tmp_if_config_index = if_config_index;
			sp_init_stage(stage, pipe_num,
				      xnr, tmp_if_config_index, two_ppc);
		}

		store_sp_stage_data(pipe_id, pipe_num, num);
	}
	sh_css_sp_group.pipe[thread_id].pipe_config |= (uint32_t)
		(me->acquire_isp_each_stage << IA_CSS_ACQUIRE_ISP_POS);
	store_sp_group_data();

}

void
sh_css_sp_uninit_pipeline(unsigned int pipe_num)
{
	unsigned int thread_id;
	ia_css_pipeline_get_sp_thread_id(pipe_num, &thread_id);
	/*memset(&sh_css_sp_group.pipe[thread_id], 0, sizeof(struct sh_css_sp_pipeline));*/
	sh_css_sp_group.pipe[thread_id].num_stages = 0;
}

bool sh_css_write_host2sp_command(enum host2sp_commands host2sp_command)
{
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_command)
				/ sizeof(int);
	enum host2sp_commands last_cmd = host2sp_cmd_error;
	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Previous command must be handled by SP (by design) */
	last_cmd = load_sp_array_uint(host_sp_com, offset);
	if (last_cmd != host2sp_cmd_ready)
		IA_CSS_ERROR("last host command not handled by SP(%d)", last_cmd);

	store_sp_array_uint(host_sp_com, offset, host2sp_command);

	return (last_cmd == host2sp_cmd_ready);
}

enum host2sp_commands
sh_css_read_host2sp_command(void)
{
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_command)
				/ sizeof(int);
	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */
	return (enum host2sp_commands)load_sp_array_uint(host_sp_com, offset);
}


/*
 * Frame data is no longer part of the sp_stage structure but part of a
 * seperate structure. The aim is to make the sp_data struct static
 * (it defines a pipeline) and that the dynamic (per frame) data is stored
 * separetly.
 *
 * This function must be called first every where were you start constructing
 * a new pipeline by defining one or more stages with use of variable
 * sh_css_sp_stage. Even the special cases like accelerator and copy_frame
 * These have a pipeline of just 1 stage.
 */
void
sh_css_init_host2sp_frame_data(void)
{
	/* Clean table */
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */
	/*
	 * rvanimme: don't clean it to save static frame info line ref_in
	 * ref_out, and tnr_frames. Once this static data is in a
	 * seperate data struct, this may be enable (but still, there is
	 * no need for it)
	 */
}


/*
 * @brief Update the offline frame information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_offline_frame(
				unsigned frame_num,
				struct ia_css_frame *frame,
				struct ia_css_metadata *metadata)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	assert(frame_num < NUM_CONTINUOUS_FRAMES);

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_offline_frames)
		/ sizeof(int);
	offset += frame_num;
	store_sp_array_uint(host_sp_com, offset, frame ? frame->data : 0);

	/* Write metadata buffer into SP DMEM */
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_offline_metadata)
		/ sizeof(int);
	offset += frame_num;
	store_sp_array_uint(host_sp_com, offset, metadata ? metadata->address : 0);
}

#if defined(USE_INPUT_SYSTEM_VERSION_2) || defined(USE_INPUT_SYSTEM_VERSION_2401)
/*
 * @brief Update the mipi frame information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_mipi_frame(
				unsigned frame_num,
				struct ia_css_frame *frame)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_mipi_frames)
		/ sizeof(int);
	offset += frame_num;

	store_sp_array_uint(host_sp_com, offset,
				frame ? frame->data : 0);
}

/*
 * @brief Update the mipi metadata information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_mipi_metadata(
				unsigned frame_num,
				struct ia_css_metadata *metadata)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int o;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	o = offsetof(struct host_sp_communication, host2sp_mipi_metadata)
		/ sizeof(int);
	o += frame_num;
	store_sp_array_uint(host_sp_com, o,
				metadata ? metadata->address : 0);
}

void
sh_css_update_host2sp_num_mipi_frames(unsigned num_frames)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
#endif

void
sh_css_update_host2sp_cont_num_raw_frames(unsigned num_frames, bool set_avail)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}

void
sh_css_event_init_irq_mask(void)
{
	int i;
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int offset;
	struct sh_css_event_irq_mask event_irq_mask_init;

	event_irq_mask_init.or_mask  = IA_CSS_EVENT_TYPE_ALL;
	event_irq_mask_init.and_mask = IA_CSS_EVENT_TYPE_NONE;
	(void)HIVE_ADDR_host_sp_com; /* Suppress warnings in CRUN */

	assert(sizeof(event_irq_mask_init) % HRT_BUS_BYTES == 0);
	for (i = 0; i < IA_CSS_PIPE_ID_NUM; i++) {
		offset = (unsigned int)offsetof(struct host_sp_communication,
						host2sp_event_irq_mask[i]);
		assert(offset % HRT_BUS_BYTES == 0);
		sp_dmem_store(SP0_ID,
			(unsigned int)sp_address_of(host_sp_com) + offset,
			&event_irq_mask_init, sizeof(event_irq_mask_init));
	}

}

enum ia_css_err
ia_css_pipe_set_irq_mask(struct ia_css_pipe *pipe,
			 unsigned int or_mask,
			 unsigned int and_mask)
{
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int offset;
	struct sh_css_event_irq_mask event_irq_mask;
	unsigned int pipe_num;

	assert(pipe != NULL);

	assert(IA_CSS_PIPE_ID_NUM == NR_OF_PIPELINES);
	/* Linux kernel does not have UINT16_MAX
	 * Therefore decided to comment out these 2 asserts for Linux
	 * Alternatives that were not chosen:
	 * - add a conditional #define for UINT16_MAX
	 * - compare with (uint16_t)~0 or 0xffff
	 * - different assert for Linux and Windows
	 */
#ifndef __KERNEL__
	assert(or_mask <= UINT16_MAX);
	assert(and_mask <= UINT16_MAX);
#endif

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	IA_CSS_LOG("or_mask=%x, and_mask=%x", or_mask, and_mask);
	event_irq_mask.or_mask  = (uint16_t)or_mask;
	event_irq_mask.and_mask = (uint16_t)and_mask;

	pipe_num = ia_css_pipe_get_pipe_num(pipe);
	if (pipe_num >= IA_CSS_PIPE_ID_NUM)
		return IA_CSS_ERR_INTERNAL_ERROR;
	offset = (unsigned int)offsetof(struct host_sp_communication,
					host2sp_event_irq_mask[pipe_num]);
	assert(offset % HRT_BUS_BYTES == 0);
	sp_dmem_store(SP0_ID,
		(unsigned int)sp_address_of(host_sp_com) + offset,
		&event_irq_mask, sizeof(event_irq_mask));

	return IA_CSS_SUCCESS;
}

enum ia_css_err
ia_css_event_get_irq_mask(const struct ia_css_pipe *pipe,
			  unsigned int *or_mask,
			  unsigned int *and_mask)
{
	unsigned int HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	unsigned int offset;
	struct sh_css_event_irq_mask event_irq_mask;
	unsigned int pipe_num;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	IA_CSS_ENTER_LEAVE("");

	assert(pipe != NULL);
	assert(IA_CSS_PIPE_ID_NUM == NR_OF_PIPELINES);

	pipe_num = ia_css_pipe_get_pipe_num(pipe);
	if (pipe_num >= IA_CSS_PIPE_ID_NUM)
		return IA_CSS_ERR_INTERNAL_ERROR;
	offset = (unsigned int)offsetof(struct host_sp_communication,
					host2sp_event_irq_mask[pipe_num]);
	assert(offset % HRT_BUS_BYTES == 0);
	sp_dmem_load(SP0_ID,
		(unsigned int)sp_address_of(host_sp_com) + offset,
		&event_irq_mask, sizeof(event_irq_mask));

	if (or_mask)
		*or_mask = event_irq_mask.or_mask;

	if (and_mask)
		*and_mask = event_irq_mask.and_mask;

	return IA_CSS_SUCCESS;
}

void
sh_css_sp_set_sp_running(bool flag)
{
	sp_running = flag;
}

bool
sh_css_sp_is_running(void)
{
	return sp_running;
}

void
sh_css_sp_start_isp(void)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_sp_sw_state;

	fw = &sh_css_sp_fw;
	HIVE_ADDR_sp_sw_state = fw->info.sp.sw_state;


	if (sp_running)
		return;

	(void)HIVE_ADDR_sp_sw_state; /* Suppres warnings in CRUN */

	/* no longer here, sp started immediately */
	/*ia_css_debug_pipe_graph_dump_epilogue();*/

	store_sp_group_data();
	store_sp_per_frame_data(fw);

	sp_dmem_store_uint32(SP0_ID,
		(unsigned int)sp_address_of(sp_sw_state),
		(uint32_t)(IA_CSS_SP_SW_TERMINATED));


	/* Note 1: The sp_start_isp function contains a wait till
	 * the input network is configured by the SP.
	 * Note 2: Not all SP binaries supports host2sp_commands.
	 * In case a binary does support it, the host2sp_command
	 * will have status cmd_ready after return of the function
	 * sh_css_hrt_sp_start_isp. There is no race-condition here
	 * because only after the process_frame command has been
	 * received, the SP starts configuring the input network.
	 */

	/* we need to set sp_running before we call ia_css_mmu_invalidate_cache
	 * as ia_css_mmu_invalidate_cache checks on sp_running to
	 * avoid that it accesses dmem while the SP is not powered
	 */
	sp_running = true;
	ia_css_mmu_invalidate_cache();
	/* Invalidate all MMU caches */
	mmu_invalidate_cache_all();

	ia_css_spctrl_start(SP0_ID);

}

bool
ia_css_isp_has_started(void)
{
	const struct ia_css_fw_info *fw = &sh_css_sp_fw;
	unsigned int HIVE_ADDR_ia_css_ispctrl_sp_isp_started = fw->info.sp.isp_started;
	(void)HIVE_ADDR_ia_css_ispctrl_sp_isp_started; /* Suppres warnings in CRUN */

	return (bool)load_sp_uint(ia_css_ispctrl_sp_isp_started);
}


/*
 * @brief Initialize the DMA software-mask in the debug mode.
 * Refer to "sh_css_sp.h" for more details.
 */
bool
sh_css_sp_init_dma_sw_reg(int dma_id)
{
	int i;

	/* enable all the DMA channels */
	for (i = 0; i < N_DMA_CHANNEL_ID; i++) {
		/* enable the writing request */
		sh_css_sp_set_dma_sw_reg(dma_id,
				i,
				0,
				true);
		/* enable the reading request */
		sh_css_sp_set_dma_sw_reg(dma_id,
				i,
				1,
				true);
	}

	return true;
}

/*
 * @brief Set the DMA software-mask in the debug mode.
 * Refer to "sh_css_sp.h" for more details.
 */
bool
sh_css_sp_set_dma_sw_reg(int dma_id,
		int channel_id,
		int request_type,
		bool enable)
{
	uint32_t sw_reg;
	uint32_t bit_val;
	uint32_t bit_offset;
	uint32_t bit_mask;

	(void)dma_id;

	assert(channel_id >= 0 && channel_id < N_DMA_CHANNEL_ID);
	assert(request_type >= 0);

	/* get the software-mask */
	sw_reg =
		sh_css_sp_group.debug.dma_sw_reg;

	/* get the offest of the target bit */
	bit_offset = (8 * request_type) + channel_id;

	/* clear the value of the target bit */
	bit_mask = ~(1 << bit_offset);
	sw_reg &= bit_mask;

	/* set the value of the bit for the DMA channel */
	bit_val = enable ? 1 : 0;
	bit_val <<= bit_offset;
	sw_reg |= bit_val;

	/* update the software status of DMA channels */
	sh_css_sp_group.debug.dma_sw_reg = sw_reg;

	return true;
}

void
sh_css_sp_reset_global_vars(void)
{
	memset(&sh_css_sp_group, 0, sizeof(struct sh_css_sp_group));
	memset(&sh_css_sp_stage, 0, sizeof(struct sh_css_sp_stage));
	memset(&sh_css_isp_stage, 0, sizeof(struct sh_css_isp_stage));
	memset(&sh_css_sp_output, 0, sizeof(struct sh_css_sp_output));
	memset(&per_frame_data, 0, sizeof(struct sh_css_sp_per_frame_data));
}
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	assert(frame_num < NUM_CONTINUOUS_FRAMES);

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_offline_frames)
		/ sizeof(int);
	offset += frame_num;
	store_sp_array_uint(host_sp_com, offset, frame ? frame->data : 0);

	/* Write metadata buffer into SP DMEM */
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_offline_metadata)
		/ sizeof(int);
	offset += frame_num;
	store_sp_array_uint(host_sp_com, offset, metadata ? metadata->address : 0);
}

#if defined(USE_INPUT_SYSTEM_VERSION_2) || defined(USE_INPUT_SYSTEM_VERSION_2401)
/*
 * @brief Update the mipi frame information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_mipi_frame(
				unsigned frame_num,
				struct ia_css_frame *frame)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_mipi_frames)
		/ sizeof(int);
	offset += frame_num;

	store_sp_array_uint(host_sp_com, offset,
				frame ? frame->data : 0);
}

/*
 * @brief Update the mipi metadata information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_mipi_metadata(
				unsigned frame_num,
				struct ia_css_metadata *metadata)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int o;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	o = offsetof(struct host_sp_communication, host2sp_mipi_metadata)
		/ sizeof(int);
	o += frame_num;
	store_sp_array_uint(host_sp_com, o,
				metadata ? metadata->address : 0);
}

void
sh_css_update_host2sp_num_mipi_frames(unsigned num_frames)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
#endif

void
sh_css_update_host2sp_cont_num_raw_frames(unsigned num_frames, bool set_avail)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_mipi_frames)
		/ sizeof(int);
	offset += frame_num;

	store_sp_array_uint(host_sp_com, offset,
				frame ? frame->data : 0);
}

/*
 * @brief Update the mipi metadata information in host_sp_communication.
 * Refer to "sh_css_sp.h" for more details.
 */
void
sh_css_update_host2sp_mipi_metadata(
				unsigned frame_num,
				struct ia_css_metadata *metadata)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int o;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	o = offsetof(struct host_sp_communication, host2sp_mipi_metadata)
		/ sizeof(int);
	o += frame_num;
	store_sp_array_uint(host_sp_com, o,
				metadata ? metadata->address : 0);
}

void
sh_css_update_host2sp_num_mipi_frames(unsigned num_frames)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
#endif

void
sh_css_update_host2sp_cont_num_raw_frames(unsigned num_frames, bool set_avail)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int o;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* MIPI buffers are dedicated to port, so now there are more of them. */
	assert(frame_num < (N_CSI_PORTS * NUM_MIPI_FRAMES_PER_STREAM));

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	o = offsetof(struct host_sp_communication, host2sp_mipi_metadata)
		/ sizeof(int);
	o += frame_num;
	store_sp_array_uint(host_sp_com, o,
				metadata ? metadata->address : 0);
}

void
sh_css_update_host2sp_num_mipi_frames(unsigned num_frames)
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
#endif

void
sh_css_update_host2sp_cont_num_raw_frames(unsigned num_frames, bool set_avail)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
{
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int offset;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	HIVE_ADDR_host_sp_com = sh_css_sp_fw.info.sp.host_sp_com;
	offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_num_mipi_frames)
		/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
#endif

void
sh_css_update_host2sp_cont_num_raw_frames(unsigned num_frames, bool set_avail)
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}
{
	const struct ia_css_fw_info *fw;
	unsigned int HIVE_ADDR_host_sp_com;
	unsigned int extra_num_frames, avail_num_frames;
	unsigned int offset, offset_extra;

	(void)HIVE_ADDR_host_sp_com; /* Suppres warnings in CRUN */

	/* Write new frame data into SP DMEM */
	fw = &sh_css_sp_fw;
	HIVE_ADDR_host_sp_com = fw->info.sp.host_sp_com;
	if (set_avail) {
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_avail_num_raw_frames)
			/ sizeof(int);
		avail_num_frames = load_sp_array_uint(host_sp_com, offset);
		extra_num_frames = num_frames - avail_num_frames;
		offset_extra = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_extra_num_raw_frames)
			/ sizeof(int);
		store_sp_array_uint(host_sp_com, offset_extra, extra_num_frames);
	} else
		offset = (unsigned int)offsetof(struct host_sp_communication, host2sp_cont_target_num_raw_frames)
			/ sizeof(int);

	store_sp_array_uint(host_sp_com, offset, num_frames);
}