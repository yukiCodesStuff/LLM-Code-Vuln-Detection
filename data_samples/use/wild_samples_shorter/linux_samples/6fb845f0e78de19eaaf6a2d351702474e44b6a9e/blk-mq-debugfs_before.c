static bool debugfs_create_files(struct dentry *parent, void *data,
				 const struct blk_mq_debugfs_attr *attr)
{
	d_inode(parent)->i_private = data;

	for (; attr->name; attr++) {
		if (!debugfs_create_file(attr->name, attr->mode, parent,