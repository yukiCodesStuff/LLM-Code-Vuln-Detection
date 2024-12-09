				handle_t *handle, struct inode *inode,
				bool is_block)
{
	struct ext4_xattr_entry *last;
	struct ext4_xattr_entry *here = s->here;
	size_t min_offs = s->end - s->base, name_len = strlen(i->name);
	int in_inode = i->in_inode;
	struct inode *old_ea_inode = NULL;

	/* Compute min_offs and last. */
	last = s->first;
	for (; !IS_LAST_ENTRY(last); last = EXT4_XATTR_NEXT(last)) {
		if (!last->e_value_inum && last->e_value_size) {
			size_t offs = le16_to_cpu(last->e_value_offs);
			if (offs < min_offs)
				min_offs = offs;