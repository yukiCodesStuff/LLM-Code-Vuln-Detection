#define IWL_AX210_HW_TYPE 0x42
/* How many bits to roll when adding to the HW type of AX210 HW */
#define IWL_AX210_HW_TYPE_ADDITION_SHIFT 12
/* This prph is used to tell apart HW_TYPE == 0x42 NICs */
#define WFPM_OTP_CFG1_ADDR 0xd03098
#define WFPM_OTP_CFG1_IS_JACKET_BIT BIT(4)
#define WFPM_OTP_CFG1_IS_CDB_BIT BIT(5)

/* struct iwl_fw_ini_dump_info - ini dump information
 * @version: dump version
 * @time_point: time point that caused the dump collection