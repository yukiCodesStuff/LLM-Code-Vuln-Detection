/******************************************************************************
 *
 * Copyright(c) 2003 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2018 - 2021 Intel Corporation
 *
 * Portions of this file are derived from the ipw3945 project.
 *****************************************************************************/

#ifndef __iwl_debug_h__
#define __iwl_debug_h__
#endif
}

enum iwl_err_mode {
	IWL_ERR_MODE_REGULAR,
	IWL_ERR_MODE_RFKILL,
	IWL_ERR_MODE_TRACE_ONLY,
	IWL_ERR_MODE_RATELIMIT,
};

struct device;
void __iwl_err(struct device *dev, enum iwl_err_mode mode, const char *fmt, ...)
	__printf(3, 4);
void __iwl_warn(struct device *dev, const char *fmt, ...) __printf(2, 3);
void __iwl_info(struct device *dev, const char *fmt, ...) __printf(2, 3);
void __iwl_crit(struct device *dev, const char *fmt, ...) __printf(2, 3);

#define CHECK_FOR_NEWLINE(f) BUILD_BUG_ON(f[sizeof(f) - 2] != '\n')

/* No matter what is m (priv, bus, trans), this will work */
#define __IWL_ERR_DEV(d, mode, f, a...)					\
	do {								\
		CHECK_FOR_NEWLINE(f);					\
		__iwl_err((d), mode, f, ## a);				\
	} while (0)
#define IWL_ERR_DEV(d, f, a...)						\
	__IWL_ERR_DEV(d, IWL_ERR_MODE_REGULAR, f, ## a)
#define IWL_ERR(m, f, a...)						\
	IWL_ERR_DEV((m)->dev, f, ## a)
#define IWL_ERR_LIMIT(m, f, a...)					\
	__IWL_ERR_DEV((m)->dev, IWL_ERR_MODE_RATELIMIT, f, ## a)
#define IWL_WARN(m, f, a...)						\
	do {								\
		CHECK_FOR_NEWLINE(f);					\
		__iwl_warn((m)->dev, f, ## a);				\