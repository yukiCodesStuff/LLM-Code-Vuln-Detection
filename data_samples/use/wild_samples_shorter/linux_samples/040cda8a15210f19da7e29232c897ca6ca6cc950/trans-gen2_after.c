#include "internal.h"
#include "fw/dbg.h"

/*
 * Start up NIC's basic functionality after it has been reset
 * (e.g. after platform boot, or shutdown via iwl_pcie_apm_stop())
 * NOTE:  This does not load uCode nor start the embedded processor

	iwl_pcie_apm_config(trans);

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;
