	int evl_cr_off;
	int cr_status_off;
	int cr_result_off;
	load_device_defaults_fn_t load_device_defaults;
};

struct idxd_evl {

	struct dentry *dbgfs_dir;
	struct dentry *dbgfs_evl_file;
};

static inline unsigned int evl_ent_size(struct idxd_device *idxd)
{