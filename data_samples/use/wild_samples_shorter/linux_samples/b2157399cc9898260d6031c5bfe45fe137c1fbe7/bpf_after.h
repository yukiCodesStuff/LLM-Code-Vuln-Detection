	u32 pages;
	u32 id;
	int numa_node;
	bool unpriv_array;
	struct user_struct *user;
	const struct bpf_map_ops *ops;
	struct work_struct work;
	atomic_t usercnt;
struct bpf_array {
	struct bpf_map map;
	u32 elem_size;
	u32 index_mask;
	/* 'ownership' of prog_array is claimed by the first program that
	 * is going to use this map or by the first program which FD is stored
	 * in the map to make sure that all callers and callees have the same
	 * prog_type and JITed flag