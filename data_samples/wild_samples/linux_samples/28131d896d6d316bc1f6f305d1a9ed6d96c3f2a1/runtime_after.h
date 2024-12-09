struct iwl_fw_runtime {
	struct iwl_trans *trans;
	const struct iwl_fw *fw;
	struct device *dev;

	const struct iwl_fw_runtime_ops *ops;
	void *ops_ctx;

	const struct iwl_dump_sanitize_ops *sanitize_ops;
	void *sanitize_ctx;

	/* Paging */
	struct iwl_fw_paging fw_paging_db[NUM_OF_FW_PAGING_BLOCKS];
	u16 num_of_paging_blk;
	u16 num_of_pages_in_last_blk;

	enum iwl_ucode_type cur_fw_img;

	/* memory configuration */
	struct iwl_fwrt_shared_mem_cfg smem_cfg;

	/* debug */
	struct {
		struct iwl_fwrt_wk_data wks[IWL_FW_RUNTIME_DUMP_WK_NUM];
		unsigned long active_wks;

		u8 conf;

		/* ts of the beginning of a non-collect fw dbg data period */
		unsigned long non_collect_ts_start[IWL_FW_INI_TIME_POINT_NUM];
		u32 *d3_debug_data;
		u32 lmac_err_id[MAX_NUM_LMAC];
		u32 umac_err_id;

		struct iwl_txf_iter_data txf_iter_data;

		struct {
			u8 type;
			u8 subtype;
			u32 lmac_major;
			u32 lmac_minor;
			u32 umac_major;
			u32 umac_minor;
		} fw_ver;
	} dump;
#ifdef CONFIG_IWLWIFI_DEBUGFS
	struct {
		struct delayed_work wk;
		u32 delay;
		u64 seq;
	} timestamp;
	bool tpc_enabled;
#endif /* CONFIG_IWLWIFI_DEBUGFS */
#ifdef CONFIG_ACPI
	struct iwl_sar_profile sar_profiles[ACPI_SAR_PROFILE_NUM];
	u8 sar_chain_a_profile;
	u8 sar_chain_b_profile;
	struct iwl_geo_profile geo_profiles[ACPI_NUM_GEO_PROFILES_REV3];
	u32 geo_rev;
	union iwl_ppag_table_cmd ppag_table;
	u32 ppag_ver;
#endif
};

void iwl_fw_runtime_init(struct iwl_fw_runtime *fwrt, struct iwl_trans *trans,
			const struct iwl_fw *fw,
			const struct iwl_fw_runtime_ops *ops, void *ops_ctx,
			const struct iwl_dump_sanitize_ops *sanitize_ops,
			void *sanitize_ctx,
			struct dentry *dbgfs_dir);

static inline void iwl_fw_runtime_free(struct iwl_fw_runtime *fwrt)
{
	int i;

	kfree(fwrt->dump.d3_debug_data);
	fwrt->dump.d3_debug_data = NULL;

	iwl_dbg_tlv_del_timers(fwrt->trans);
	for (i = 0; i < IWL_FW_RUNTIME_DUMP_WK_NUM; i++)
		cancel_delayed_work_sync(&fwrt->dump.wks[i].wk);
}

void iwl_fw_runtime_suspend(struct iwl_fw_runtime *fwrt);

void iwl_fw_runtime_resume(struct iwl_fw_runtime *fwrt);

static inline void iwl_fw_set_current_image(struct iwl_fw_runtime *fwrt,
					    enum iwl_ucode_type cur_fw_img)
{
	fwrt->cur_fw_img = cur_fw_img;
}

int iwl_init_paging(struct iwl_fw_runtime *fwrt, enum iwl_ucode_type type);
void iwl_free_fw_paging(struct iwl_fw_runtime *fwrt);

void iwl_get_shared_mem_conf(struct iwl_fw_runtime *fwrt);
int iwl_set_soc_latency(struct iwl_fw_runtime *fwrt);
int iwl_configure_rxq(struct iwl_fw_runtime *fwrt);

#endif /* __iwl_fw_runtime_h__ */
	struct {
		struct delayed_work wk;
		u32 delay;
		u64 seq;
	} timestamp;
	bool tpc_enabled;
#endif /* CONFIG_IWLWIFI_DEBUGFS */
#ifdef CONFIG_ACPI
	struct iwl_sar_profile sar_profiles[ACPI_SAR_PROFILE_NUM];
	u8 sar_chain_a_profile;
	u8 sar_chain_b_profile;
	struct iwl_geo_profile geo_profiles[ACPI_NUM_GEO_PROFILES_REV3];
	u32 geo_rev;
	union iwl_ppag_table_cmd ppag_table;
	u32 ppag_ver;
#endif
};

void iwl_fw_runtime_init(struct iwl_fw_runtime *fwrt, struct iwl_trans *trans,
			const struct iwl_fw *fw,
			const struct iwl_fw_runtime_ops *ops, void *ops_ctx,
			const struct iwl_dump_sanitize_ops *sanitize_ops,
			void *sanitize_ctx,
			struct dentry *dbgfs_dir);

static inline void iwl_fw_runtime_free(struct iwl_fw_runtime *fwrt)
{
	int i;

	kfree(fwrt->dump.d3_debug_data);
	fwrt->dump.d3_debug_data = NULL;

	iwl_dbg_tlv_del_timers(fwrt->trans);
	for (i = 0; i < IWL_FW_RUNTIME_DUMP_WK_NUM; i++)
		cancel_delayed_work_sync(&fwrt->dump.wks[i].wk);
}
	struct {
		struct delayed_work wk;
		u32 delay;
		u64 seq;
	} timestamp;
	bool tpc_enabled;
#endif /* CONFIG_IWLWIFI_DEBUGFS */
#ifdef CONFIG_ACPI
	struct iwl_sar_profile sar_profiles[ACPI_SAR_PROFILE_NUM];
	u8 sar_chain_a_profile;
	u8 sar_chain_b_profile;
	struct iwl_geo_profile geo_profiles[ACPI_NUM_GEO_PROFILES_REV3];
	u32 geo_rev;
	union iwl_ppag_table_cmd ppag_table;
	u32 ppag_ver;
#endif
};

void iwl_fw_runtime_init(struct iwl_fw_runtime *fwrt, struct iwl_trans *trans,
			const struct iwl_fw *fw,
			const struct iwl_fw_runtime_ops *ops, void *ops_ctx,
			const struct iwl_dump_sanitize_ops *sanitize_ops,
			void *sanitize_ctx,
			struct dentry *dbgfs_dir);

static inline void iwl_fw_runtime_free(struct iwl_fw_runtime *fwrt)
{
	int i;

	kfree(fwrt->dump.d3_debug_data);
	fwrt->dump.d3_debug_data = NULL;

	iwl_dbg_tlv_del_timers(fwrt->trans);
	for (i = 0; i < IWL_FW_RUNTIME_DUMP_WK_NUM; i++)
		cancel_delayed_work_sync(&fwrt->dump.wks[i].wk);
}