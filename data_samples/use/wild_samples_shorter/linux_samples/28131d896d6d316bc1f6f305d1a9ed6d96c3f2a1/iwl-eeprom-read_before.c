// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2005-2014, 2018-2019 Intel Corporation
 */
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/export.h>
{
	int ret;

	ret = iwl_finish_nic_init(trans, trans->trans_cfg);
	if (ret)
		return ret;

	iwl_set_bits_prph(trans, APMG_PS_CTRL_REG,