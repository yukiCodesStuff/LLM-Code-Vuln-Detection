			if (bitop_getbit(cur_rsrc->active_table, packet_entry) == 1) {
				bitop_clearbit(cur_rsrc->active_table, packet_entry);

				if (packet_type == CSI_MIPI_PACKET_TYPE_LONG)
					cur_rsrc->num_long_packets--;
				else
					cur_rsrc->num_short_packets--;
				cur_rsrc->num_active--;
			}
		}
	}
}

enum ia_css_err ia_css_isys_csi_rx_register_stream(
	enum mipi_port_id port,
	uint32_t isys_stream_id)
{
	enum ia_css_err retval = IA_CSS_ERR_INTERNAL_ERROR;

	if ((port < N_INPUT_SYSTEM_CSI_PORT) &&
	    (isys_stream_id < SH_CSS_MAX_ISYS_CHANNEL_NODES)) {
		struct sh_css_sp_pipeline_io_status *pipe_io_status;
		pipe_io_status = ia_css_pipeline_get_pipe_io_status();
		if (bitop_getbit(pipe_io_status->active[port], isys_stream_id) == 0) {
			bitop_setbit(pipe_io_status->active[port], isys_stream_id);
			pipe_io_status->running[port] = 0;
			retval = IA_CSS_SUCCESS;
		}
	}
	return retval;
}
		if (bitop_getbit(pipe_io_status->active[port], isys_stream_id) == 0) {
			bitop_setbit(pipe_io_status->active[port], isys_stream_id);
			pipe_io_status->running[port] = 0;
			retval = IA_CSS_SUCCESS;
		}
	}
	return retval;
}

enum ia_css_err ia_css_isys_csi_rx_unregister_stream(
	enum mipi_port_id port,
	uint32_t isys_stream_id)
{
	enum ia_css_err retval = IA_CSS_ERR_INTERNAL_ERROR;

	if ((port < N_INPUT_SYSTEM_CSI_PORT) &&
	    (isys_stream_id < SH_CSS_MAX_ISYS_CHANNEL_NODES)) {
		struct sh_css_sp_pipeline_io_status *pipe_io_status;
		pipe_io_status = ia_css_pipeline_get_pipe_io_status();
		if (bitop_getbit(pipe_io_status->active[port], isys_stream_id) == 1) {
			bitop_clearbit(pipe_io_status->active[port], isys_stream_id);
			retval = IA_CSS_SUCCESS;
		}
	}
	return retval;
}