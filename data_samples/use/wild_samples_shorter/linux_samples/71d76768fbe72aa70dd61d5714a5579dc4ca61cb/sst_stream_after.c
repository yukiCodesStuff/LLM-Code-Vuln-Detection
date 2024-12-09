	u32 length;
	int pvt_id, ret = 0;
	struct sst_block *block = NULL;
	u8 bytes_block = bytes->block;

	dev_dbg(sst_drv_ctx->dev,
		"type:%u ipc_msg:%u block:%u task_id:%u pipe: %#x length:%#x\n",
		bytes->type, bytes->ipc_msg, bytes_block, bytes->task_id,
		bytes->pipe_id, bytes->len);

	if (sst_create_ipc_msg(&msg, true))
		return -ENOMEM;
	pvt_id = sst_assign_pvt_id(sst_drv_ctx);
	sst_fill_header_mrfld(&msg->mrfld_header, bytes->ipc_msg,
			bytes->task_id, 1, pvt_id);
	msg->mrfld_header.p.header_high.part.res_rqd = bytes_block;
	length = bytes->len;
	msg->mrfld_header.p.header_low_payload = length;
	dev_dbg(sst_drv_ctx->dev, "length is %d\n", length);
	memcpy(msg->mailbox_data, &bytes->bytes, bytes->len);
	if (bytes_block) {
		block = sst_create_block(sst_drv_ctx, bytes->ipc_msg, pvt_id);
		if (block == NULL) {
			kfree(msg);
			ret = -ENOMEM;
	dev_dbg(sst_drv_ctx->dev, "msg->mrfld_header.p.header_low_payload:%d",
			msg->mrfld_header.p.header_low_payload);

	if (bytes_block) {
		ret = sst_wait_timeout(sst_drv_ctx, block);
		if (ret) {
			dev_err(sst_drv_ctx->dev, "fw returned err %d\n", ret);
			sst_free_block(sst_drv_ctx, block);
		 * copy the reply and send back
		 * we need to update only sz and payload
		 */
		if (bytes_block) {
			unsigned char *r = block->data;

			dev_dbg(sst_drv_ctx->dev, "read back %d bytes",
					bytes->len);
			memcpy(bytes->bytes, r, bytes->len);
		}
	}
	if (bytes_block)
		sst_free_block(sst_drv_ctx, block);
out:
	test_and_clear_bit(pvt_id, &sst_drv_ctx->pvt_id);
	return ret;