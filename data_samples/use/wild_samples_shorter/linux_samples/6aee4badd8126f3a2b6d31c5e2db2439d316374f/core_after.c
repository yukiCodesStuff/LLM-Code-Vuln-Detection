{
	struct path ns_path;
	struct inode *ns_inode;
	int error;

	error = ns_get_path(&ns_path, task, ns_ops);
	if (!error) {
		ns_inode = ns_path.dentry->d_inode;