		/* reset the device */
		iwl_trans_sw_reset(trans);

		err = iwl_finish_nic_init(trans);
		if (err)
			return;
	}

	for (i = 0; i < ARRAY_SIZE(table.sw_status); i++)
		IWL_ERR(fwrt, "0x%08X | tcm SW status[%d]\n",
			table.sw_status[i], i);

	if (trans->trans_cfg->device_family >= IWL_DEVICE_FAMILY_BZ) {
		u32 scratch = iwl_read32(trans, CSR_FUNC_SCRATCH);

		IWL_ERR(fwrt, "Function Scratch status:\n");
		IWL_ERR(fwrt, "0x%08X | Func Scratch\n", scratch);
	}
}

static void iwl_fwrt_dump_iml_error_log(struct iwl_fw_runtime *fwrt)
{