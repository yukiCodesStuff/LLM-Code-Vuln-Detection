
#include <linux/bitops.h>

#define IWL_FW_INI_MAX_REGION_ID		64
#define IWL_FW_INI_MAX_NAME			32
#define IWL_FW_INI_MAX_CFG_NAME			64
#define IWL_FW_INI_DOMAIN_ALWAYS_ON		0
	struct iwl_fw_ini_hcmd hcmd;
} __packed; /* FW_TLV_DEBUG_HCMD_API_S_VER_1 */

/**
 * enum iwl_fw_ini_allocation_id
 *
 * @IWL_FW_INI_ALLOCATION_INVALID: invalid