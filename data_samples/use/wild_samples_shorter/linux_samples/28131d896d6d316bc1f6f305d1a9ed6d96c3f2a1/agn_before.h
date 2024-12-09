/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2005-2014 Intel Corporation
 */
#ifndef __iwl_agn_h__
#define __iwl_agn_h__

	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev, true,				\
			  !iwl_have_debug_level(IWL_DL_RADIO),		\
			  fmt, ##args);					\
} while (0)
#else
#define IWL_DEBUG_QUIET_RFKILL(m, fmt, args...)	\
	if (!iwl_is_rfkill((m)))					\
		IWL_ERR(m, fmt, ##args);				\
	else								\
		__iwl_err((m)->dev, true, true, fmt, ##args);	\
} while (0)
#endif				/* CONFIG_IWLWIFI_DEBUG */

#endif /* __iwl_agn_h__ */