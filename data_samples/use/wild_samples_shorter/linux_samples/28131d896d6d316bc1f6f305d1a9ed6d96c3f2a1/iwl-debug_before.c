// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2005-2011 Intel Corporation
 */
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/export.h>
__iwl_fn(crit)
IWL_EXPORT_SYMBOL(__iwl_crit);

void __iwl_err(struct device *dev, bool rfkill_prefix, bool trace_only,
		const char *fmt, ...)
{
	struct va_format vaf = {
		.fmt = fmt,
	};
	va_list args;

	va_start(args, fmt);
	vaf.va = &args;
	if (!trace_only) {
		if (rfkill_prefix)
			dev_err(dev, "(RFKILL) %pV", &vaf);
		else
			dev_err(dev, "%pV", &vaf);
	}
	trace_iwlwifi_err(&vaf);
	va_end(args);
}