	int evl_cr_off;
	int cr_status_off;
	int cr_result_off;
	bool user_submission_safe;
	load_device_defaults_fn_t load_device_defaults;
};

struct idxd_evl {

	struct dentry *dbgfs_dir;
	struct dentry *dbgfs_evl_file;

	bool user_submission_safe;
};

static inline unsigned int evl_ent_size(struct idxd_device *idxd)
{