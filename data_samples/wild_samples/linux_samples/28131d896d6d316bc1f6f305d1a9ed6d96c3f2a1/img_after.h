struct fw_img {
	struct fw_desc *sec;
	int num_sec;
	bool is_dual_cpus;
	u32 paging_mem_size;
};

/*
 * Block paging calculations
 */
#define PAGE_2_EXP_SIZE 12 /* 4K == 2^12 */
#define FW_PAGING_SIZE BIT(PAGE_2_EXP_SIZE) /* page size is 4KB */
#define PAGE_PER_GROUP_2_EXP_SIZE 3
/* 8 pages per group */
#define NUM_OF_PAGE_PER_GROUP BIT(PAGE_PER_GROUP_2_EXP_SIZE)
/* don't change, support only 32KB size */
#define PAGING_BLOCK_SIZE (NUM_OF_PAGE_PER_GROUP * FW_PAGING_SIZE)
/* 32K == 2^15 */
#define BLOCK_2_EXP_SIZE (PAGE_2_EXP_SIZE + PAGE_PER_GROUP_2_EXP_SIZE)

/*
 * Image paging calculations
 */
#define BLOCK_PER_IMAGE_2_EXP_SIZE 5
/* 2^5 == 32 blocks per image */
#define NUM_OF_BLOCK_PER_IMAGE BIT(BLOCK_PER_IMAGE_2_EXP_SIZE)
/* maximum image size 1024KB */
#define MAX_PAGING_IMAGE_SIZE (NUM_OF_BLOCK_PER_IMAGE * PAGING_BLOCK_SIZE)

/* Virtual address signature */
#define PAGING_ADDR_SIG 0xAA000000

#define PAGING_CMD_IS_SECURED BIT(9)
#define PAGING_CMD_IS_ENABLED BIT(8)
#define PAGING_CMD_NUM_OF_PAGES_IN_LAST_GRP_POS	0
#define PAGING_TLV_SECURE_MASK 1

/* FW MSB Mask for regions/cache_control */
#define FW_ADDR_CACHE_CONTROL 0xC0000000UL

/**
 * struct iwl_fw_paging
 * @fw_paging_phys: page phy pointer
 * @fw_paging_block: pointer to the allocated block
 * @fw_paging_size: page size
 * @fw_offs: offset in the device
 */
struct iwl_fw_paging {
	dma_addr_t fw_paging_phys;
	struct page *fw_paging_block;
	u32 fw_paging_size;
	u32 fw_offs;
};

/**
 * struct iwl_fw_cscheme_list - a cipher scheme list
 * @size: a number of entries
 * @cs: cipher scheme entries
 */
struct iwl_fw_cscheme_list {
	u8 size;
	struct iwl_fw_cipher_scheme cs[];
} __packed;

/**
 * enum iwl_fw_type - iwlwifi firmware type
 * @IWL_FW_DVM: DVM firmware
 * @IWL_FW_MVM: MVM firmware
 */
enum iwl_fw_type {
	IWL_FW_DVM,
	IWL_FW_MVM,
};

/**
 * struct iwl_fw_dbg - debug data
 *
 * @dest_tlv: points to debug destination TLV (typically SRAM or DRAM)
 * @n_dest_reg: num of reg_ops in dest_tlv
 * @conf_tlv: array of pointers to configuration HCMDs
 * @trigger_tlv: array of pointers to triggers TLVs
 * @trigger_tlv_len: lengths of the @dbg_trigger_tlv entries
 * @mem_tlv: Runtime addresses to dump
 * @n_mem_tlv: number of runtime addresses
 * @dump_mask: bitmask of dump regions
*/
struct iwl_fw_dbg {
	struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	u8 n_dest_reg;
	struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv *trigger_tlv[FW_DBG_TRIGGER_MAX];
	size_t trigger_tlv_len[FW_DBG_TRIGGER_MAX];
	struct iwl_fw_dbg_mem_seg_tlv *mem_tlv;
	size_t n_mem_tlv;
	u32 dump_mask;
};

struct iwl_dump_exclude {
	u32 addr, size;
};

