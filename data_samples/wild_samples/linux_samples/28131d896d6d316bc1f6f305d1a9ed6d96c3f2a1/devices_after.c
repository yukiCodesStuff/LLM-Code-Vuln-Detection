// SPDX-License-Identifier: GPL-2.0-only
/******************************************************************************
 *
 * Copyright(c) 2008 - 2014 Intel Corporation. All rights reserved.
 * Copyright (C) 2019 Intel Corporation
 *****************************************************************************/

#include <linux/units.h>

/*
 * DVM device-specific data & functions
 */
#include "iwl-io.h"
#include "iwl-prph.h"
#include "iwl-eeprom-parse.h"

#include "agn.h"
#include "dev.h"
#include "commands.h"


/*
 * 1000 series
 * ===========
 */

/*
 * For 1000, use advance thermal throttling critical temperature threshold,
 * but legacy thermal management implementation for now.
 * This is for the reason of 1000 uCode using advance thermal throttling API
 * but not implement ct_kill_exit based on ct_kill exit temperature
 * so the thermal throttling will still based on legacy thermal throttling
 * management.
 * The code here need to be modified once 1000 uCode has the advanced thermal
 * throttling algorithm in place
 */
static void iwl1000_set_ct_threshold(struct iwl_priv *priv)
{
	/* want Celsius */
	priv->hw_params.ct_kill_threshold = CT_KILL_THRESHOLD_LEGACY;
	priv->hw_params.ct_kill_exit_threshold = CT_KILL_EXIT_THRESHOLD;
}