	return 0;
}

int iwl_finish_nic_init(struct iwl_trans *trans,
			const struct iwl_cfg_trans_params *cfg_trans)
{
	u32 poll_ready;
	int err;

	if (cfg_trans->bisr_workaround) {
	 * and accesses to uCode SRAM.
	 */
	err = iwl_poll_bit(trans, CSR_GP_CNTRL, poll_ready, poll_ready, 25000);
	if (err < 0)
		IWL_DEBUG_INFO(trans, "Failed to wake NIC\n");

	if (cfg_trans->bisr_workaround) {
		/* ensure BISR shift has finished */
		udelay(200);
	}