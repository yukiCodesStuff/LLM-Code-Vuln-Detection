/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2005-2014, 2021 Intel Corporation
 */
#ifndef __iwl_agn_h__
#define __iwl_agn_h__

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
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev, IWL_ERR_MODE_TRACE_ONLY,		\
			  fmt, ##args);					\
} while (0)
#endif				/* CONFIG_IWLWIFI_DEBUG */

#endif /* __iwl_agn_h__ */