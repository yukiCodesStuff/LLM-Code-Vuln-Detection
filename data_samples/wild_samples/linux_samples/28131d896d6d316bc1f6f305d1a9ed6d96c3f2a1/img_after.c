// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright(c) 2019 - 2020 Intel Corporation
 */

#include "img.h"

u8 iwl_fw_lookup_cmd_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def)
{
	const struct iwl_fw_cmd_version *entry;
	unsigned int i;

	if (!fw->ucode_capa.cmd_versions ||
	    !fw->ucode_capa.n_cmd_versions)
		return def;

	entry = fw->ucode_capa.cmd_versions;
	for (i = 0; i < fw->ucode_capa.n_cmd_versions; i++, entry++) {
		if (entry->group == grp && entry->cmd == cmd) {
			if (entry->cmd_ver == IWL_FW_CMD_VER_UNKNOWN)
				return def;
			return entry->cmd_ver;
		}
	}

	return def;
}