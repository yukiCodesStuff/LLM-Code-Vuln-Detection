/* SPDX-License-Identifier: GPL-2.0-only */
/******************************************************************************
 *
 * Copyright(c) 2009 - 2014 Intel Corporation. All rights reserved.
 * Copyright(c) 2015 Intel Mobile Communications GmbH
 * Copyright(c) 2016 - 2017 Intel Deutschland GmbH
 * Copyright(c) 2018        Intel Corporation
 *****************************************************************************/

#if !defined(__IWLWIFI_DEVICE_TRACE_IWLWIFI) || defined(TRACE_HEADER_MULTI_READ)
#define __IWLWIFI_DEVICE_TRACE_IWLWIFI

#include <linux/tracepoint.h>

#undef TRACE_SYSTEM
#define TRACE_SYSTEM iwlwifi

TRACE_EVENT(iwlwifi_dev_hcmd,
	TP_PROTO(const struct device *dev,
		 struct iwl_host_cmd *cmd, u16 total_size,
		 struct iwl_cmd_header_wide *hdr),
	TP_ARGS(dev, cmd, total_size, hdr),
	TP_STRUCT__entry(
		DEV_ENTRY
		__dynamic_array(u8, hcmd, total_size)
		__field(u32, flags)
	),
	TP_fast_assign(
		int i, offset = sizeof(struct iwl_cmd_header);

		if (hdr->group_id)
			offset = sizeof(struct iwl_cmd_header_wide);

		DEV_ASSIGN;
		__entry->flags = cmd->flags;
		memcpy(__get_dynamic_array(hcmd), hdr, offset);

		for (i = 0; i < IWL_MAX_CMD_TBS_PER_TFD; i++) {
			if (!cmd->len[i])
				continue;
			memcpy((u8 *)__get_dynamic_array(hcmd) + offset,
			       cmd->data[i], cmd->len[i]);
			offset += cmd->len[i];
		}