/* Global variable to store the dtrace verbosity level */
unsigned int ia_css_debug_trace_level = IA_CSS_DEBUG_WARNING;

/* Assumes that IA_CSS_STREAM_FORMAT_BINARY_8 is last */
#define N_IA_CSS_STREAM_FORMAT (IA_CSS_STREAM_FORMAT_BINARY_8+1)

#define DPG_START "ia_css_debug_pipe_graph_dump_start "
#define DPG_END   " ia_css_debug_pipe_graph_dump_end\n"

#define ENABLE_LINE_MAX_LENGTH (25)
	int width;
	int eff_height;
	int eff_width;
	enum ia_css_stream_format stream_format;
} pg_inst = {true, 0, 0, 0, 0, N_IA_CSS_STREAM_FORMAT};

static const char * const queue_id_to_str[] = {
	/* [SH_CSS_QUEUE_A_ID]     =*/ "queue_A",
	/* [SH_CSS_QUEUE_B_ID]     =*/ "queue_B",
	return ia_css_debug_trace_level;
}

static const char *debug_stream_format2str(const enum ia_css_stream_format stream_format)
{
	switch (stream_format) {
	case IA_CSS_STREAM_FORMAT_YUV420_8_LEGACY:
		return "yuv420-8-legacy";
	case IA_CSS_STREAM_FORMAT_YUV420_8:
		return "yuv420-8";
	case IA_CSS_STREAM_FORMAT_YUV420_10:
		return "yuv420-10";
	case IA_CSS_STREAM_FORMAT_YUV420_16:
		return "yuv420-16";
	case IA_CSS_STREAM_FORMAT_YUV422_8:
		return "yuv422-8";
	case IA_CSS_STREAM_FORMAT_YUV422_10:
		return "yuv422-10";
	case IA_CSS_STREAM_FORMAT_YUV422_16:
		return "yuv422-16";
	case IA_CSS_STREAM_FORMAT_RGB_444:
		return "rgb444";
	case IA_CSS_STREAM_FORMAT_RGB_555:
		return "rgb555";
	case IA_CSS_STREAM_FORMAT_RGB_565:
		return "rgb565";
	case IA_CSS_STREAM_FORMAT_RGB_666:
		return "rgb666";
	case IA_CSS_STREAM_FORMAT_RGB_888:
		return "rgb888";
	case IA_CSS_STREAM_FORMAT_RAW_6:
		return "raw6";
	case IA_CSS_STREAM_FORMAT_RAW_7:
		return "raw7";
	case IA_CSS_STREAM_FORMAT_RAW_8:
		return "raw8";
	case IA_CSS_STREAM_FORMAT_RAW_10:
		return "raw10";
	case IA_CSS_STREAM_FORMAT_RAW_12:
		return "raw12";
	case IA_CSS_STREAM_FORMAT_RAW_14:
		return "raw14";
	case IA_CSS_STREAM_FORMAT_RAW_16:
		return "raw16";
	case IA_CSS_STREAM_FORMAT_BINARY_8:
		return "binary8";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT1:
		return "generic-short1";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT2:
		return "generic-short2";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT3:
		return "generic-short3";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT4:
		return "generic-short4";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT5:
		return "generic-short5";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT6:
		return "generic-short6";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT7:
		return "generic-short7";
	case IA_CSS_STREAM_FORMAT_GENERIC_SHORT8:
		return "generic-short8";
	case IA_CSS_STREAM_FORMAT_YUV420_8_SHIFT:
		return "yuv420-8-shift";
	case IA_CSS_STREAM_FORMAT_YUV420_10_SHIFT:
		return "yuv420-10-shift";
	case IA_CSS_STREAM_FORMAT_EMBEDDED:
		return "embedded-8";
	case IA_CSS_STREAM_FORMAT_USER_DEF1:
		return "user-def-8-type-1";
	case IA_CSS_STREAM_FORMAT_USER_DEF2:
		return "user-def-8-type-2";
	case IA_CSS_STREAM_FORMAT_USER_DEF3:
		return "user-def-8-type-3";
	case IA_CSS_STREAM_FORMAT_USER_DEF4:
		return "user-def-8-type-4";
	case IA_CSS_STREAM_FORMAT_USER_DEF5:
		return "user-def-8-type-5";
	case IA_CSS_STREAM_FORMAT_USER_DEF6:
		return "user-def-8-type-6";
	case IA_CSS_STREAM_FORMAT_USER_DEF7:
		return "user-def-8-type-7";
	case IA_CSS_STREAM_FORMAT_USER_DEF8:
		return "user-def-8-type-8";

	default:
		assert(!"Unknown stream format");
	}
	dtrace_dot(
		"node [shape = box, "
		"fixedsize=true, width=2, height=0.7]; \"0x%08lx\" "
		"[label = \"%s\\n%d(%d) x %d, %dbpp\\n%s\"];",
		HOST_ADDRESS(frame),
		debug_frame_format2str(frame->info.format),
		frame->info.res.width,
		frame->info.padded_width,
		frame->info.res.height,

	if (in_frame) {
		dtrace_dot(
			"\"0x%08lx\"->\"%s(pipe%d)\" "
			"[label = %s_frame];",
			HOST_ADDRESS(frame),
			blob_name, id, frame_name);
	} else {
		dtrace_dot(
			"\"%s(pipe%d)\"->\"0x%08lx\" "
			"[label = %s_frame];",
			blob_name, id,
			HOST_ADDRESS(frame),
			frame_name);
	}
}

	}


	if (pg_inst.stream_format != N_IA_CSS_STREAM_FORMAT) {
		/* An input stream format has been set so assume we have
		 * an input system and sensor
		 */

	pg_inst.height = 0;
	pg_inst.eff_width = 0;
	pg_inst.eff_height = 0;
	pg_inst.stream_format = N_IA_CSS_STREAM_FORMAT;
}

void
ia_css_debug_pipe_graph_dump_stage(

	snprintf(ring_buffer, sizeof(ring_buffer),
		"node [shape = box, "
		"fixedsize=true, width=2, height=0.7]; \"0x%08lx\" "
		"[label = \"%s\\n%d(%d) x %d\\nRingbuffer\"];",
		HOST_ADDRESS(out_frame),
		debug_frame_format2str(out_frame->info.format),
		out_frame->info.res.width,
		out_frame->info.padded_width,
		out_frame->info.res.height);
	dtrace_dot(ring_buffer);

	dtrace_dot(
		"\"%s(pipe%d)\"->\"0x%08lx\" "
		"[label = out_frame];",
		"sp_raw_copy", 1, HOST_ADDRESS(out_frame));

	snprintf(dot_id_input_bin, sizeof(dot_id_input_bin), "%s(pipe%d)", "sp_raw_copy", 1);
}
