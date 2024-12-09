// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2005-2011, 2021 Intel Corporation
 */
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/export.h>
__iwl_fn(crit)
IWL_EXPORT_SYMBOL(__iwl_crit);

void __iwl_err(struct device *dev, enum iwl_err_mode mode, const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args, args2;

	va_start(args, fmt);
	switch (mode) {
	case IWL_ERR_MODE_RATELIMIT:
		if (net_ratelimit())
			break;
		fallthrough;
	case IWL_ERR_MODE_REGULAR:
	case IWL_ERR_MODE_RFKILL:
		va_copy(args2, args);
		vaf.va = &args2;
		if (mode == IWL_ERR_MODE_RFKILL)
			dev_err(dev, "(RFKILL) %pV", &vaf);
		else
			dev_err(dev, "%pV", &vaf);
		va_end(args2);
		break;
	default:
		break;
	}
	trace_iwlwifi_err(&vaf);
	va_end(args);
}