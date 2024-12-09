#define FPNTBL_BYTES(binary) \
	(sizeof(char) * (binary)->in_frame_info.res.height * \
	 (binary)->in_frame_info.padded_width)
	 
#ifndef ISP2401

#define SCTBL_BYTES(binary) \
	(sizeof(unsigned short) * (binary)->sctbl_height * \
				out_infos[0] = &args->out_frame[0]->info;
			info = &stage->firmware->info.isp;
			ia_css_binary_fill_info(info, false, false,
				IA_CSS_STREAM_FORMAT_RAW_10,
				args->in_frame  ? &args->in_frame->info  : NULL,
				NULL,
				out_infos,
				args->out_vf_frame ? &args->out_vf_frame->info
	}
}

unsigned g_param_buffer_dequeue_count = 0;
unsigned g_param_buffer_enqueue_count = 0;

enum ia_css_err
ia_css_stream_isp_parameters_init(struct ia_css_stream *stream)
{

		enum sh_css_queue_id queue_id;

		(void)stage;
		pipe = curr_pipe->stream->pipes[i];
		pipeline = ia_css_pipe_get_pipeline(pipe);
		pipe_num = ia_css_pipe_get_pipe_num(pipe);
		isp_pipe_version = ia_css_pipe_get_isp_pipe_version(pipe);