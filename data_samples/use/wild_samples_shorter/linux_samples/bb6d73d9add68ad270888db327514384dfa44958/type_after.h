	bool remote_access:1;
	bool use_hmc_fcn_index:1;
	bool use_pf_rid:1;
	bool all_memory:1;
	u8 hmc_fcn_index;
};

struct irdma_mw_alloc_info {
	bool use_hmc_fcn_index:1;
	u8 hmc_fcn_index;
	bool use_pf_rid:1;
	bool all_memory:1;
};

struct irdma_fast_reg_stag_info {
	u64 wr_id;