/**
 * struct iwl_fw - variables associated with the firmware
 *
 * @ucode_ver: ucode version from the ucode file
 * @fw_version: firmware version string
 * @img: ucode image like ucode_rt, ucode_init, ucode_wowlan.
 * @iml_len: length of the image loader image
 * @iml: image loader fw image
 * @ucode_capa: capabilities parsed from the ucode file.
 * @enhance_sensitivity_table: device can do enhanced sensitivity.
 * @init_evtlog_ptr: event log offset for init ucode.
 * @init_evtlog_size: event log size for init ucode.
 * @init_errlog_ptr: error log offfset for init ucode.
 * @inst_evtlog_ptr: event log offset for runtime ucode.
 * @inst_evtlog_size: event log size for runtime ucode.
 * @inst_errlog_ptr: error log offfset for runtime ucode.
 * @type: firmware type (&enum iwl_fw_type)
 * @cipher_scheme: optional external cipher scheme.
 * @human_readable: human readable version
 *	we get the ALIVE from the uCode
 * @phy_integration_ver: PHY integration version string
 * @phy_integration_ver_len: length of @phy_integration_ver
 * @dump_excl: image dump exclusion areas for RT image
 * @dump_excl_wowlan: image dump exclusion areas for WoWLAN image
 */
struct iwl_fw {
	u32 ucode_ver;

	char fw_version[64];

	/* ucode images */
	struct fw_img img[IWL_UCODE_TYPE_MAX];
	size_t iml_len;
	u8 *iml;

	struct iwl_ucode_capabilities ucode_capa;
	bool enhance_sensitivity_table;

	u32 init_evtlog_ptr, init_evtlog_size, init_errlog_ptr;
	u32 inst_evtlog_ptr, inst_evtlog_size, inst_errlog_ptr;

	struct iwl_tlv_calib_ctrl default_calib[IWL_UCODE_TYPE_MAX];
	u32 phy_config;
	u8 valid_tx_ant;
	u8 valid_rx_ant;

	enum iwl_fw_type type;

	struct iwl_fw_cipher_scheme cs[IWL_UCODE_MAX_CS];
	u8 human_readable[FW_VER_HUMAN_READABLE_SZ];

	struct iwl_fw_dbg dbg;

	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;

	struct iwl_dump_exclude dump_excl[2], dump_excl_wowlan[2];
};

static inline const char *get_fw_dbg_mode_string(int mode)
{
	switch (mode) {
	case SMEM_MODE:
		return "SMEM";
	case EXTERNAL_MODE:
		return "EXTERNAL_DRAM";
	case MARBH_MODE:
		return "MARBH";
	case MIPI_MODE:
		return "MIPI";
	default:
		return "UNKNOWN";
	}
}

static inline bool
iwl_fw_dbg_conf_usniffer(const struct iwl_fw *fw, u8 id)
{
	const struct iwl_fw_dbg_conf_tlv *conf_tlv = fw->dbg.conf_tlv[id];

	if (!conf_tlv)
		return false;

	return conf_tlv->usniffer;
}

static inline const struct fw_img *
iwl_get_ucode_image(const struct iwl_fw *fw, enum iwl_ucode_type ucode_type)
{
	if (ucode_type >= IWL_UCODE_TYPE_MAX)
		return NULL;

	return &fw->img[ucode_type];
}

u8 iwl_fw_lookup_cmd_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);

u8 iwl_fw_lookup_notif_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);
const char *iwl_fw_lookup_assert_desc(u32 num);
#endif  /* __iwl_fw_img_h__ */
struct iwl_fw_dbg {
	struct iwl_fw_dbg_dest_tlv_v1 *dest_tlv;
	u8 n_dest_reg;
	struct iwl_fw_dbg_conf_tlv *conf_tlv[FW_DBG_CONF_MAX];
	struct iwl_fw_dbg_trigger_tlv *trigger_tlv[FW_DBG_TRIGGER_MAX];
	size_t trigger_tlv_len[FW_DBG_TRIGGER_MAX];
	struct iwl_fw_dbg_mem_seg_tlv *mem_tlv;
	size_t n_mem_tlv;
	u32 dump_mask;
};

struct iwl_dump_exclude {
	u32 addr, size;
};

