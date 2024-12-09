/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2005-2014, 2020 Intel Corporation
 * Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 */
#ifndef __iwl_drv_h__
#define __iwl_drv_h__

/* for all modules */
#define DRV_NAME        "iwlwifi"
#define DRV_AUTHOR	"Intel Corporation <linuxwifi@intel.com>"

/* radio config bits (actual values from NVM definition) */
#define NVM_RF_CFG_DASH_MSK(x)   (x & 0x3)         /* bits 0-1   */
#define NVM_RF_CFG_STEP_MSK(x)   ((x >> 2)  & 0x3) /* bits 2-3   */