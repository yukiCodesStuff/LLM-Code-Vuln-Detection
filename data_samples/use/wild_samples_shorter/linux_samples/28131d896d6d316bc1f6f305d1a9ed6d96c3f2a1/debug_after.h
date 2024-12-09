	struct iwl_buf_alloc_frag frags[BUF_ALLOC_MAX_NUM_FRAGS];
} __packed; /* BUFFER_ALLOCATION_CMD_API_S_VER_2 */

#define DRAM_INFO_FIRST_MAGIC_WORD 0x76543210
#define DRAM_INFO_SECOND_MAGIC_WORD 0x89ABCDEF

/**
 * struct iwL_dram_info - DRAM fragments allocation struct
 *
 * Driver will fill in the first 1K(+) of the pointed DRAM fragment
 *
 * @first_word: magic word value
 * @second_word: magic word value
 * @framfrags: DRAM fragmentaion detail
*/
struct iwl_dram_info {
	__le32 first_word;
	__le32 second_word;
	struct iwl_buf_alloc_cmd dram_frags[IWL_FW_INI_ALLOCATION_NUM - 1];
} __packed; /* INIT_DRAM_FRAGS_ALLOCATIONS_S_VER_1 */

/**
 * struct iwl_dbgc1_info - DBGC1 address and size
 *
 * Driver will fill the dbcg1 address and size at address based on config TLV.
 *
 * @first_word: all 0 set as identifier
 * @dbgc1_add_lsb: LSB bits of DBGC1 physical address
 * @dbgc1_add_msb: MSB bits of DBGC1 physical address
 * @dbgc1_size: DBGC1 size
*/
struct iwl_dbgc1_info {
	__le32 first_word;
	__le32 dbgc1_add_lsb;
	__le32 dbgc1_add_msb;
	__le32 dbgc1_size;
} __packed; /* INIT_DRAM_FRAGS_ALLOCATIONS_S_VER_1 */

/**
 * struct iwl_dbg_host_event_cfg_cmd
 * @enabled_severities: enabled severities
 */