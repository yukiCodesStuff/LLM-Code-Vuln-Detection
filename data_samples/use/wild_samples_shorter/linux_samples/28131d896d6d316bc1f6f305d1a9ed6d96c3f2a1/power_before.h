/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2012-2014, 2018-2020 Intel Corporation
 * Copyright (C) 2013-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2015-2017 Intel Deutschland GmbH
 */
#ifndef __iwl_fw_api_power_h__
	};
};

#define IWL_NUM_GEO_PROFILES   3
#define IWL_NUM_BANDS_PER_CHAIN_V1 2
#define IWL_NUM_BANDS_PER_CHAIN_V2 3

/**
 * enum iwl_geo_per_chain_offset_operation - type of operation
 * @IWL_PER_CHAIN_OFFSET_SET_TABLES: send the tables from the host to the FW.
enum iwl_geo_per_chain_offset_operation {
	IWL_PER_CHAIN_OFFSET_SET_TABLES,
	IWL_PER_CHAIN_OFFSET_GET_CURRENT_TABLE,
};  /* GEO_TX_POWER_LIMIT FLAGS TYPE */

/**
 * struct iwl_per_chain_offset - embedded struct for GEO_TX_POWER_LIMIT.
 * @max_tx_power: maximum allowed tx power.
 * @chain_a: tx power offset for chain a.
 * @chain_b: tx power offset for chain b.
 */
} __packed; /* PER_CHAIN_LIMIT_OFFSET_PER_CHAIN_S_VER_1 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v1 - struct for GEO_TX_POWER_LIMIT cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 */
struct iwl_geo_tx_power_profiles_cmd_v1 {
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V1];
} __packed; /* GEO_TX_POWER_LIMIT_VER_1 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v2 - struct for GEO_TX_POWER_LIMIT cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V1];
	__le32 table_revision;
} __packed; /* GEO_TX_POWER_LIMIT_VER_2 */

/**
 * struct iwl_geo_tx_power_profile_cmd_v3 - struct for GEO_TX_POWER_LIMIT cmd.
 * @ops: operations, value from &enum iwl_geo_per_chain_offset_operation
 * @table: offset profile per band.
 * @table_revision: BIOS table revision.
 */
	__le32 ops;
	struct iwl_per_chain_offset table[IWL_NUM_GEO_PROFILES][IWL_NUM_BANDS_PER_CHAIN_V2];
	__le32 table_revision;
} __packed; /* GEO_TX_POWER_LIMIT_VER_3 */

union iwl_geo_tx_power_profiles_cmd {
	struct iwl_geo_tx_power_profiles_cmd_v1 v1;
	struct iwl_geo_tx_power_profiles_cmd_v2 v2;
	struct iwl_geo_tx_power_profiles_cmd_v3 v3;
};

/**
 * struct iwl_geo_tx_power_profiles_resp -  response to GEO_TX_POWER_LIMIT cmd
 * @profile_idx: current geo profile in use
 */
struct iwl_geo_tx_power_profiles_resp {
	__le32 profile_idx;
} __packed; /* GEO_TX_POWER_LIMIT_RESP */

/**
 * union iwl_ppag_table_cmd - union for all versions of PPAG command
 * @v1: version 1