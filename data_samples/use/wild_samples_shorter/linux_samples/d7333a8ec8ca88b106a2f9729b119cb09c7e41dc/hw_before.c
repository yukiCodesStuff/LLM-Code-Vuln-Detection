#include <linux/bitops.h>
#include <linux/bitfield.h>

#include "hw.h"
#include "core.h"
#include "ce.h"
#include "hif.h"

/* Map from pdev index to hw mac index */
static u8 ath11k_hw_ipq8074_mac_from_pdev_id(int pdev_idx)
{
	.pcie_qserdes_sysclk_en_sel = 0x01e0c0ac,
	.pcie_pcs_osc_dtct_config_base = 0x01e0c628,
};