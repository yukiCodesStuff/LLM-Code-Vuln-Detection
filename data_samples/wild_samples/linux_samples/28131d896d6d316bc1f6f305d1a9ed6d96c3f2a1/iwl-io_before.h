/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2018-2020 Intel Corporation
 */
#ifndef __iwl_io_h__
#define __iwl_io_h__

#include "iwl-devtrace.h"
#include "iwl-trans.h"

void iwl_write8(struct iwl_trans *trans, u32 ofs, u8 val);
void iwl_write32(struct iwl_trans *trans, u32 ofs, u32 val);
void iwl_write64(struct iwl_trans *trans, u64 ofs, u64 val);
u32 iwl_read32(struct iwl_trans *trans, u32 ofs);

static inline void iwl_set_bit(struct iwl_trans *trans, u32 reg, u32 mask)
{
	iwl_trans_set_bits_mask(trans, reg, mask, mask);
}
{
	iwl_write_prph_delay(trans, ofs, val, 0);
}

int iwl_poll_prph_bit(struct iwl_trans *trans, u32 addr,
		      u32 bits, u32 mask, int timeout);
void iwl_set_bits_prph(struct iwl_trans *trans, u32 ofs, u32 mask);
void iwl_set_bits_mask_prph(struct iwl_trans *trans, u32 ofs,
			    u32 bits, u32 mask);
void iwl_clear_bits_prph(struct iwl_trans *trans, u32 ofs, u32 mask);
void iwl_force_nmi(struct iwl_trans *trans);

int iwl_finish_nic_init(struct iwl_trans *trans,
			const struct iwl_cfg_trans_params *cfg_trans);

/* Error handling */
int iwl_dump_fh(struct iwl_trans *trans, char **buf);

/*
 * UMAC periphery address space changed from 0xA00000 to 0xD00000 starting from
 * device family AX200. So peripheries used in families above and below AX200
 * should go through iwl_..._umac_..._prph.
 */
static inline u32 iwl_umac_prph(struct iwl_trans *trans, u32 ofs)
{
	return ofs + trans->trans_cfg->umac_prph_offset;
}