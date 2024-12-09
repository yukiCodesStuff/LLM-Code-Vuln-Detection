struct irdma_allocate_stag_info {
	u64 total_len;
	u64 first_pm_pbl_idx;
	u32 chunk_size;
	u32 stag_idx;
	u32 page_size;
	u32 pd_id;
	u16 access_rights;
	bool remote_access:1;
	bool use_hmc_fcn_index:1;
	bool use_pf_rid:1;
	bool all_memory:1;
	u8 hmc_fcn_index;
};

struct irdma_mw_alloc_info {
	u32 mw_stag_index;
	u32 page_size;
	u32 pd_id;
	bool remote_access:1;
	bool mw_wide:1;
	bool mw1_bind_dont_vldt_key:1;
};

struct irdma_reg_ns_stag_info {
	u64 reg_addr_pa;
	u64 va;
	u64 total_len;
	u32 page_size;
	u32 chunk_size;
	u32 first_pm_pbl_index;
	enum irdma_addressing_type addr_type;
	irdma_stag_index stag_idx;
	u16 access_rights;
	u32 pd_id;
	irdma_stag_key stag_key;
	bool use_hmc_fcn_index:1;
	u8 hmc_fcn_index;
	bool use_pf_rid:1;
	bool all_memory:1;
};

struct irdma_fast_reg_stag_info {
	u64 wr_id;
	u64 reg_addr_pa;
	u64 fbo;
	void *va;
	u64 total_len;
	u32 page_size;
	u32 chunk_size;
	u32 first_pm_pbl_index;
	enum irdma_addressing_type addr_type;
	irdma_stag_index stag_idx;
	u16 access_rights;
	u32 pd_id;
	irdma_stag_key stag_key;
	bool local_fence:1;
	bool read_fence:1;
	bool signaled:1;
	bool use_hmc_fcn_index:1;
	u8 hmc_fcn_index;
	bool use_pf_rid:1;
	bool defer_flag:1;
};

struct irdma_dealloc_stag_info {
	u32 stag_idx;
	u32 pd_id;
	bool mr:1;
	bool dealloc_pbl:1;
};

struct irdma_register_shared_stag {
	u64 va;
	enum irdma_addressing_type addr_type;
	irdma_stag_index new_stag_idx;
	irdma_stag_index parent_stag_idx;
	u32 access_rights;
	u32 pd_id;
	u32 page_size;
	irdma_stag_key new_stag_key;
};

struct irdma_qp_init_info {
	struct irdma_qp_uk_init_info qp_uk_init_info;
	struct irdma_sc_pd *pd;
	struct irdma_sc_vsi *vsi;
	__le64 *host_ctx;
	u8 *q2;
	u64 sq_pa;
	u64 rq_pa;
	u64 host_ctx_pa;
	u64 q2_pa;
	u64 shadow_area_pa;
	u8 sq_tph_val;
	u8 rq_tph_val;
	bool sq_tph_en:1;
	bool rq_tph_en:1;
	bool rcv_tph_en:1;
	bool xmit_tph_en:1;
	bool virtual_map:1;
};

struct irdma_cq_init_info {
	struct irdma_sc_dev *dev;
	u64 cq_base_pa;
	u64 shadow_area_pa;
	u32 ceq_id;
	u32 shadow_read_threshold;
	u8 pbl_chunk_size;
	u32 first_pm_pbl_idx;
	bool virtual_map:1;
	bool ceqe_mask:1;
	bool ceq_id_valid:1;
	bool tph_en:1;
	u8 tph_val;
	u8 type;
	struct irdma_cq_uk_init_info cq_uk_init_info;
	struct irdma_sc_vsi *vsi;
};

struct irdma_upload_context_info {
	u64 buf_pa;
	u32 qp_id;
	u8 qp_type;
	bool freeze_qp:1;
	bool raw_format:1;
};

struct irdma_local_mac_entry_info {
	u8 mac_addr[6];
	u16 entry_idx;
};

struct irdma_add_arp_cache_entry_info {
	u8 mac_addr[ETH_ALEN];
	u32 reach_max;
	u16 arp_index;
	bool permanent;
};

struct irdma_apbvt_info {
	u16 port;
	bool add;
};

