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
}

int iwl_finish_nic_init(struct iwl_trans *trans)
{
	const struct iwl_cfg_trans_params *cfg_trans = trans->trans_cfg;
	u32 poll_ready;
	int err;

	if (cfg_trans->bisr_workaround) {
	 * and accesses to uCode SRAM.
	 */
	err = iwl_poll_bit(trans, CSR_GP_CNTRL, poll_ready, poll_ready, 25000);
	if (err < 0) {
		IWL_DEBUG_INFO(trans, "Failed to wake NIC\n");

		iwl_dump_host_monitor(trans);
	}

	if (cfg_trans->bisr_workaround) {
		/* ensure BISR shift has finished */
		udelay(200);
	}