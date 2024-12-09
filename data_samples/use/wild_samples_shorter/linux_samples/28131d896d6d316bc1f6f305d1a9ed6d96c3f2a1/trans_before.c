static void iwl_trans_pcie_sw_reset(struct iwl_trans *trans)
{
	/* Reset entire device - do controller reset (results in SHRD_HW_RST) */
	iwl_set_bit(trans, CSR_RESET, CSR_RESET_REG_FLAG_SW_RESET);
	usleep_range(5000, 6000);
}

static void iwl_pcie_free_fw_monitor(struct iwl_trans *trans)
	if (trans->trans_cfg->base_params->pll_cfg)
		iwl_set_bit(trans, CSR_ANA_PLL_CFG, CSR50_ANA_PLL_CFG_VAL);

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;

	if (trans->cfg->host_interrupt_operation_mode) {

	iwl_trans_pcie_sw_reset(trans);

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (WARN_ON(ret)) {
		/* Release XTAL ON request */
		__iwl_trans_pcie_clear_bit(trans, CSR_GP_CNTRL,
					   CSR_GP_CNTRL_REG_FLAG_XTAL_ON);
				   CSR_GP_CNTRL_REG_FLAG_BUS_MASTER_DISABLE_STATUS,
				   CSR_GP_CNTRL_REG_FLAG_BUS_MASTER_DISABLE_STATUS,
				   100);
	} else {
		iwl_set_bit(trans, CSR_RESET, CSR_RESET_REG_FLAG_STOP_MASTER);

		ret = iwl_poll_bit(trans, CSR_RESET,
	u8 addr;
};

static struct iwl_causes_list causes_list[] = {
	{MSIX_FH_INT_CAUSES_D2S_CH0_NUM,	CSR_MSIX_FH_INT_MASK_AD, 0},
	{MSIX_FH_INT_CAUSES_D2S_CH1_NUM,	CSR_MSIX_FH_INT_MASK_AD, 0x1},
	{MSIX_FH_INT_CAUSES_S2D,		CSR_MSIX_FH_INT_MASK_AD, 0x3},
	{MSIX_FH_INT_CAUSES_FH_ERR,		CSR_MSIX_FH_INT_MASK_AD, 0x5},
	{MSIX_HW_INT_CAUSES_REG_CT_KILL,	CSR_MSIX_HW_INT_MASK_AD, 0x16},
	{MSIX_HW_INT_CAUSES_REG_RF_KILL,	CSR_MSIX_HW_INT_MASK_AD, 0x17},
	{MSIX_HW_INT_CAUSES_REG_PERIODIC,	CSR_MSIX_HW_INT_MASK_AD, 0x18},
	{MSIX_HW_INT_CAUSES_REG_SW_ERR,		CSR_MSIX_HW_INT_MASK_AD, 0x29},
	{MSIX_HW_INT_CAUSES_REG_SCD,		CSR_MSIX_HW_INT_MASK_AD, 0x2A},
	{MSIX_HW_INT_CAUSES_REG_FH_TX,		CSR_MSIX_HW_INT_MASK_AD, 0x2B},
	{MSIX_HW_INT_CAUSES_REG_HW_ERR,		CSR_MSIX_HW_INT_MASK_AD, 0x2D},
	{MSIX_HW_INT_CAUSES_REG_HAP,		CSR_MSIX_HW_INT_MASK_AD, 0x2E},
};

static void iwl_pcie_map_non_rx_causes(struct iwl_trans *trans)
{
	struct iwl_trans_pcie *trans_pcie =  IWL_TRANS_GET_PCIE_TRANS(trans);
	int val = trans_pcie->def_irq | MSIX_NON_AUTO_CLEAR_CAUSE;
	int i, arr_size = ARRAY_SIZE(causes_list);
	struct iwl_causes_list *causes = causes_list;

	/*
	 * Access all non RX causes and map them to the default irq.
	 * In case we are missing at least one interrupt vector,
	 * the first interrupt vector will serve non-RX and FBQ causes.
	 */
	for (i = 0; i < arr_size; i++) {
		iwl_write8(trans, CSR_MSIX_IVAR(causes[i].addr), val);
		iwl_clear_bit(trans, causes[i].mask_reg,
			      causes[i].cause_num);
	}
}

static void iwl_pcie_map_rx_causes(struct iwl_trans *trans)
{
	}

	/* Make sure (redundant) we've released our request to stay awake */
	iwl_clear_bit(trans, CSR_GP_CNTRL,
		      CSR_GP_CNTRL_REG_FLAG_MAC_ACCESS_REQ);

	/* Stop the device, and put it in low power state */
	iwl_pcie_apm_stop(trans, false);

	iwl_set_bit(trans, CSR_GP_CNTRL,
		    CSR_GP_CNTRL_REG_FLAG_MAC_ACCESS_REQ);

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;

	/*
{
	int ret;

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret < 0)
		return ret;

	iwl_set_bits_prph(trans, HPM_HIPM_GEN_CFG,

	if (trans_pcie->cmd_hold_nic_awake)
		goto out;

	__iwl_trans_pcie_clear_bit(trans, CSR_GP_CNTRL,
				   CSR_GP_CNTRL_REG_FLAG_MAC_ACCESS_REQ);
	/*
	 * Above we read the CSR_GP_CNTRL register, which will flush
	 * any previous writes, but we need the write that clears the
	 * MAC_ACCESS_REQ bit to be performed before any other writes
	return 0;
}

static struct iwl_trans_dump_data
*iwl_trans_pcie_dump_data(struct iwl_trans *trans,
			  u32 dump_mask)
{
	struct iwl_trans_pcie *trans_pcie = IWL_TRANS_GET_PCIE_TRANS(trans);
	struct iwl_fw_error_dump_data *data;
	struct iwl_txq *cmdq = trans->txqs.txq[trans->txqs.cmd.q_id];
				txcmd->caplen = cpu_to_le32(caplen);
				memcpy(txcmd->data, cmdq->entries[idx].cmd,
				       caplen);
				txcmd = (void *)((u8 *)txcmd->data + caplen);
			}

			ptr = iwl_txq_dec_wrap(trans, ptr);

	if (trans_pcie->msix_enabled) {
		inta_addr = CSR_MSIX_HW_INT_CAUSES_AD;
		sw_err_bit = MSIX_HW_INT_CAUSES_REG_SW_ERR;
	} else {
		inta_addr = CSR_INT;
		sw_err_bit = CSR_INT_BIT_SW_ERR;
	}