struct irdma_qhash_table_info {
	struct irdma_sc_vsi *vsi;
	enum irdma_quad_hash_manage_type manage;
	enum irdma_quad_entry_type entry_type;
	bool vlan_valid:1;
	bool ipv4_valid:1;
	u8 mac_addr[ETH_ALEN];
	u16 vlan_id;
	u8 user_pri;
	u32 qp_num;
	u32 dest_ip[4];
	u32 src_ip[4];
	u16 dest_port;
	u16 src_port;
};

struct irdma_cqp_manage_push_page_info {
	u32 push_idx;
	u16 qs_handle;
	u8 free_page;
	u8 push_page_type;
};

struct irdma_qp_flush_info {
	u16 sq_minor_code;
	u16 sq_major_code;
	u16 rq_minor_code;
	u16 rq_major_code;
	u16 ae_code;
	u8 ae_src;
	bool sq:1;
	bool rq:1;
	bool userflushcode:1;
	bool generate_ae:1;
};

struct irdma_gen_ae_info {
	u16 ae_code;
	u8 ae_src;
};

struct irdma_cqp_timeout {
	u64 compl_cqp_cmds;
	u32 count;
};

struct irdma_irq_ops {
	void (*irdma_cfg_aeq)(struct irdma_sc_dev *dev, u32 idx, bool enable);
	void (*irdma_cfg_ceq)(struct irdma_sc_dev *dev, u32 ceq_id, u32 idx,
			      bool enable);
	void (*irdma_dis_irq)(struct irdma_sc_dev *dev, u32 idx);
	void (*irdma_en_irq)(struct irdma_sc_dev *dev, u32 idx);
};

void irdma_sc_ccq_arm(struct irdma_sc_cq *ccq);
int irdma_sc_ccq_create(struct irdma_sc_cq *ccq, u64 scratch,
			bool check_overflow, bool post_sq);
int irdma_sc_ccq_destroy(struct irdma_sc_cq *ccq, u64 scratch, bool post_sq);
int irdma_sc_ccq_get_cqe_info(struct irdma_sc_cq *ccq,
			      struct irdma_ccq_cqe_info *info);
int irdma_sc_ccq_init(struct irdma_sc_cq *ccq,
		      struct irdma_ccq_init_info *info);

int irdma_sc_cceq_create(struct irdma_sc_ceq *ceq, u64 scratch);
int irdma_sc_cceq_destroy_done(struct irdma_sc_ceq *ceq);

int irdma_sc_ceq_destroy(struct irdma_sc_ceq *ceq, u64 scratch, bool post_sq);
int irdma_sc_ceq_init(struct irdma_sc_ceq *ceq,
		      struct irdma_ceq_init_info *info);
void irdma_sc_cleanup_ceqes(struct irdma_sc_cq *cq, struct irdma_sc_ceq *ceq);
void *irdma_sc_process_ceq(struct irdma_sc_dev *dev, struct irdma_sc_ceq *ceq);

int irdma_sc_aeq_init(struct irdma_sc_aeq *aeq,
		      struct irdma_aeq_init_info *info);
int irdma_sc_get_next_aeqe(struct irdma_sc_aeq *aeq,
			   struct irdma_aeqe_info *info);
void irdma_sc_repost_aeq_entries(struct irdma_sc_dev *dev, u32 count);

void irdma_sc_pd_init(struct irdma_sc_dev *dev, struct irdma_sc_pd *pd, u32 pd_id,
		      int abi_ver);
void irdma_cfg_aeq(struct irdma_sc_dev *dev, u32 idx, bool enable);
void irdma_check_cqp_progress(struct irdma_cqp_timeout *cqp_timeout,
			      struct irdma_sc_dev *dev);
int irdma_sc_cqp_create(struct irdma_sc_cqp *cqp, u16 *maj_err, u16 *min_err);
int irdma_sc_cqp_destroy(struct irdma_sc_cqp *cqp);
int irdma_sc_cqp_init(struct irdma_sc_cqp *cqp,
		      struct irdma_cqp_init_info *info);
void irdma_sc_cqp_post_sq(struct irdma_sc_cqp *cqp);
int irdma_sc_poll_for_cqp_op_done(struct irdma_sc_cqp *cqp, u8 opcode,
				  struct irdma_ccq_cqe_info *cmpl_info);
