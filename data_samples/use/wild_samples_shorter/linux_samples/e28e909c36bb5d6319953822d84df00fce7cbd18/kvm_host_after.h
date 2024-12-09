#endif
	long tlbs_dirty;
	struct list_head devices;
	struct dentry *debugfs_dentry;
	struct kvm_stat_data **debugfs_stat_data;
};

#define kvm_err(fmt, ...) \
	pr_err("kvm [%i]: " fmt, task_pid_nr(current), ## __VA_ARGS__)
	KVM_STAT_VCPU,
};

struct kvm_stat_data {
	int offset;
	struct kvm *kvm;
};

struct kvm_stats_debugfs_item {
	const char *name;
	int offset;
	enum kvm_stat_kind kind;