/**
 * struct iwl_fw - variables associated with the firmware
 *
 * @ucode_ver: ucode version from the ucode file
 * @fw_version: firmware version string
 * @img: ucode image like ucode_rt, ucode_init, ucode_wowlan.
 * @iml_len: length of the image loader image
 * @iml: image loader fw image
 * @ucode_capa: capabilities parsed from the ucode file.
 * @enhance_sensitivity_table: device can do enhanced sensitivity.
 * @init_evtlog_ptr: event log offset for init ucode.
 * @init_evtlog_size: event log size for init ucode.
 * @init_errlog_ptr: error log offfset for init ucode.
 * @inst_evtlog_ptr: event log offset for runtime ucode.
 * @inst_evtlog_size: event log size for runtime ucode.
 * @inst_errlog_ptr: error log offfset for runtime ucode.
 * @type: firmware type (&enum iwl_fw_type)
 * @cipher_scheme: optional external cipher scheme.
 * @human_readable: human readable version
 *	we get the ALIVE from the uCode
 * @phy_integration_ver: PHY integration version string
 * @phy_integration_ver_len: length of @phy_integration_ver
 * @dump_excl: image dump exclusion areas for RT image
 * @dump_excl_wowlan: image dump exclusion areas for WoWLAN image
 */
struct iwl_fw {
	u32 ucode_ver;

	char fw_version[64];

	/* ucode images */
	struct fw_img img[IWL_UCODE_TYPE_MAX];
	size_t iml_len;
	u8 *iml;

	struct iwl_ucode_capabilities ucode_capa;
	bool enhance_sensitivity_table;

	u32 init_evtlog_ptr, init_evtlog_size, init_errlog_ptr;
	u32 inst_evtlog_ptr, inst_evtlog_size, inst_errlog_ptr;

	struct iwl_tlv_calib_ctrl default_calib[IWL_UCODE_TYPE_MAX];
	u32 phy_config;
	u8 valid_tx_ant;
	u8 valid_rx_ant;

	enum iwl_fw_type type;

	struct iwl_fw_cipher_scheme cs[IWL_UCODE_MAX_CS];
	u8 human_readable[FW_VER_HUMAN_READABLE_SZ];

	struct iwl_fw_dbg dbg;

	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;

	struct iwl_dump_exclude dump_excl[2], dump_excl_wowlan[2];
};

static inline const char *get_fw_dbg_mode_string(int mode)
{
	switch (mode) {
	case SMEM_MODE:
		return "SMEM";
	case EXTERNAL_MODE:
		return "EXTERNAL_DRAM";
	case MARBH_MODE:
		return "MARBH";
	case MIPI_MODE:
		return "MIPI";
	default:
		return "UNKNOWN";
	}
}

static inline bool
iwl_fw_dbg_conf_usniffer(const struct iwl_fw *fw, u8 id)
{
	const struct iwl_fw_dbg_conf_tlv *conf_tlv = fw->dbg.conf_tlv[id];

	if (!conf_tlv)
		return false;

	return conf_tlv->usniffer;
}

static inline const struct fw_img *
iwl_get_ucode_image(const struct iwl_fw *fw, enum iwl_ucode_type ucode_type)
{
	if (ucode_type >= IWL_UCODE_TYPE_MAX)
		return NULL;

	return &fw->img[ucode_type];
}

u8 iwl_fw_lookup_cmd_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);

u8 iwl_fw_lookup_notif_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);
const char *iwl_fw_lookup_assert_desc(u32 num);
#endif  /* __iwl_fw_img_h__ */
struct iwl_dump_exclude {
	u32 addr, size;
};

/**
 * struct iwl_fw - variables associated with the firmware
 *
 * @ucode_ver: ucode version from the ucode file
 * @fw_version: firmware version string
 * @img: ucode image like ucode_rt, ucode_init, ucode_wowlan.
 * @iml_len: length of the image loader image
 * @iml: image loader fw image
 * @ucode_capa: capabilities parsed from the ucode file.
 * @enhance_sensitivity_table: device can do enhanced sensitivity.
 * @init_evtlog_ptr: event log offset for init ucode.
 * @init_evtlog_size: event log size for init ucode.
 * @init_errlog_ptr: error log offfset for init ucode.
 * @inst_evtlog_ptr: event log offset for runtime ucode.
 * @inst_evtlog_size: event log size for runtime ucode.
 * @inst_errlog_ptr: error log offfset for runtime ucode.
 * @type: firmware type (&enum iwl_fw_type)
 * @cipher_scheme: optional external cipher scheme.
 * @human_readable: human readable version
 *	we get the ALIVE from the uCode
 * @phy_integration_ver: PHY integration version string
 * @phy_integration_ver_len: length of @phy_integration_ver
 * @dump_excl: image dump exclusion areas for RT image
 * @dump_excl_wowlan: image dump exclusion areas for WoWLAN image
 */