int irdma_sc_fast_register(struct irdma_sc_qp *qp,
			   struct irdma_fast_reg_stag_info *info, bool post_sq);
int irdma_sc_qp_create(struct irdma_sc_qp *qp,
		       struct irdma_create_qp_info *info, u64 scratch,
		       bool post_sq);
int irdma_sc_qp_destroy(struct irdma_sc_qp *qp, u64 scratch,
			bool remove_hash_idx, bool ignore_mw_bnd, bool post_sq);
int irdma_sc_qp_flush_wqes(struct irdma_sc_qp *qp,
			   struct irdma_qp_flush_info *info, u64 scratch,
			   bool post_sq);
int irdma_sc_qp_init(struct irdma_sc_qp *qp, struct irdma_qp_init_info *info);
int irdma_sc_qp_modify(struct irdma_sc_qp *qp,
		       struct irdma_modify_qp_info *info, u64 scratch,
		       bool post_sq);
void irdma_sc_send_lsmm(struct irdma_sc_qp *qp, void *lsmm_buf, u32 size,
			irdma_stag stag);

void irdma_sc_send_rtt(struct irdma_sc_qp *qp, bool read);
void irdma_sc_qp_setctx(struct irdma_sc_qp *qp, __le64 *qp_ctx,
			struct irdma_qp_host_ctx_info *info);
void irdma_sc_qp_setctx_roce(struct irdma_sc_qp *qp, __le64 *qp_ctx,
			     struct irdma_qp_host_ctx_info *info);
int irdma_sc_cq_destroy(struct irdma_sc_cq *cq, u64 scratch, bool post_sq);
int irdma_sc_cq_init(struct irdma_sc_cq *cq, struct irdma_cq_init_info *info);
void irdma_sc_cq_resize(struct irdma_sc_cq *cq, struct irdma_modify_cq_info *info);
int irdma_sc_static_hmc_pages_allocated(struct irdma_sc_cqp *cqp, u64 scratch,
					u8 hmc_fn_id, bool post_sq,
					bool poll_registers);

void sc_vsi_update_stats(struct irdma_sc_vsi *vsi);
struct cqp_info {
	union {
		struct {
			struct irdma_sc_qp *qp;
			struct irdma_create_qp_info info;
			u64 scratch;
		} qp_create;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_modify_qp_info info;
			u64 scratch;
		} qp_modify;

		struct {
			struct irdma_sc_qp *qp;
			u64 scratch;
			bool remove_hash_idx;
			bool ignore_mw_bnd;
		} qp_destroy;

		struct {
			struct irdma_sc_cq *cq;
			u64 scratch;
			bool check_overflow;
		} cq_create;

		struct {
			struct irdma_sc_cq *cq;
			struct irdma_modify_cq_info info;
			u64 scratch;
		} cq_modify;

