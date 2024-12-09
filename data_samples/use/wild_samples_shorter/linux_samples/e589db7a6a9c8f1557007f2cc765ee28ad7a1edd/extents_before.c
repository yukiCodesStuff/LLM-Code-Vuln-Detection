#define EXT4_EXT_MARK_UNINIT1	0x2  /* mark first half uninitialized */
#define EXT4_EXT_MARK_UNINIT2	0x4  /* mark second half uninitialized */

static __le32 ext4_extent_block_csum(struct inode *inode,
				     struct ext4_extent_header *eh)
{
	struct ext4_inode_info *ei = EXT4_I(inode);
	unsigned int ee_len, depth;
	int err = 0;

	ext_debug("ext4_split_extents_at: inode %lu, logical"
		"block %llu\n", inode->i_ino, (unsigned long long)split);

	ext4_ext_show_leaf(inode, path);

	err = ext4_ext_insert_extent(handle, inode, path, &newex, flags);
	if (err == -ENOSPC && (EXT4_EXT_MAY_ZEROOUT & split_flag)) {
		err = ext4_ext_zeroout(inode, &orig_ex);
		if (err)
			goto fix_extent_len;
		/* update the extent length and mark as initialized */
		ex->ee_len = cpu_to_le16(ee_len);
	uninitialized = ext4_ext_is_uninitialized(ex);

	if (map->m_lblk + map->m_len < ee_block + ee_len) {
		split_flag1 = split_flag & EXT4_EXT_MAY_ZEROOUT ?
			      EXT4_EXT_MAY_ZEROOUT : 0;
		flags1 = flags | EXT4_GET_BLOCKS_PRE_IO;
		if (uninitialized)
			split_flag1 |= EXT4_EXT_MARK_UNINIT1 |
				       EXT4_EXT_MARK_UNINIT2;
		err = ext4_split_extent_at(handle, inode, path,
				map->m_lblk + map->m_len, split_flag1, flags1);
		if (err)
			goto out;
		return PTR_ERR(path);

	if (map->m_lblk >= ee_block) {
		split_flag1 = split_flag & EXT4_EXT_MAY_ZEROOUT ?
			      EXT4_EXT_MAY_ZEROOUT : 0;
		if (uninitialized)
			split_flag1 |= EXT4_EXT_MARK_UNINIT1;
		if (split_flag & EXT4_EXT_MARK_UNINIT2)
			split_flag1 |= EXT4_EXT_MARK_UNINIT2;

	split_flag |= ee_block + ee_len <= eof_block ? EXT4_EXT_MAY_ZEROOUT : 0;
	split_flag |= EXT4_EXT_MARK_UNINIT2;

	flags |= EXT4_GET_BLOCKS_PRE_IO;
	return ext4_split_extent(handle, inode, path, map, split_flag, flags);
}

static int ext4_convert_unwritten_extents_endio(handle_t *handle,
					      struct inode *inode,
					      struct ext4_ext_path *path)
{
	struct ext4_extent *ex;
	int depth;
	int err = 0;

	depth = ext_depth(inode);
	ex = path[depth].p_ext;

	ext_debug("ext4_convert_unwritten_extents_endio: inode %lu, logical"
		"block %llu, max_blocks %u\n", inode->i_ino,
		(unsigned long long)le32_to_cpu(ex->ee_block),
		ext4_ext_get_actual_len(ex));

	err = ext4_ext_get_access(handle, inode, path + depth);
	if (err)
		goto out;
	}
	/* IO end_io complete, convert the filled extent to written */
	if ((flags & EXT4_GET_BLOCKS_CONVERT)) {
		ret = ext4_convert_unwritten_extents_endio(handle, inode,
							path);
		if (ret >= 0) {
			ext4_update_inode_fsync_trans(handle, inode, 1);
			err = check_eofblocks_fl(handle, inode, map->m_lblk,
	 */
	if (len <= EXT_UNINIT_MAX_LEN << blkbits)
		flags |= EXT4_GET_BLOCKS_NO_NORMALIZE;
retry:
	while (ret >= 0 && ret < max_blocks) {
		map.m_lblk = map.m_lblk + ret;
		map.m_len = max_blocks = max_blocks - ret;