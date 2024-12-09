#define CREATE_TRACE_POINTS
#include <trace/events/kvm.h>

/* Worst case buffer size needed for holding an integer. */
#define ITOA_MAX_LEN 12

MODULE_AUTHOR("Qumranet");
MODULE_LICENSE("GPL");

/* Architectures should define their poll value according to the halt latency */
struct dentry *kvm_debugfs_dir;
EXPORT_SYMBOL_GPL(kvm_debugfs_dir);

static int kvm_debugfs_num_entries;
static const struct file_operations *stat_fops_per_vm[];

static long kvm_vcpu_ioctl(struct file *file, unsigned int ioctl,
			   unsigned long arg);
#ifdef CONFIG_KVM_COMPAT
static long kvm_vcpu_compat_ioctl(struct file *file, unsigned int ioctl,
	kvfree(slots);
}

static void kvm_destroy_vm_debugfs(struct kvm *kvm)
{
	int i;

	if (!kvm->debugfs_dentry)
		return;

	debugfs_remove_recursive(kvm->debugfs_dentry);

	for (i = 0; i < kvm_debugfs_num_entries; i++)
		kfree(kvm->debugfs_stat_data[i]);
	kfree(kvm->debugfs_stat_data);
}

static int kvm_create_vm_debugfs(struct kvm *kvm, int fd)
{
	char dir_name[ITOA_MAX_LEN * 2];
	struct kvm_stat_data *stat_data;
	struct kvm_stats_debugfs_item *p;

	if (!debugfs_initialized())
		return 0;

	snprintf(dir_name, sizeof(dir_name), "%d-%d", task_pid_nr(current), fd);
	kvm->debugfs_dentry = debugfs_create_dir(dir_name,
						 kvm_debugfs_dir);
	if (!kvm->debugfs_dentry)
		return -ENOMEM;

	kvm->debugfs_stat_data = kcalloc(kvm_debugfs_num_entries,
					 sizeof(*kvm->debugfs_stat_data),
					 GFP_KERNEL);
	if (!kvm->debugfs_stat_data)
		return -ENOMEM;

	for (p = debugfs_entries; p->name; p++) {
		stat_data = kzalloc(sizeof(*stat_data), GFP_KERNEL);
		if (!stat_data)
			return -ENOMEM;

		stat_data->kvm = kvm;
		stat_data->offset = p->offset;
		kvm->debugfs_stat_data[p - debugfs_entries] = stat_data;
		if (!debugfs_create_file(p->name, 0444,
					 kvm->debugfs_dentry,
					 stat_data,
					 stat_fops_per_vm[p->kind]))
			return -ENOMEM;
	}
	return 0;
}

static struct kvm *kvm_create_vm(unsigned long type)
{
	int r, i;
	struct kvm *kvm = kvm_arch_alloc_vm();
	int i;
	struct mm_struct *mm = kvm->mm;

	kvm_destroy_vm_debugfs(kvm);
	kvm_arch_sync_events(kvm);
	spin_lock(&kvm_lock);
	list_del(&kvm->vm_list);
	spin_unlock(&kvm_lock);
	}
#endif
	r = anon_inode_getfd("kvm-vm", &kvm_vm_fops, kvm, O_RDWR | O_CLOEXEC);
	if (r < 0) {
		kvm_put_kvm(kvm);
		return r;
	}

	if (kvm_create_vm_debugfs(kvm, r) < 0) {
		kvm_put_kvm(kvm);
		return -ENOMEM;
	}

	return r;
}

	.notifier_call = kvm_cpu_hotplug,
};

static int kvm_debugfs_open(struct inode *inode, struct file *file,
			   int (*get)(void *, u64 *), int (*set)(void *, u64),
			   const char *fmt)
{
	struct kvm_stat_data *stat_data = (struct kvm_stat_data *)
					  inode->i_private;

	/* The debugfs files are a reference to the kvm struct which
	 * is still valid when kvm_destroy_vm is called.
	 * To avoid the race between open and the removal of the debugfs
	 * directory we test against the users count.
	 */
	if (!atomic_add_unless(&stat_data->kvm->users_count, 1, 0))
		return -ENOENT;

	if (simple_attr_open(inode, file, get, set, fmt)) {
		kvm_put_kvm(stat_data->kvm);
		return -ENOMEM;
	}

	return 0;
}

