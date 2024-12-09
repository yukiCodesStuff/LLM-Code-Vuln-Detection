#define HCMD_ARR(x)	\
	{ .arr = x, .size = ARRAY_SIZE(x) }

/**
 * struct iwl_dump_sanitize_ops - dump sanitization operations
 * @frob_txf: Scrub the TX FIFO data
 * @frob_hcmd: Scrub a host command, the %hcmd pointer is to the header
 *	but that might be short or long (&struct iwl_cmd_header or
 *	&struct iwl_cmd_header_wide)
 * @frob_mem: Scrub memory data
 */
struct iwl_dump_sanitize_ops {
	void (*frob_txf)(void *ctx, void *buf, size_t buflen);
	void (*frob_hcmd)(void *ctx, void *hcmd, size_t buflen);
	void (*frob_mem)(void *ctx, u32 mem_addr, void *mem, size_t buflen);
};

/**
 * struct iwl_trans_config - transport configuration
 *
 * @op_mode: pointer to the upper layer.
			      u32 value);

	struct iwl_trans_dump_data *(*dump_data)(struct iwl_trans *trans,
						 u32 dump_mask,
						 const struct iwl_dump_sanitize_ops *sanitize_ops,
						 void *sanitize_ctx);
	void (*debugfs_cleanup)(struct iwl_trans *trans);
	void (*sync_nmi)(struct iwl_trans *trans);
	int (*set_pnvm)(struct iwl_trans *trans, const void *data, u32 len);
	int (*set_reduce_power)(struct iwl_trans *trans,
 * @debug_info_tlv_list: list of debug info TLVs
 * @time_point: array of debug time points
 * @periodic_trig_list: periodic triggers list
 * @domains_bitmap: bitmap of active domains other than &IWL_FW_INI_DOMAIN_ALWAYS_ON
 * @ucode_preset: preset based on ucode
 */
struct iwl_trans_debug {
	u8 n_dest_reg;
	bool rec_on;
	struct list_head periodic_trig_list;

	u32 domains_bitmap;
	u32 ucode_preset;
};

struct iwl_dma_ptr {
	dma_addr_t dma;
}

static inline struct iwl_trans_dump_data *
iwl_trans_dump_data(struct iwl_trans *trans, u32 dump_mask,
		    const struct iwl_dump_sanitize_ops *sanitize_ops,
		    void *sanitize_ctx)
{
	if (!trans->ops->dump_data)
		return NULL;
	return trans->ops->dump_data(trans, dump_mask,
				     sanitize_ops, sanitize_ctx);
}

static inline struct iwl_device_tx_cmd *
iwl_trans_alloc_tx_cmd(struct iwl_trans *trans)