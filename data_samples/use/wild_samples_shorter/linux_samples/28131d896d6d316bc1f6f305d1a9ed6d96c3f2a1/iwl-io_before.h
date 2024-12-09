/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2018-2020 Intel Corporation
 */
#ifndef __iwl_io_h__
#define __iwl_io_h__

void iwl_clear_bits_prph(struct iwl_trans *trans, u32 ofs, u32 mask);
void iwl_force_nmi(struct iwl_trans *trans);

int iwl_finish_nic_init(struct iwl_trans *trans,
			const struct iwl_cfg_trans_params *cfg_trans);

/* Error handling */
int iwl_dump_fh(struct iwl_trans *trans, char **buf);
