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
	u32 dump_mask;
};

struct iwl_dump_exclude {
	u32 addr, size;
};

/**
 * struct iwl_fw - variables associated with the firmware
 *
 * @ucode_ver: ucode version from the ucode file
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


	u8 *phy_integration_ver;
	u32 phy_integration_ver_len;

	struct iwl_dump_exclude dump_excl[2], dump_excl_wowlan[2];
};

static inline const char *get_fw_dbg_mode_string(int mode)
{