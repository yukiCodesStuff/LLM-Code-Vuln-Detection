#include "internal.h"
#include "fw/dbg.h"

static int iwl_pcie_gen2_force_power_gating(struct iwl_trans *trans)
{
	iwl_set_bits_prph(trans, HPM_HIPM_GEN_CFG,
			  HPM_HIPM_GEN_CFG_CR_FORCE_ACTIVE);
	udelay(20);
	iwl_set_bits_prph(trans, HPM_HIPM_GEN_CFG,
			  HPM_HIPM_GEN_CFG_CR_PG_EN |
			  HPM_HIPM_GEN_CFG_CR_SLP_EN);
	udelay(20);
	iwl_clear_bits_prph(trans, HPM_HIPM_GEN_CFG,
			    HPM_HIPM_GEN_CFG_CR_FORCE_ACTIVE);

	iwl_trans_sw_reset(trans);
	iwl_clear_bit(trans, CSR_GP_CNTRL, CSR_GP_CNTRL_REG_FLAG_INIT_DONE);

	return 0;
}

/*
 * Start up NIC's basic functionality after it has been reset
 * (e.g. after platform boot, or shutdown via iwl_pcie_apm_stop())
 * NOTE:  This does not load uCode nor start the embedded processor

	iwl_pcie_apm_config(trans);

	if (trans->trans_cfg->device_family == IWL_DEVICE_FAMILY_22000 &&
	    trans->cfg->integrated) {
		ret = iwl_pcie_gen2_force_power_gating(trans);
		if (ret)
			return ret;
	}

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;
