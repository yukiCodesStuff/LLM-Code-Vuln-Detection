#ifndef ATH11K_HW_H
#define ATH11K_HW_H

#include "hal.h"
#include "wmi.h"

/* Target configuration defines */

	u8 host2rxdma[ATH11K_EXT_IRQ_GRP_NUM_MAX];
};

struct ath11k_hw_hal_params {
	enum hal_rx_buf_return_buf_manager rx_buf_rbm;
};

struct ath11k_hw_params {
	const char *name;
	u16 hw_rev;
	u8 max_radios;
	u32 hal_desc_sz;
	bool fix_l1ss;
	u8 max_tx_ring;
	const struct ath11k_hw_hal_params *hal_params;
};

struct ath11k_hw_ops {
	u8 (*get_hw_mac_from_pdev_id)(int pdev_id);
extern const struct ath11k_hw_ring_mask ath11k_hw_ring_mask_qca6390;
extern const struct ath11k_hw_ring_mask ath11k_hw_ring_mask_qcn9074;

extern const struct ath11k_hw_hal_params ath11k_hw_hal_params_ipq8074;
extern const struct ath11k_hw_hal_params ath11k_hw_hal_params_qca6390;

static inline
int ath11k_hw_get_mac_from_pdev_id(struct ath11k_hw_params *hw,
				   int pdev_idx)
{