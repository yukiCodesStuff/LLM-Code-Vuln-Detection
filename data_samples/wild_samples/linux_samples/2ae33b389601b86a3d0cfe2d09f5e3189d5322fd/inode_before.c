static struct file_system_type hypfs_type = {
	.owner		= THIS_MODULE,
	.name		= "s390_hypfs",
	.mount		= hypfs_mount,
	.kill_sb	= hypfs_kill_super
};

static const struct super_operations hypfs_s_ops = {
	.statfs		= simple_statfs,
	.evict_inode	= hypfs_evict_inode,
	.show_options	= hypfs_show_options,
};

static struct kobject *s390_kobj;

static int __init hypfs_init(void)
{
	int rc;

	rc = hypfs_dbfs_init();
	if (rc)
		return rc;
	if (hypfs_diag_init()) {
		rc = -ENODATA;
		goto fail_dbfs_exit;
	}
	if (hypfs_vm_init()) {
		rc = -ENODATA;
		goto fail_hypfs_diag_exit;
	}
	s390_kobj = kobject_create_and_add("s390", hypervisor_kobj);
	if (!s390_kobj) {
		rc = -ENOMEM;
		goto fail_hypfs_vm_exit;
	}
	rc = register_filesystem(&hypfs_type);
	if (rc)
		goto fail_filesystem;
	return 0;

fail_filesystem:
	kobject_put(s390_kobj);
fail_hypfs_vm_exit:
	hypfs_vm_exit();
fail_hypfs_diag_exit:
	hypfs_diag_exit();
fail_dbfs_exit:
	hypfs_dbfs_exit();
	pr_err("Initialization of hypfs failed with rc=%i\n", rc);
	return rc;
}

static void __exit hypfs_exit(void)
{
	hypfs_diag_exit();
	hypfs_vm_exit();
	hypfs_dbfs_exit();
	unregister_filesystem(&hypfs_type);
	kobject_put(s390_kobj);
}

module_init(hypfs_init)
module_exit(hypfs_exit)

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Holzheu <holzheu@de.ibm.com>");
MODULE_DESCRIPTION("s390 Hypervisor Filesystem");