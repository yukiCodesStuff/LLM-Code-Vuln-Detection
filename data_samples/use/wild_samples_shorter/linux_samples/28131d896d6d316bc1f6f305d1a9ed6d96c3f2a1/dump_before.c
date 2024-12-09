		/* reset the device */
		iwl_trans_sw_reset(trans);

		err = iwl_finish_nic_init(trans, trans->trans_cfg);
		if (err)
			return;
	}

	for (i = 0; i < ARRAY_SIZE(table.sw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm SW status[%d]\n",
			table.sw_status[i], i);
}

static void iwl_fwrt_dump_iml_error_log(struct iwl_fw_runtime *fwrt)
{