static int kvm_debugfs_release(struct inode *inode, struct file *file)
{
	struct kvm_stat_data *stat_data = (struct kvm_stat_data *)
					  inode->i_private;

	simple_attr_release(inode, file);
	kvm_put_kvm(stat_data->kvm);

	return 0;
}

static int vm_stat_get_per_vm(void *data, u64 *val)
{
	struct kvm_stat_data *stat_data = (struct kvm_stat_data *)data;

	*val = *(u32 *)((void *)stat_data->kvm + stat_data->offset);

	return 0;
}

static int vm_stat_get_per_vm_open(struct inode *inode, struct file *file)
{
	__simple_attr_check_format("%llu\n", 0ull);
	return kvm_debugfs_open(inode, file, vm_stat_get_per_vm,
				NULL, "%llu\n");
}

static const struct file_operations vm_stat_get_per_vm_fops = {
	.owner   = THIS_MODULE,
	.open    = vm_stat_get_per_vm_open,
	.release = kvm_debugfs_release,
	.read    = simple_attr_read,
	.write   = simple_attr_write,
	.llseek  = generic_file_llseek,
};

static int vcpu_stat_get_per_vm(void *data, u64 *val)
{
	int i;
	struct kvm_stat_data *stat_data = (struct kvm_stat_data *)data;
	struct kvm_vcpu *vcpu;

	*val = 0;

	kvm_for_each_vcpu(i, vcpu, stat_data->kvm)
		*val += *(u32 *)((void *)vcpu + stat_data->offset);

	return 0;
}

static int vcpu_stat_get_per_vm_open(struct inode *inode, struct file *file)
{
	__simple_attr_check_format("%llu\n", 0ull);
	return kvm_debugfs_open(inode, file, vcpu_stat_get_per_vm,
				 NULL, "%llu\n");
}

static const struct file_operations vcpu_stat_get_per_vm_fops = {
	.owner   = THIS_MODULE,
	.open    = vcpu_stat_get_per_vm_open,
	.release = kvm_debugfs_release,
	.read    = simple_attr_read,
	.write   = simple_attr_write,
	.llseek  = generic_file_llseek,
};

static const struct file_operations *stat_fops_per_vm[] = {
	[KVM_STAT_VCPU] = &vcpu_stat_get_per_vm_fops,
	[KVM_STAT_VM]   = &vm_stat_get_per_vm_fops,
};

static int vm_stat_get(void *_offset, u64 *val)
{
	unsigned offset = (long)_offset;
	struct kvm *kvm;
	struct kvm_stat_data stat_tmp = {.offset = offset};
	u64 tmp_val;

	*val = 0;
	spin_lock(&kvm_lock);
	list_for_each_entry(kvm, &vm_list, vm_list) {
		stat_tmp.kvm = kvm;
		vm_stat_get_per_vm((void *)&stat_tmp, &tmp_val);
		*val += tmp_val;
	}
	spin_unlock(&kvm_lock);
	return 0;
}

{
	unsigned offset = (long)_offset;
	struct kvm *kvm;
	struct kvm_stat_data stat_tmp = {.offset = offset};
	u64 tmp_val;

	*val = 0;
	spin_lock(&kvm_lock);
	list_for_each_entry(kvm, &vm_list, vm_list) {
		stat_tmp.kvm = kvm;
		vcpu_stat_get_per_vm((void *)&stat_tmp, &tmp_val);
		*val += tmp_val;
	}
	spin_unlock(&kvm_lock);
	return 0;
}

	if (kvm_debugfs_dir == NULL)
		goto out;

	kvm_debugfs_num_entries = 0;
	for (p = debugfs_entries; p->name; ++p, kvm_debugfs_num_entries++) {
		if (!debugfs_create_file(p->name, 0444, kvm_debugfs_dir,
					 (void *)(long)p->offset,
					 stat_fops[p->kind]))
			goto out_dir;