		struct {
			struct irdma_sc_cq *cq;
			u64 scratch;
		} cq_destroy;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_allocate_stag_info info;
			u64 scratch;
		} alloc_stag;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_mw_alloc_info info;
			u64 scratch;
		} mw_alloc;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_reg_ns_stag_info info;
			u64 scratch;
		} mr_reg_non_shared;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_dealloc_stag_info info;
			u64 scratch;
		} dealloc_stag;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_add_arp_cache_entry_info info;
			u64 scratch;
		} add_arp_cache_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
			u16 arp_index;
		} del_arp_cache_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_local_mac_entry_info info;
			u64 scratch;
		} add_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
			u8 entry_idx;
			u8 ignore_ref_count;
		} del_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
		} alloc_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_cqp_manage_push_page_info info;
			u64 scratch;
		} manage_push_page;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_upload_context_info info;
			u64 scratch;
		} qp_upload_context;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_hmc_fcn_info info;
			u64 scratch;
		} manage_hmc_pm;

		struct {
			struct irdma_sc_ceq *ceq;
			u64 scratch;
		} ceq_create;

		struct {
			struct irdma_sc_ceq *ceq;
			u64 scratch;
		} ceq_destroy;

		struct {
			struct irdma_sc_aeq *aeq;
			u64 scratch;
		} aeq_create;

		struct {
			struct irdma_sc_aeq *aeq;
			u64 scratch;
		} aeq_destroy;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_qp_flush_info info;
			u64 scratch;
		} qp_flush_wqes;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_gen_ae_info info;
			u64 scratch;
		} gen_ae;

		struct {
			struct irdma_sc_cqp *cqp;
			void *fpm_val_va;
			u64 fpm_val_pa;
			u8 hmc_fn_id;
			u64 scratch;
		} query_fpm_val;

		struct {
			struct irdma_sc_cqp *cqp;
			void *fpm_val_va;
			u64 fpm_val_pa;
			u8 hmc_fn_id;
			u64 scratch;
		} commit_fpm_val;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_apbvt_info info;
			u64 scratch;
		} manage_apbvt_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_qhash_table_info info;
			u64 scratch;
		} manage_qhash_table_entry;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_update_sds_info info;
			u64 scratch;
		} update_pe_sds;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_sc_qp *qp;
			u64 scratch;
		} suspend_resume;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ah_info info;
			u64 scratch;
		} ah_create;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ah_info info;
			u64 scratch;
		} ah_destroy;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_create;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_destroy;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_modify;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_stats_inst_info info;
			u64 scratch;
		} stats_manage;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_stats_gather_info info;
			u64 scratch;
		} stats_gather;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ws_node_info info;
			u64 scratch;
		} ws_node;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_up_info info;
			u64 scratch;
		} up_map;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_dma_mem query_buff_mem;
			u64 scratch;
		} query_rdma;
	} u;
};

struct cqp_cmds_info {
	struct list_head cqp_cmd_entry;
	u8 cqp_cmd;
	u8 post_sq;
	struct cqp_info in;
};

__le64 *irdma_sc_cqp_get_next_send_wqe_idx(struct irdma_sc_cqp *cqp, u64 scratch,
					   u32 *wqe_idx);

/**
 * irdma_sc_cqp_get_next_send_wqe - get next wqe on cqp sq
 * @cqp: struct for cqp hw
 * @scratch: private data for CQP WQE
 */
static inline __le64 *irdma_sc_cqp_get_next_send_wqe(struct irdma_sc_cqp *cqp, u64 scratch)
{
	u32 wqe_idx;

	return irdma_sc_cqp_get_next_send_wqe_idx(cqp, scratch, &wqe_idx);
}
#endif /* IRDMA_TYPE_H */
struct irdma_reg_ns_stag_info {
	u64 reg_addr_pa;
	u64 va;
	u64 total_len;
	u32 page_size;
	u32 chunk_size;
	u32 first_pm_pbl_index;
	enum irdma_addressing_type addr_type;
	irdma_stag_index stag_idx;
	u16 access_rights;
	u32 pd_id;
	irdma_stag_key stag_key;
	bool use_hmc_fcn_index:1;
	u8 hmc_fcn_index;
	bool use_pf_rid:1;
	bool all_memory:1;
};

struct irdma_fast_reg_stag_info {
	u64 wr_id;
	u64 reg_addr_pa;
	u64 fbo;
	void *va;
	u64 total_len;
	u32 page_size;
	u32 chunk_size;
	u32 first_pm_pbl_index;
	enum irdma_addressing_type addr_type;
	irdma_stag_index stag_idx;
	u16 access_rights;
	u32 pd_id;
	irdma_stag_key stag_key;
	bool local_fence:1;
	bool read_fence:1;
	bool signaled:1;
	bool use_hmc_fcn_index:1;
	u8 hmc_fcn_index;
	bool use_pf_rid:1;
	bool defer_flag:1;
};

struct irdma_dealloc_stag_info {
	u32 stag_idx;
	u32 pd_id;
	bool mr:1;
	bool dealloc_pbl:1;
};

struct irdma_register_shared_stag {
	u64 va;
	enum irdma_addressing_type addr_type;
	irdma_stag_index new_stag_idx;
	irdma_stag_index parent_stag_idx;
	u32 access_rights;
	u32 pd_id;
	u32 page_size;
	irdma_stag_key new_stag_key;
};

