
#include <linux/bitops.h>

#define IWL_FW_INI_HW_SMEM_REGION_ID		15
#define IWL_FW_INI_MAX_REGION_ID		64
#define IWL_FW_INI_MAX_NAME			32
#define IWL_FW_INI_MAX_CFG_NAME			64
#define IWL_FW_INI_DOMAIN_ALWAYS_ON		0
	struct iwl_fw_ini_hcmd hcmd;
} __packed; /* FW_TLV_DEBUG_HCMD_API_S_VER_1 */

/**
* struct iwl_fw_ini_conf_tlv - preset configuration TLV
*
* @address: the base address
* @value: value to set at address

*/
struct iwl_fw_ini_addr_val {
	__le32 address;
	__le32 value;
} __packed; /* FW_TLV_DEBUG_ADDR_VALUE_VER_1 */

/**
 * struct iwl_fw_ini_conf_tlv - configuration TLV to set register/memory.
 *
 * @hdr: debug header
 * @time_point: time point to apply config. One of &enum iwl_fw_ini_time_point
 * @set_type: write access type preset token for time point.
 *  one of &enum iwl_fw_ini_config_set_type
 * @addr_offset: the offset to add to any item in address[0] field
 * @addr_val: address value pair
 */
struct iwl_fw_ini_conf_set_tlv {
	struct iwl_fw_ini_header hdr;
	__le32 time_point;
	__le32 set_type;
	__le32 addr_offset;
	struct iwl_fw_ini_addr_val addr_val[0];
} __packed; /* FW_TLV_DEBUG_CONFIG_SET_API_S_VER_1 */

/**
 * enum iwl_fw_ini_config_set_type
 *
 * @IWL_FW_INI_CONFIG_SET_TYPE_INVALID: invalid config set
 * @IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_MAC: for PERIPHERY MAC configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_PHY: for PERIPHERY PHY configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_AUX: for PERIPHERY AUX configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_MEMORY: for DEVICE MEMORY configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_CSR: for CSR configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_DBGC_DRAM_ADDR: for DBGC_DRAM_ADDR configuration
 * @IWL_FW_INI_CONFIG_SET_TYPE_PERIPH_SCRATCH_HWM: for PERIPH SCRATCH HWM configuration
 * @IWL_FW_INI_ALLOCATION_NUM: max number of configuration supported
*/

enum iwl_fw_ini_config_set_type {
	IWL_FW_INI_CONFIG_SET_TYPE_INVALID = 0,
	IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_MAC,
	IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_PHY,
	IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_PERIPHERY_AUX,
	IWL_FW_INI_CONFIG_SET_TYPE_DEVICE_MEMORY,
	IWL_FW_INI_CONFIG_SET_TYPE_CSR,
	IWL_FW_INI_CONFIG_SET_TYPE_DBGC_DRAM_ADDR,
	IWL_FW_INI_CONFIG_SET_TYPE_PERIPH_SCRATCH_HWM,
	IWL_FW_INI_CONFIG_SET_TYPE_MAX_NUM,
} __packed;

/**
 * enum iwl_fw_ini_allocation_id
 *
 * @IWL_FW_INI_ALLOCATION_INVALID: invalid