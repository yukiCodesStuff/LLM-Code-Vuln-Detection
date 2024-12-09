{
	int error = -EFSCORRUPTED;

	if (BHDR(bh)->h_magic != cpu_to_le32(EXT4_XATTR_MAGIC) ||
	    BHDR(bh)->h_blocks != cpu_to_le32(1))
		goto errout;
	if (buffer_verified(bh))
		return 0;

	error = -EFSBADCRC;
	if (!ext4_xattr_block_csum_verify(inode, bh))
		goto errout;
	error = ext4_xattr_check_entries(BFIRST(bh), bh->b_data + bh->b_size,
					 bh->b_data);
errout:
	if (error)
		__ext4_error_inode(inode, function, line, 0,
				   "corrupted xattr block %llu",
				   (unsigned long long) bh->b_blocknr);
	else
		set_buffer_verified(bh);
	return error;
}

#define ext4_xattr_check_block(inode, bh) \
	__ext4_xattr_check_block((inode), (bh),  __func__, __LINE__)


static int
__xattr_check_inode(struct inode *inode, struct ext4_xattr_ibody_header *header,
			 void *end, const char *function, unsigned int line)
{
	int error = -EFSCORRUPTED;

	if (end - (void *)header < sizeof(*header) + sizeof(u32) ||
	    (header->h_magic != cpu_to_le32(EXT4_XATTR_MAGIC)))
		goto errout;
	error = ext4_xattr_check_entries(IFIRST(header), end, IFIRST(header));
errout:
	if (error)
		__ext4_error_inode(inode, function, line, 0,
				   "corrupted in-inode xattr");
	return error;
}

#define xattr_check_inode(inode, header, end) \
	__xattr_check_inode((inode), (header), (end), __func__, __LINE__)

static int
xattr_find_entry(struct inode *inode, struct ext4_xattr_entry **pentry,
		 void *end, int name_index, const char *name, int sorted)
{
	struct ext4_xattr_entry *entry, *next;
	size_t name_len;
	int cmp = 1;

	if (name == NULL)
		return -EINVAL;
	name_len = strlen(name);
	for (entry = *pentry; !IS_LAST_ENTRY(entry); entry = next) {
		next = EXT4_XATTR_NEXT(entry);
		if ((void *) next >= end) {
			EXT4_ERROR_INODE(inode, "corrupted xattr entries");
			return -EFSCORRUPTED;
		}
		cmp = name_index - entry->e_name_index;
		if (!cmp)
			cmp = name_len - entry->e_name_len;
		if (!cmp)
			cmp = memcmp(name, entry->e_name, name_len);
		if (cmp <= 0 && (sorted || cmp == 0))
			break;
	}