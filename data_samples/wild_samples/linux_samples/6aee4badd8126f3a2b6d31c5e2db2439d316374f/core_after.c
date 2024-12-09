{
	struct path ns_path;
	struct inode *ns_inode;
	int error;

	error = ns_get_path(&ns_path, task, ns_ops);
	if (!error) {
		ns_inode = ns_path.dentry->d_inode;
		ns_link_info->dev = new_encode_dev(ns_inode->i_sb->s_dev);
		ns_link_info->ino = ns_inode->i_ino;
		path_put(&ns_path);
	}