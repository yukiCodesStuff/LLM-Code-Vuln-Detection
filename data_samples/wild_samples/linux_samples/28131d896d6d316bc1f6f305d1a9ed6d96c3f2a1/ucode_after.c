// SPDX-License-Identifier: GPL-2.0-only
/******************************************************************************
 *
 * Copyright(c) 2008 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2015 Intel Deutschland GmbH
 *****************************************************************************/

#include <linux/kernel.h>

#include "iwl-io.h"
#include "iwl-agn-hw.h"
#include "iwl-trans.h"
#include "iwl-fh.h"
#include "iwl-op-mode.h"

#include "dev.h"
#include "agn.h"
#include "calib.h"

/******************************************************************************
 *
 * uCode download functions
 *
 ******************************************************************************/

/*
 *  Calibration
 */
static int iwl_set_Xtal_calib(struct iwl_priv *priv)
{
	struct iwl_calib_xtal_freq_cmd cmd;
	__le16 *xtal_calib = priv->nvm_data->xtal_calib;

	iwl_set_calib_hdr(&cmd.hdr, IWL_PHY_CALIBRATE_CRYSTAL_FRQ_CMD);
	cmd.cap_pin1 = le16_to_cpu(xtal_calib[0]);
	cmd.cap_pin2 = le16_to_cpu(xtal_calib[1]);
	return iwl_calib_set(priv, (void *)&cmd, sizeof(cmd));
}