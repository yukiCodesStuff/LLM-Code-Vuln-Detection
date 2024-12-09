/* Global variable to store the dtrace verbosity level */
unsigned int ia_css_debug_trace_level = IA_CSS_DEBUG_WARNING;

#define DPG_START "ia_css_debug_pipe_graph_dump_start "
#define DPG_END   " ia_css_debug_pipe_graph_dump_end\n"

#define ENABLE_LINE_MAX_LENGTH (25)
	int width;
	int eff_height;
	int eff_width;
	enum atomisp_input_format stream_format;
} pg_inst = {true, 0, 0, 0, 0, N_ATOMISP_INPUT_FORMAT};

static const char * const queue_id_to_str[] = {
	/* [SH_CSS_QUEUE_A_ID]     =*/ "queue_A",
	/* [SH_CSS_QUEUE_B_ID]     =*/ "queue_B",
	return ia_css_debug_trace_level;
}

static const char *debug_stream_format2str(const enum atomisp_input_format stream_format)
{
	switch (stream_format) {
	case ATOMISP_INPUT_FORMAT_YUV420_8_LEGACY:
		return "yuv420-8-legacy";
	case ATOMISP_INPUT_FORMAT_YUV420_8:
		return "yuv420-8";
	case ATOMISP_INPUT_FORMAT_YUV420_10:
		return "yuv420-10";
	case ATOMISP_INPUT_FORMAT_YUV420_16:
		return "yuv420-16";
	case ATOMISP_INPUT_FORMAT_YUV422_8:
		return "yuv422-8";
	case ATOMISP_INPUT_FORMAT_YUV422_10:
		return "yuv422-10";
	case ATOMISP_INPUT_FORMAT_YUV422_16:
		return "yuv422-16";
	case ATOMISP_INPUT_FORMAT_RGB_444:
		return "rgb444";
	case ATOMISP_INPUT_FORMAT_RGB_555:
		return "rgb555";
	case ATOMISP_INPUT_FORMAT_RGB_565:
		return "rgb565";
	case ATOMISP_INPUT_FORMAT_RGB_666:
		return "rgb666";
	case ATOMISP_INPUT_FORMAT_RGB_888:
		return "rgb888";
	case ATOMISP_INPUT_FORMAT_RAW_6:
		return "raw6";
	case ATOMISP_INPUT_FORMAT_RAW_7:
		return "raw7";
	case ATOMISP_INPUT_FORMAT_RAW_8:
		return "raw8";
	case ATOMISP_INPUT_FORMAT_RAW_10:
		return "raw10";
	case ATOMISP_INPUT_FORMAT_RAW_12:
		return "raw12";
	case ATOMISP_INPUT_FORMAT_RAW_14:
		return "raw14";
	case ATOMISP_INPUT_FORMAT_RAW_16:
		return "raw16";
	case ATOMISP_INPUT_FORMAT_BINARY_8:
		return "binary8";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT1:
		return "generic-short1";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT2:
		return "generic-short2";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT3:
		return "generic-short3";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT4:
		return "generic-short4";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT5:
		return "generic-short5";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT6:
		return "generic-short6";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT7:
		return "generic-short7";
	case ATOMISP_INPUT_FORMAT_GENERIC_SHORT8:
		return "generic-short8";
	case ATOMISP_INPUT_FORMAT_YUV420_8_SHIFT:
		return "yuv420-8-shift";
	case ATOMISP_INPUT_FORMAT_YUV420_10_SHIFT:
		return "yuv420-10-shift";
	case ATOMISP_INPUT_FORMAT_EMBEDDED:
		return "embedded-8";
	case ATOMISP_INPUT_FORMAT_USER_DEF1:
		return "user-def-8-type-1";
	case ATOMISP_INPUT_FORMAT_USER_DEF2:
		return "user-def-8-type-2";
	case ATOMISP_INPUT_FORMAT_USER_DEF3:
		return "user-def-8-type-3";
	case ATOMISP_INPUT_FORMAT_USER_DEF4:
		return "user-def-8-type-4";
	case ATOMISP_INPUT_FORMAT_USER_DEF5:
		return "user-def-8-type-5";
	case ATOMISP_INPUT_FORMAT_USER_DEF6:
		return "user-def-8-type-6";
	case ATOMISP_INPUT_FORMAT_USER_DEF7:
		return "user-def-8-type-7";
	case ATOMISP_INPUT_FORMAT_USER_DEF8:
		return "user-def-8-type-8";

	default:
		assert(!"Unknown stream format");
	}
	dtrace_dot(
		"node [shape = box, "
		"fixedsize=true, width=2, height=0.7]; \"%p\" "
		"[label = \"%s\\n%d(%d) x %d, %dbpp\\n%s\"];",
		frame,
		debug_frame_format2str(frame->info.format),
		frame->info.res.width,
		frame->info.padded_width,
		frame->info.res.height,

	if (in_frame) {
		dtrace_dot(
			"\"%p\"->\"%s(pipe%d)\" "
			"[label = %s_frame];",
			frame,
			blob_name, id, frame_name);
	} else {
		dtrace_dot(
			"\"%s(pipe%d)\"->\"%p\" "
			"[label = %s_frame];",
			blob_name, id,
			frame,
			frame_name);
	}
}

	}


	if (pg_inst.stream_format != N_ATOMISP_INPUT_FORMAT) {
		/* An input stream format has been set so assume we have
		 * an input system and sensor
		 */

	pg_inst.height = 0;
	pg_inst.eff_width = 0;
	pg_inst.eff_height = 0;
	pg_inst.stream_format = N_ATOMISP_INPUT_FORMAT;
}

void
ia_css_debug_pipe_graph_dump_stage(

	snprintf(ring_buffer, sizeof(ring_buffer),
		"node [shape = box, "
		"fixedsize=true, width=2, height=0.7]; \"%p\" "
		"[label = \"%s\\n%d(%d) x %d\\nRingbuffer\"];",
		out_frame,
		debug_frame_format2str(out_frame->info.format),
		out_frame->info.res.width,
		out_frame->info.padded_width,
		out_frame->info.res.height);
	dtrace_dot(ring_buffer);

	dtrace_dot(
		"\"%s(pipe%d)\"->\"%p\" "
		"[label = out_frame];",
		"sp_raw_copy", 1, out_frame);

	snprintf(dot_id_input_bin, sizeof(dot_id_input_bin), "%s(pipe%d)", "sp_raw_copy", 1);
}
