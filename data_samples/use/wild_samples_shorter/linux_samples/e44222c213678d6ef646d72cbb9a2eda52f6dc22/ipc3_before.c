		} else {
			if (sof_debug_check_flag(SOF_DBG_PRINT_IPC_SUCCESS_LOGS))
				ipc3_log_header(sdev->dev, "ipc tx succeeded", hdr->cmd);
			if (msg->reply_size)
				/* copy the data returned from DSP */
				memcpy(reply_data, msg->reply_data,
				       msg->reply_size);
		}