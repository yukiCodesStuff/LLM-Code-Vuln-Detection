	if (buf) {
		int pos = 0;
		size_t bufsz = ARRAY_SIZE(fh_tbl) * 48 + 40;

		*buf = kmalloc(bufsz, GFP_KERNEL);
		if (!*buf)
			return -ENOMEM;

		pos += scnprintf(*buf + pos, bufsz - pos,
				"FH register values:\n");

		for (i = 0; i < ARRAY_SIZE(fh_tbl); i++)
			pos += scnprintf(*buf + pos, bufsz - pos,
				"  %34s: 0X%08x\n",
				get_fh_string(fh_tbl[i]),
				iwl_read_direct32(trans, fh_tbl[i]));

		return pos;
	}
#endif

	IWL_ERR(trans, "FH register values:\n");
	for (i = 0; i <  ARRAY_SIZE(fh_tbl); i++)
		IWL_ERR(trans, "  %34s: 0X%08x\n",
			get_fh_string(fh_tbl[i]),
			iwl_read_direct32(trans, fh_tbl[i]));

	return 0;
}

#define IWL_HOST_MON_BLOCK_PEMON	0x00
#define IWL_HOST_MON_BLOCK_HIPM		0x22

#define IWL_HOST_MON_BLOCK_PEMON_VEC0	0x00
#define IWL_HOST_MON_BLOCK_PEMON_VEC1	0x01
#define IWL_HOST_MON_BLOCK_PEMON_WFPM	0x06

static void iwl_dump_host_monitor_block(struct iwl_trans *trans,
					u32 block, u32 vec, u32 iter)
{
	int i;

	IWL_ERR(trans, "Host monitor block 0x%x vector 0x%x\n", block, vec);
	iwl_write32(trans, CSR_MONITOR_CFG_REG, (block << 8) | vec);
	for (i = 0; i < iter; i++)
		IWL_ERR(trans, "    value [iter %d]: 0x%08x\n",
			i, iwl_read32(trans, CSR_MONITOR_STATUS_REG));
}

static void iwl_dump_host_monitor(struct iwl_trans *trans)
{
	switch (trans->trans_cfg->device_family) {
	case IWL_DEVICE_FAMILY_22000:
	case IWL_DEVICE_FAMILY_AX210:
		IWL_ERR(trans, "CSR_RESET = 0x%x\n",
			iwl_read32(trans, CSR_RESET));
		iwl_dump_host_monitor_block(trans, IWL_HOST_MON_BLOCK_PEMON,
					    IWL_HOST_MON_BLOCK_PEMON_VEC0, 15);
		iwl_dump_host_monitor_block(trans, IWL_HOST_MON_BLOCK_PEMON,
					    IWL_HOST_MON_BLOCK_PEMON_VEC1, 15);
		iwl_dump_host_monitor_block(trans, IWL_HOST_MON_BLOCK_PEMON,
					    IWL_HOST_MON_BLOCK_PEMON_WFPM, 15);
		iwl_dump_host_monitor_block(trans, IWL_HOST_MON_BLOCK_HIPM,
					    IWL_HOST_MON_BLOCK_PEMON_VEC0, 1);
		break;
	default:
		/* not supported yet */
		return;
	}
	} else {
		iwl_set_bit(trans, CSR_GP_CNTRL,
			    CSR_GP_CNTRL_REG_FLAG_INIT_DONE);
		poll_ready = CSR_GP_CNTRL_REG_FLAG_MAC_CLOCK_READY;
	}

	if (cfg_trans->device_family == IWL_DEVICE_FAMILY_8000)
		udelay(2);

	/*
	 * Wait for clock stabilization; once stabilized, access to
	 * device-internal resources is supported, e.g. iwl_write_prph()
	 * and accesses to uCode SRAM.
	 */
	err = iwl_poll_bit(trans, CSR_GP_CNTRL, poll_ready, poll_ready, 25000);
	if (err < 0) {
		IWL_DEBUG_INFO(trans, "Failed to wake NIC\n");

		iwl_dump_host_monitor(trans);
	}