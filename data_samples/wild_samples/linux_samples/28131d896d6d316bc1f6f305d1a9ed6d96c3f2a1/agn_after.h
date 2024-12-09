/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2005-2014, 2021 Intel Corporation
 */
#ifndef __iwl_agn_h__
#define __iwl_agn_h__

#include "iwl-config.h"

#include "dev.h"

/* The first 11 queues (0-10) are used otherwise */
#define IWLAGN_FIRST_AMPDU_QUEUE	11

/* AUX (TX during scan dwell) queue */
#define IWL_AUX_QUEUE		10

#define IWL_INVALID_STATION	255

/* device operations */
extern const struct iwl_dvm_cfg iwl_dvm_1000_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_2000_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_105_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_2030_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_5000_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_5150_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_6000_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_6005_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_6050_cfg;
extern const struct iwl_dvm_cfg iwl_dvm_6030_cfg;


#define TIME_UNIT		1024

/*****************************************************
* DRIVER STATUS FUNCTIONS
******************************************************/
#define STATUS_RF_KILL_HW	0
#define STATUS_CT_KILL		1
#define STATUS_ALIVE		2
#define STATUS_READY		3
#define STATUS_EXIT_PENDING	5
#define STATUS_STATISTICS	6
#define STATUS_SCANNING		7
#define STATUS_SCAN_ABORTING	8
#define STATUS_SCAN_HW		9
#define STATUS_FW_ERROR		10
#define STATUS_CHANNEL_SWITCH_PENDING 11
#define STATUS_SCAN_COMPLETE	12
#define STATUS_POWER_PMI	13

struct iwl_ucode_capabilities;

extern const struct ieee80211_ops iwlagn_hw_ops;

static inline void iwl_set_calib_hdr(struct iwl_calib_hdr *hdr, u8 cmd)
{
	hdr->op_code = cmd;
	hdr->first_group = 0;
	hdr->groups_num = 1;
	hdr->data_valid = 1;
}
{
	if (state)
		set_bit(STATUS_POWER_PMI, &priv->status);
	else
		clear_bit(STATUS_POWER_PMI, &priv->status);
	iwl_trans_set_pmi(priv->trans, state);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
void iwl_dbgfs_register(struct iwl_priv *priv, struct dentry *dbgfs_dir);
#else
static inline void iwl_dbgfs_register(struct iwl_priv *priv,
				      struct dentry *dbgfs_dir) { }
#endif /* CONFIG_IWLWIFI_DEBUGFS */

#ifdef CONFIG_IWLWIFI_DEBUG
#define IWL_DEBUG_QUIET_RFKILL(m, fmt, args...)	\
do {									\
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev,					\
			  iwl_have_debug_level(IWL_DL_RADIO) ?		\
				IWL_ERR_MODE_RFKILL :			\
				IWL_ERR_MODE_TRACE_ONLY,		\
			  fmt, ##args);					\
} while (0)
#else
#define IWL_DEBUG_QUIET_RFKILL(m, fmt, args...)	\
do {									\
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev, IWL_ERR_MODE_TRACE_ONLY,		\
			  fmt, ##args);					\
} while (0)
#endif				/* CONFIG_IWLWIFI_DEBUG */

#endif /* __iwl_agn_h__ */
{
	if (state)
		set_bit(STATUS_POWER_PMI, &priv->status);
	else
		clear_bit(STATUS_POWER_PMI, &priv->status);
	iwl_trans_set_pmi(priv->trans, state);
}

#ifdef CONFIG_IWLWIFI_DEBUGFS
void iwl_dbgfs_register(struct iwl_priv *priv, struct dentry *dbgfs_dir);
#else
static inline void iwl_dbgfs_register(struct iwl_priv *priv,
				      struct dentry *dbgfs_dir) { }
#endif /* CONFIG_IWLWIFI_DEBUGFS */

#ifdef CONFIG_IWLWIFI_DEBUG
#define IWL_DEBUG_QUIET_RFKILL(m, fmt, args...)	\
do {									\
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev,					\
			  iwl_have_debug_level(IWL_DL_RADIO) ?		\
				IWL_ERR_MODE_RFKILL :			\
				IWL_ERR_MODE_TRACE_ONLY,		\
			  fmt, ##args);					\
} while (0)
#else
#define IWL_DEBUG_QUIET_RFKILL(m, fmt, args...)	\
do {									\
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev, IWL_ERR_MODE_TRACE_ONLY,		\
			  fmt, ##args);					\
} while (0)
#endif				/* CONFIG_IWLWIFI_DEBUG */

#endif /* __iwl_agn_h__ */