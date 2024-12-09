#include "iwl-prph.h"

/* Highest firmware API version supported */
#define IWL_22000_UCODE_API_MAX	66

/* Lowest firmware API version supported */
#define IWL_22000_UCODE_API_MIN	39

#define IWL_BZ_A_GF_A_FW_PRE		"iwlwifi-bz-a0-gf-a0-"
#define IWL_BZ_A_GF4_A_FW_PRE		"iwlwifi-bz-a0-gf4-a0-"
#define IWL_BZ_A_MR_A_FW_PRE		"iwlwifi-bz-a0-mr-a0-"

#define IWL_QU_B_HR_B_MODULE_FIRMWARE(api) \
	IWL_QU_B_HR_B_FW_PRE __stringify(api) ".ucode"
#define IWL_QNJ_B_HR_B_MODULE_FIRMWARE(api)	\
	IWL_BZ_A_GF4_A_FW_PRE __stringify(api) ".ucode"
#define IWL_BZ_A_MR_A_MODULE_FIRMWARE(api) \
	IWL_BZ_A_MR_A_FW_PRE __stringify(api) ".ucode"

static const struct iwl_base_params iwl_22000_base_params = {
	.eeprom_size = OTP_LOW_IMAGE_SIZE_32K,
	.num_of_queues = 512,
	"Killer(R) Wi-Fi 6E AX1675w 160MHz Wireless Network Adapter (210D2W)";
const char iwl_ax210_killer_1675x_name[] =
	"Killer(R) Wi-Fi 6E AX1675x 160MHz Wireless Network Adapter (210NGW)";

const struct iwl_cfg iwl_qu_b0_hr1_b0 = {
	.fw_name_pre = IWL_QU_B_HR_B_FW_PRE,
	IWL_DEVICE_22500,
	.num_rbds = IWL_NUM_RBDS_AX210_HE,
};

MODULE_FIRMWARE(IWL_QU_B_HR_B_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_QNJ_B_HR_B_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_QU_C_HR_B_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_QU_B_JF_B_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_BZ_A_GF_A_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_BZ_A_GF4_A_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));
MODULE_FIRMWARE(IWL_BZ_A_MR_A_MODULE_FIRMWARE(IWL_22000_UCODE_API_MAX));