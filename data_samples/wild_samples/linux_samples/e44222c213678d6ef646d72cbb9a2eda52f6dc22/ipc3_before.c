		} else {
			if (sof_debug_check_flag(SOF_DBG_PRINT_IPC_SUCCESS_LOGS))
				ipc3_log_header(sdev->dev, "ipc tx succeeded", hdr->cmd);
			if (msg->reply_size)
				/* copy the data returned from DSP */
				memcpy(reply_data, msg->reply_data,
				       msg->reply_size);
		}

		/* re-enable dumps after successful IPC tx */
		if (sdev->ipc_dump_printed) {
			sdev->dbg_dump_printed = false;
			sdev->ipc_dump_printed = false;
		}
	}

	return ret;
}

/* send IPC message from host to DSP */
static int ipc3_tx_msg_unlocked(struct snd_sof_ipc *ipc,
				void *msg_data, size_t msg_bytes,
				void *reply_data, size_t reply_bytes)
{
	struct sof_ipc_cmd_hdr *hdr = msg_data;
	struct snd_sof_dev *sdev = ipc->sdev;
	int ret;

	ipc3_log_header(sdev->dev, "ipc tx", hdr->cmd);

	ret = sof_ipc_send_msg(sdev, msg_data, msg_bytes, reply_bytes);

	if (ret) {
		dev_err_ratelimited(sdev->dev,
				    "%s: ipc message send for %#x failed: %d\n",
				    __func__, hdr->cmd, ret);
		return ret;
	}

	/* now wait for completion */
	return ipc3_wait_tx_done(ipc, reply_data);
}

static int sof_ipc3_tx_msg(struct snd_sof_dev *sdev, void *msg_data, size_t msg_bytes,
			   void *reply_data, size_t reply_bytes, bool no_pm)
{
	struct snd_sof_ipc *ipc = sdev->ipc;
	int ret;

	if (!msg_data || msg_bytes < sizeof(struct sof_ipc_cmd_hdr)) {
		dev_err_ratelimited(sdev->dev, "No IPC message to send\n");
		return -EINVAL;
	}

	if (!no_pm) {
		const struct sof_dsp_power_state target_state = {
			.state = SOF_DSP_PM_D0,
		};

		/* ensure the DSP is in D0 before sending a new IPC */
		ret = snd_sof_dsp_set_power_state(sdev, &target_state);
		if (ret < 0) {
			dev_err(sdev->dev, "%s: resuming DSP failed: %d\n",
				__func__, ret);
			return ret;
		}