struct iwl_fw {
	u32 ucode_ver;

	char fw_version[64];

	/* ucode images */
	struct fw_img img[IWL_UCODE_TYPE_MAX];
	size_t iml_len;
	u8 *iml;

	struct iwl_ucode_capabilities ucode_capa;
	bool enhance_sensitivity_table;

	u32 init_evtlog_ptr, init_evtlog_size, init_errlog_ptr;
	u32 inst_evtlog_ptr, inst_evtlog_size, inst_errlog_ptr;

	struct iwl_tlv_calib_ctrl default_calib[IWL_UCODE_TYPE_MAX];
	u32 phy_config;
	u8 valid_tx_ant;
	u8 valid_rx_ant;

	enum iwl_fw_type type;

	struct iwl_fw_cipher_scheme cs[IWL_UCODE_MAX_CS];
	u8 human_readable[FW_VER_HUMAN_READABLE_SZ];

	struct iwl_fw_dbg dbg;

	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;

	struct iwl_dump_exclude dump_excl[2], dump_excl_wowlan[2];
};

static inline const char *get_fw_dbg_mode_string(int mode)
{
	switch (mode) {
	case SMEM_MODE:
		return "SMEM";
	case EXTERNAL_MODE:
		return "EXTERNAL_DRAM";
	case MARBH_MODE:
		return "MARBH";
	case MIPI_MODE:
		return "MIPI";
	default:
		return "UNKNOWN";
	}
}

static inline bool
iwl_fw_dbg_conf_usniffer(const struct iwl_fw *fw, u8 id)
{
	const struct iwl_fw_dbg_conf_tlv *conf_tlv = fw->dbg.conf_tlv[id];

	if (!conf_tlv)
		return false;

	return conf_tlv->usniffer;
}

static inline const struct fw_img *
iwl_get_ucode_image(const struct iwl_fw *fw, enum iwl_ucode_type ucode_type)
{
	if (ucode_type >= IWL_UCODE_TYPE_MAX)
		return NULL;

	return &fw->img[ucode_type];
}

u8 iwl_fw_lookup_cmd_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);

u8 iwl_fw_lookup_notif_ver(const struct iwl_fw *fw, u8 grp, u8 cmd, u8 def);
const char *iwl_fw_lookup_assert_desc(u32 num);
#endif  /* __iwl_fw_img_h__ */
struct iwl_fw {
	u32 ucode_ver;

	char fw_version[64];

	/* ucode images */
	struct fw_img img[IWL_UCODE_TYPE_MAX];
	size_t iml_len;
	u8 *iml;

	struct iwl_ucode_capabilities ucode_capa;
	bool enhance_sensitivity_table;

	u32 init_evtlog_ptr, init_evtlog_size, init_errlog_ptr;
	u32 inst_evtlog_ptr, inst_evtlog_size, inst_errlog_ptr;

	struct iwl_tlv_calib_ctrl default_calib[IWL_UCODE_TYPE_MAX];
	u32 phy_config;
	u8 valid_tx_ant;
	u8 valid_rx_ant;

	enum iwl_fw_type type;

	struct iwl_fw_cipher_scheme cs[IWL_UCODE_MAX_CS];
	u8 human_readable[FW_VER_HUMAN_READABLE_SZ];

	struct iwl_fw_dbg dbg;

	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;

	struct iwl_dump_exclude dump_excl[2], dump_excl_wowlan[2];
};

static inline const char *get_fw_dbg_mode_string(int mode)
{
	switch (mode) {
	case SMEM_MODE:
		return "SMEM";
	case EXTERNAL_MODE:
		return "EXTERNAL_DRAM";
	case MARBH_MODE:
		return "MARBH";
	case MIPI_MODE:
		return "MIPI";
	default:
		return "UNKNOWN";
	}
}