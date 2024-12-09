/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2012-2014, 2018-2021 Intel Corporation
 * Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2015-2017 Intel Deutschland GmbH
 */
#ifndef __iwl_fw_api_power_h__
	};
};

#define IWL_NUM_GEO_PROFILES		3
#define IWL_NUM_GEO_PROFILES_V3		8
#define IWL_NUM_BANDS_PER_CHAIN_V1	2
#define IWL_NUM_BANDS_PER_CHAIN_V2	3

/**
 * enum iwl_geo_per_chain_offset_operation - type of operation
 * @IWL_PER_CHAIN_OFFSET_SET_TABLES: send the tables from the host to the FW.
enum iwl_geo_per_chain_offset_operation {
	IWL_PER_CHAIN_OFFSET_SET_TABLES,
	IWL_PER_CHAIN_OFFSET_GET_CURRENT_TABLE,
};  /* PER_CHAIN_OFFSET_OPERATION_E */

/**
 * struct iwl_per_chain_offset - embedded struct for PER_CHAIN_LIMIT_OFFSET_CMD.
 * @max_tx_power: maximum allowed tx power.
 * @chain_a: tx power offset for chain a.
 * @chain_b: tx power offset for chain b.
 */
} __packed; /* PER_CHAIN_LIMIT_OFFSET_PER_CHAIN_S_VER_1 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v1 - struct for PER_CHAIN_LIMIT_OFFSET_CMD cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 */
struct iwl_geo_tx_power_profiles_cmd_v1 {
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V1];
} __packed; /* PER_CHAIN_LIMIT_OFFSET_CMD_VER_1 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v2 - struct for PER_CHAIN_LIMIT_OFFSET_CMD cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V1];
	__le32 table_revision;
} __packed; /* PER_CHAIN_LIMIT_OFFSET_CMD_VER_2 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v3 - struct for PER_CHAIN_LIMIT_OFFSET_CMD cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V2];
	__le32 table_revision;
} __packed; /* PER_CHAIN_LIMIT_OFFSET_CMD_VER_3 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v4 - struct for PER_CHAIN_LIMIT_OFFSET_CMD cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
struct iwl_geo_tx_power_profiles_cmd_v4 {
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES_V3][IWL_NUM_BANDS_PER_CHAIN_V1];
	__le32 table_revision;
} __packed; /* PER_CHAIN_LIMIT_OFFSET_CMD_VER_4 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v5 - struct for PER_CHAIN_LIMIT_OFFSET_CMD cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
struct iwl_geo_tx_power_profiles_cmd_v5 {
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES_V3][IWL_NUM_BANDS_PER_CHAIN_V2];
	__le32 table_revision;
} __packed; /* PER_CHAIN_LIMIT_OFFSET_CMD_VER_5 */

union iwl_geo_tx_power_profiles_cmd {
	struct iwl_geo_tx_power_profiles_cmd_v1 v1;
	struct iwl_geo_tx_power_profiles_cmd_v2 v2;
	struct iwl_geo_tx_power_profiles_cmd_v3 v3;
	struct iwl_geo_tx_power_profiles_cmd_v4 v4;
	struct iwl_geo_tx_power_profiles_cmd_v5 v5;
};

/**
 * struct iwl_geo_tx_power_profiles_resp -  response to PER_CHAIN_LIMIT_OFFSET_CMD cmd
 * @profile_idx: current geo profile in use
 */
struct iwl_geo_tx_power_profiles_resp {
	__le32 profile_idx;
} __packed; /* PER_CHAIN_LIMIT_OFFSET_RSP */

/**
 * union iwl_ppag_table_cmd - union for all versions of PPAG command
 * @v1: version 1