struct irdma_qp_init_info {
	struct irdma_qp_uk_init_info qp_uk_init_info;
	struct irdma_sc_pd *pd;
	struct irdma_sc_vsi *vsi;
	__le64 *host_ctx;
	u8 *q2;
	u64 sq_pa;
	u64 rq_pa;
	u64 host_ctx_pa;
	u64 q2_pa;
	u64 shadow_area_pa;
	u8 sq_tph_val;
	u8 rq_tph_val;
	bool sq_tph_en:1;
	bool rq_tph_en:1;
	bool rcv_tph_en:1;
	bool xmit_tph_en:1;
	bool virtual_map:1;
};

struct irdma_cq_init_info {
	struct irdma_sc_dev *dev;
	u64 cq_base_pa;
	u64 shadow_area_pa;
	u32 ceq_id;
	u32 shadow_read_threshold;
	u8 pbl_chunk_size;
	u32 first_pm_pbl_idx;
	bool virtual_map:1;
	bool ceqe_mask:1;
	bool ceq_id_valid:1;
	bool tph_en:1;
	u8 tph_val;
	u8 type;
	struct irdma_cq_uk_init_info cq_uk_init_info;
	struct irdma_sc_vsi *vsi;
};

struct irdma_upload_context_info {
	u64 buf_pa;
	u32 qp_id;
	u8 qp_type;
	bool freeze_qp:1;
	bool raw_format:1;
};

struct irdma_local_mac_entry_info {
	u8 mac_addr[6];
	u16 entry_idx;
};

struct irdma_add_arp_cache_entry_info {
	u8 mac_addr[ETH_ALEN];
	u32 reach_max;
	u16 arp_index;
	bool permanent;
};

struct irdma_apbvt_info {
	u16 port;
	bool add;
};

struct irdma_qhash_table_info {
	struct irdma_sc_vsi *vsi;
	enum irdma_quad_hash_manage_type manage;
	enum irdma_quad_entry_type entry_type;
	bool vlan_valid:1;
	bool ipv4_valid:1;
	u8 mac_addr[ETH_ALEN];
	u16 vlan_id;
	u8 user_pri;
	u32 qp_num;
	u32 dest_ip[4];
	u32 src_ip[4];
	u16 dest_port;
	u16 src_port;
};

struct irdma_cqp_manage_push_page_info {
	u32 push_idx;
	u16 qs_handle;
	u8 free_page;
	u8 push_page_type;
};

struct irdma_qp_flush_info {
	u16 sq_minor_code;
	u16 sq_major_code;
	u16 rq_minor_code;
	u16 rq_major_code;
	u16 ae_code;
	u8 ae_src;
	bool sq:1;
	bool rq:1;
	bool userflushcode:1;
	bool generate_ae:1;
};

struct irdma_gen_ae_info {
	u16 ae_code;
	u8 ae_src;
};

struct irdma_cqp_timeout {
	u64 compl_cqp_cmds;
	u32 count;
};

struct irdma_irq_ops {
	void (*irdma_cfg_aeq)(struct irdma_sc_dev *dev, u32 idx, bool enable);
	void (*irdma_cfg_ceq)(struct irdma_sc_dev *dev, u32 ceq_id, u32 idx,
			      bool enable);
	void (*irdma_dis_irq)(struct irdma_sc_dev *dev, u32 idx);
	void (*irdma_en_irq)(struct irdma_sc_dev *dev, u32 idx);
};

void irdma_sc_ccq_arm(struct irdma_sc_cq *ccq);
int irdma_sc_ccq_create(struct irdma_sc_cq *ccq, u64 scratch,
			bool check_overflow, bool post_sq);
int irdma_sc_ccq_destroy(struct irdma_sc_cq *ccq, u64 scratch, bool post_sq);
int irdma_sc_ccq_get_cqe_info(struct irdma_sc_cq *ccq,
			      struct irdma_ccq_cqe_info *info);
int irdma_sc_ccq_init(struct irdma_sc_cq *ccq,
		      struct irdma_ccq_init_info *info);

int irdma_sc_cceq_create(struct irdma_sc_ceq *ceq, u64 scratch);
int irdma_sc_cceq_destroy_done(struct irdma_sc_ceq *ceq);

int irdma_sc_ceq_destroy(struct irdma_sc_ceq *ceq, u64 scratch, bool post_sq);
int irdma_sc_ceq_init(struct irdma_sc_ceq *ceq,
		      struct irdma_ceq_init_info *info);
