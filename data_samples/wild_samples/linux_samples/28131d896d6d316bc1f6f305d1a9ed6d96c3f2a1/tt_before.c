// SPDX-License-Identifier: GPL-2.0-only
/******************************************************************************
 *
 * Copyright(c) 2007 - 2014 Intel Corporation. All rights reserved.
 * Copyright (C) 2018, 2020 Intel Corporation
 *
 * Portions of this file are derived from the ipw3945 project, as well
 * as portions of the ieee80211 subsystem header files.
 *
 * Contact Information:
 *  Intel Linux Wireless <linuxwifi@intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *****************************************************************************/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <net/mac80211.h>
#include "iwl-io.h"
#include "iwl-modparams.h"
#include "iwl-debug.h"
#include "agn.h"
#include "dev.h"
#include "commands.h"
#include "tt.h"

/* default Thermal Throttling transaction table
 * Current state   |         Throttling Down               |  Throttling Up
 *=============================================================================
 *                 Condition Nxt State  Condition Nxt State Condition Nxt State
 *-----------------------------------------------------------------------------
 *     IWL_TI_0     T >= 114   CT_KILL  114>T>=105   TI_1      N/A      N/A
 *     IWL_TI_1     T >= 114   CT_KILL  114>T>=110   TI_2     T<=95     TI_0
 *     IWL_TI_2     T >= 114   CT_KILL                        T<=100    TI_1
 *    IWL_CT_KILL      N/A       N/A       N/A        N/A     T<=95     TI_0
 *=============================================================================
 */
static const struct iwl_tt_trans tt_range_0[IWL_TI_STATE_MAX - 1] = {
	{IWL_TI_0, IWL_ABSOLUTE_ZERO, 104},
	{IWL_TI_1, 105, CT_KILL_THRESHOLD - 1},
	{IWL_TI_CT_KILL, CT_KILL_THRESHOLD, IWL_ABSOLUTE_MAX}