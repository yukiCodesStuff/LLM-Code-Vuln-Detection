		break;
	}

	if (!rc)
		return false;

	if (!acquire_sid(me->stream2mmio_id, &(me->stream2mmio_sid_id))) {
		return false;

	rc = calculate_stream2mmio_cfg(isys_cfg, metadata,
			&(channel_cfg->stream2mmio_cfg));
	if (!rc)
		return false;

	rc = calculate_ibuf_ctrl_cfg(
			channel,
			input_port,
			isys_cfg,
			&(channel_cfg->ibuf_ctrl_cfg));
	if (!rc)
		return false;
	if (metadata)
		channel_cfg->ibuf_ctrl_cfg.stores_per_frame = isys_cfg->metadata.lines_per_frame;

			channel,
			isys_cfg,
			&(channel_cfg->dma_cfg));
	if (!rc)
		return false;

	rc = calculate_isys2401_dma_port_cfg(
			isys_cfg,
			false,
			metadata,
			&(channel_cfg->dma_src_port_cfg));
	if (!rc)
		return false;

	rc = calculate_isys2401_dma_port_cfg(
			isys_cfg,
			isys_cfg->raw_packed,
			metadata,
			&(channel_cfg->dma_dest_port_cfg));
	if (!rc)
		return false;

	return true;
}