void irdma_sc_cleanup_ceqes(struct irdma_sc_cq *cq, struct irdma_sc_ceq *ceq);
void *irdma_sc_process_ceq(struct irdma_sc_dev *dev, struct irdma_sc_ceq *ceq);

int irdma_sc_aeq_init(struct irdma_sc_aeq *aeq,
		      struct irdma_aeq_init_info *info);
int irdma_sc_get_next_aeqe(struct irdma_sc_aeq *aeq,
			   struct irdma_aeqe_info *info);
void irdma_sc_repost_aeq_entries(struct irdma_sc_dev *dev, u32 count);

void irdma_sc_pd_init(struct irdma_sc_dev *dev, struct irdma_sc_pd *pd, u32 pd_id,
		      int abi_ver);
void irdma_cfg_aeq(struct irdma_sc_dev *dev, u32 idx, bool enable);
void irdma_check_cqp_progress(struct irdma_cqp_timeout *cqp_timeout,
			      struct irdma_sc_dev *dev);
int irdma_sc_cqp_create(struct irdma_sc_cqp *cqp, u16 *maj_err, u16 *min_err);
int irdma_sc_cqp_destroy(struct irdma_sc_cqp *cqp);
int irdma_sc_cqp_init(struct irdma_sc_cqp *cqp,
		      struct irdma_cqp_init_info *info);
void irdma_sc_cqp_post_sq(struct irdma_sc_cqp *cqp);
int irdma_sc_poll_for_cqp_op_done(struct irdma_sc_cqp *cqp, u8 opcode,
				  struct irdma_ccq_cqe_info *cmpl_info);
int irdma_sc_fast_register(struct irdma_sc_qp *qp,
			   struct irdma_fast_reg_stag_info *info, bool post_sq);
int irdma_sc_qp_create(struct irdma_sc_qp *qp,
		       struct irdma_create_qp_info *info, u64 scratch,
		       bool post_sq);
int irdma_sc_qp_destroy(struct irdma_sc_qp *qp, u64 scratch,
			bool remove_hash_idx, bool ignore_mw_bnd, bool post_sq);
int irdma_sc_qp_flush_wqes(struct irdma_sc_qp *qp,
			   struct irdma_qp_flush_info *info, u64 scratch,
			   bool post_sq);
int irdma_sc_qp_init(struct irdma_sc_qp *qp, struct irdma_qp_init_info *info);
int irdma_sc_qp_modify(struct irdma_sc_qp *qp,
		       struct irdma_modify_qp_info *info, u64 scratch,
		       bool post_sq);
void irdma_sc_send_lsmm(struct irdma_sc_qp *qp, void *lsmm_buf, u32 size,
			irdma_stag stag);

void irdma_sc_send_rtt(struct irdma_sc_qp *qp, bool read);
void irdma_sc_qp_setctx(struct irdma_sc_qp *qp, __le64 *qp_ctx,
			struct irdma_qp_host_ctx_info *info);
void irdma_sc_qp_setctx_roce(struct irdma_sc_qp *qp, __le64 *qp_ctx,
			     struct irdma_qp_host_ctx_info *info);
int irdma_sc_cq_destroy(struct irdma_sc_cq *cq, u64 scratch, bool post_sq);
int irdma_sc_cq_init(struct irdma_sc_cq *cq, struct irdma_cq_init_info *info);
void irdma_sc_cq_resize(struct irdma_sc_cq *cq, struct irdma_modify_cq_info *info);
int irdma_sc_static_hmc_pages_allocated(struct irdma_sc_cqp *cqp, u64 scratch,
					u8 hmc_fn_id, bool post_sq,
					bool poll_registers);

void sc_vsi_update_stats(struct irdma_sc_vsi *vsi);
struct cqp_info {
	union {
		struct {
			struct irdma_sc_qp *qp;
			struct irdma_create_qp_info info;
			u64 scratch;
		} qp_create;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_modify_qp_info info;
			u64 scratch;
		} qp_modify;

		struct {
			struct irdma_sc_qp *qp;
			u64 scratch;
			bool remove_hash_idx;
			bool ignore_mw_bnd;
		} qp_destroy;

		struct {
			struct irdma_sc_cq *cq;
			u64 scratch;
			bool check_overflow;
		} cq_create;

