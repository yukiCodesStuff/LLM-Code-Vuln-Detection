static const struct blk_mq_debugfs_attr blk_mq_debugfs_ctx_attrs[] = {
	{"default_rq_list", 0400, .seq_ops = &ctx_default_rq_list_seq_ops},
	{"read_rq_list", 0400, .seq_ops = &ctx_read_rq_list_seq_ops},
	{"poll_rq_list", 0400, .seq_ops = &ctx_poll_rq_list_seq_ops},
	{"dispatched", 0600, ctx_dispatched_show, ctx_dispatched_write},
	{"merged", 0600, ctx_merged_show, ctx_merged_write},
	{"completed", 0600, ctx_completed_show, ctx_completed_write},
	{},
};

static bool debugfs_create_files(struct dentry *parent, void *data,
				 const struct blk_mq_debugfs_attr *attr)
{
	if (IS_ERR_OR_NULL(parent))
		return false;

	d_inode(parent)->i_private = data;

	for (; attr->name; attr++) {
		if (!debugfs_create_file(attr->name, attr->mode, parent,
					 (void *)attr, &blk_mq_debugfs_fops))
			return false;
	}
	return true;
}