{
	int retval = 0;
	struct stream_info *str_info;
	u16 data = 0;

	dev_dbg(sst_drv_ctx->dev, "sst_start_stream for %d\n", str_id);
	str_info = get_stream_info(sst_drv_ctx, str_id);
	if (!str_info)
		return -EINVAL;
	if (str_info->status != STREAM_RUNNING)
		return -EBADRQC;

	retval = sst_prepare_and_post_msg(sst_drv_ctx, str_info->task_id,
			IPC_CMD, IPC_IA_START_STREAM_MRFLD, str_info->pipe_id,
			sizeof(u16), &data, NULL, true, true, true, false);

	return retval;
}

int sst_send_byte_stream_mrfld(struct intel_sst_drv *sst_drv_ctx,
		struct snd_sst_bytes_v2 *bytes)
{	struct ipc_post *msg = NULL;
	u32 length;
	int pvt_id, ret = 0;
	struct sst_block *block = NULL;

	dev_dbg(sst_drv_ctx->dev,
		"type:%u ipc_msg:%u block:%u task_id:%u pipe: %#x length:%#x\n",
		bytes->type, bytes->ipc_msg, bytes->block, bytes->task_id,
		bytes->pipe_id, bytes->len);

	if (sst_create_ipc_msg(&msg, true))
		return -ENOMEM;

	pvt_id = sst_assign_pvt_id(sst_drv_ctx);
	sst_fill_header_mrfld(&msg->mrfld_header, bytes->ipc_msg,
			bytes->task_id, 1, pvt_id);
	msg->mrfld_header.p.header_high.part.res_rqd = bytes->block;
	length = bytes->len;
	msg->mrfld_header.p.header_low_payload = length;
	dev_dbg(sst_drv_ctx->dev, "length is %d\n", length);
	memcpy(msg->mailbox_data, &bytes->bytes, bytes->len);
	if (bytes->block) {
		block = sst_create_block(sst_drv_ctx, bytes->ipc_msg, pvt_id);
		if (block == NULL) {
			kfree(msg);
			ret = -ENOMEM;
			goto out;
		}
	}
{
	int retval = 0;
	struct stream_info *str_info;
	u16 data = 0;

	dev_dbg(sst_drv_ctx->dev, "sst_start_stream for %d\n", str_id);
	str_info = get_stream_info(sst_drv_ctx, str_id);
	if (!str_info)
		return -EINVAL;
	if (str_info->status != STREAM_RUNNING)
		return -EBADRQC;

	retval = sst_prepare_and_post_msg(sst_drv_ctx, str_info->task_id,
			IPC_CMD, IPC_IA_START_STREAM_MRFLD, str_info->pipe_id,
			sizeof(u16), &data, NULL, true, true, true, false);

	return retval;
}

int sst_send_byte_stream_mrfld(struct intel_sst_drv *sst_drv_ctx,
		struct snd_sst_bytes_v2 *bytes)
{	struct ipc_post *msg = NULL;
	u32 length;
	int pvt_id, ret = 0;
	struct sst_block *block = NULL;

	dev_dbg(sst_drv_ctx->dev,
		"type:%u ipc_msg:%u block:%u task_id:%u pipe: %#x length:%#x\n",
		bytes->type, bytes->ipc_msg, bytes->block, bytes->task_id,
		bytes->pipe_id, bytes->len);

	if (sst_create_ipc_msg(&msg, true))
		return -ENOMEM;

	pvt_id = sst_assign_pvt_id(sst_drv_ctx);
	sst_fill_header_mrfld(&msg->mrfld_header, bytes->ipc_msg,
			bytes->task_id, 1, pvt_id);
	msg->mrfld_header.p.header_high.part.res_rqd = bytes->block;
	length = bytes->len;
	msg->mrfld_header.p.header_low_payload = length;
	dev_dbg(sst_drv_ctx->dev, "length is %d\n", length);
	memcpy(msg->mailbox_data, &bytes->bytes, bytes->len);
	if (bytes->block) {
		block = sst_create_block(sst_drv_ctx, bytes->ipc_msg, pvt_id);
		if (block == NULL) {
			kfree(msg);
			ret = -ENOMEM;
			goto out;
		}
	}
		if (block == NULL) {
			kfree(msg);
			ret = -ENOMEM;
			goto out;
		}
	}

	sst_add_to_dispatch_list_and_post(sst_drv_ctx, msg);
	dev_dbg(sst_drv_ctx->dev, "msg->mrfld_header.p.header_low_payload:%d",
			msg->mrfld_header.p.header_low_payload);

	if (bytes->block) {
		ret = sst_wait_timeout(sst_drv_ctx, block);
		if (ret) {
			dev_err(sst_drv_ctx->dev, "fw returned err %d\n", ret);
			sst_free_block(sst_drv_ctx, block);
			goto out;
		}
	}
	if (bytes->type == SND_SST_BYTES_GET) {
		/*
		 * copy the reply and send back
		 * we need to update only sz and payload
		 */
		if (bytes->block) {
			unsigned char *r = block->data;

			dev_dbg(sst_drv_ctx->dev, "read back %d bytes",
					bytes->len);
			memcpy(bytes->bytes, r, bytes->len);
		}