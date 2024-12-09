 * @fw_paging_phys: page phy pointer
 * @fw_paging_block: pointer to the allocated block
 * @fw_paging_size: page size
 */
struct iwl_fw_paging {
	dma_addr_t fw_paging_phys;
	struct page *fw_paging_block;
	u32 fw_paging_size;
};

/**
 * struct iwl_fw_cscheme_list - a cipher scheme list
	u32 dump_mask;
};

/**
 * struct iwl_fw - variables associated with the firmware
 *
 * @ucode_ver: ucode version from the ucode file
 * @cipher_scheme: optional external cipher scheme.
 * @human_readable: human readable version
 *	we get the ALIVE from the uCode
 */
struct iwl_fw {
	u32 ucode_ver;


	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;
};

static inline const char *get_fw_dbg_mode_string(int mode)
{