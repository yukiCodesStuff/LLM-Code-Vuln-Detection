	return 0;
}

static int _iwl_trans_pcie_start_hw(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	int err;

	iwl_trans_pcie_sw_reset(trans);

	err = iwl_pcie_apm_init(trans);
	if (err)
		return err;