		struct {
			struct irdma_sc_cq *cq;
			struct irdma_modify_cq_info info;
			u64 scratch;
		} cq_modify;

		struct {
			struct irdma_sc_cq *cq;
			u64 scratch;
		} cq_destroy;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_allocate_stag_info info;
			u64 scratch;
		} alloc_stag;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_mw_alloc_info info;
			u64 scratch;
		} mw_alloc;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_reg_ns_stag_info info;
			u64 scratch;
		} mr_reg_non_shared;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_dealloc_stag_info info;
			u64 scratch;
		} dealloc_stag;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_add_arp_cache_entry_info info;
			u64 scratch;
		} add_arp_cache_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
			u16 arp_index;
		} del_arp_cache_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_local_mac_entry_info info;
			u64 scratch;
		} add_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
			u8 entry_idx;
			u8 ignore_ref_count;
		} del_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			u64 scratch;
		} alloc_local_mac_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_cqp_manage_push_page_info info;
			u64 scratch;
		} manage_push_page;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_upload_context_info info;
			u64 scratch;
		} qp_upload_context;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_hmc_fcn_info info;
			u64 scratch;
		} manage_hmc_pm;

		struct {
			struct irdma_sc_ceq *ceq;
			u64 scratch;
		} ceq_create;

		struct {
			struct irdma_sc_ceq *ceq;
			u64 scratch;
		} ceq_destroy;

		struct {
			struct irdma_sc_aeq *aeq;
			u64 scratch;
		} aeq_create;

		struct {
			struct irdma_sc_aeq *aeq;
			u64 scratch;
		} aeq_destroy;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_qp_flush_info info;
			u64 scratch;
		} qp_flush_wqes;

		struct {
			struct irdma_sc_qp *qp;
			struct irdma_gen_ae_info info;
			u64 scratch;
		} gen_ae;

		struct {
			struct irdma_sc_cqp *cqp;
			void *fpm_val_va;
			u64 fpm_val_pa;
			u8 hmc_fn_id;
			u64 scratch;
		} query_fpm_val;

		struct {
			struct irdma_sc_cqp *cqp;
			void *fpm_val_va;
			u64 fpm_val_pa;
			u8 hmc_fn_id;
			u64 scratch;
		} commit_fpm_val;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_apbvt_info info;
			u64 scratch;
		} manage_apbvt_entry;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_qhash_table_info info;
			u64 scratch;
		} manage_qhash_table_entry;

		struct {
			struct irdma_sc_dev *dev;
			struct irdma_update_sds_info info;
			u64 scratch;
		} update_pe_sds;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_sc_qp *qp;
			u64 scratch;
		} suspend_resume;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ah_info info;
			u64 scratch;
		} ah_create;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ah_info info;
			u64 scratch;
		} ah_destroy;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_create;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_destroy;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_mcast_grp_info info;
			u64 scratch;
		} mc_modify;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_stats_inst_info info;
			u64 scratch;
		} stats_manage;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_stats_gather_info info;
			u64 scratch;
		} stats_gather;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_ws_node_info info;
			u64 scratch;
		} ws_node;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_up_info info;
			u64 scratch;
		} up_map;

		struct {
			struct irdma_sc_cqp *cqp;
			struct irdma_dma_mem query_buff_mem;
			u64 scratch;
		} query_rdma;
	} u;
};

struct cqp_cmds_info {
	struct list_head cqp_cmd_entry;
	u8 cqp_cmd;
	u8 post_sq;
	struct cqp_info in;
};

__le64 *irdma_sc_cqp_get_next_send_wqe_idx(struct irdma_sc_cqp *cqp, u64 scratch,
					   u32 *wqe_idx);

/**
 * irdma_sc_cqp_get_next_send_wqe - get next wqe on cqp sq
 * @cqp: struct for cqp hw
 * @scratch: private data for CQP WQE
 */
static inline __le64 *irdma_sc_cqp_get_next_send_wqe(struct irdma_sc_cqp *cqp, u64 scratch)
{
	u32 wqe_idx;

	return irdma_sc_cqp_get_next_send_wqe_idx(cqp, scratch, &wqe_idx);
}
#endif /* IRDMA_TYPE_H */