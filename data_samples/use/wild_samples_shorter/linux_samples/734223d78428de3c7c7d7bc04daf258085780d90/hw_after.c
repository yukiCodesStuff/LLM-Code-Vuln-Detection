#include <linux/bitops.h>
#include <linux/bitfield.h>

#include "core.h"
#include "ce.h"
#include "hif.h"
#include "hal.h"
#include "hw.h"

/* Map from pdev index to hw mac index */
static u8 ath11k_hw_ipq8074_mac_from_pdev_id(int pdev_idx)
{
	.pcie_qserdes_sysclk_en_sel = 0x01e0c0ac,
	.pcie_pcs_osc_dtct_config_base = 0x01e0c628,
};

const struct ath11k_hw_hal_params ath11k_hw_hal_params_ipq8074 = {
	.rx_buf_rbm = HAL_RX_BUF_RBM_SW3_BM,
};

const struct ath11k_hw_hal_params ath11k_hw_hal_params_qca6390 = {
	.rx_buf_rbm = HAL_RX_BUF_RBM_SW1_BM,
};