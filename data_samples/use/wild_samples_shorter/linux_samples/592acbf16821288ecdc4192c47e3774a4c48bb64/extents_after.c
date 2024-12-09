	__le32 border;
	ext4_fsblk_t *ablocks = NULL; /* array of allocated blocks */
	int err = 0;
	size_t ext_size = 0;

	/* make decision: where to split? */
	/* FIXME: now decision is simplest: at current extent */

		le16_add_cpu(&neh->eh_entries, m);
	}

	/* zero out unused area in the extent block */
	ext_size = sizeof(struct ext4_extent_header) +
		sizeof(struct ext4_extent) * le16_to_cpu(neh->eh_entries);
	memset(bh->b_data + ext_size, 0, inode->i_sb->s_blocksize - ext_size);
	ext4_extent_block_csum_set(inode, neh);
	set_buffer_uptodate(bh);
	unlock_buffer(bh);

				sizeof(struct ext4_extent_idx) * m);
			le16_add_cpu(&neh->eh_entries, m);
		}
		/* zero out unused area in the extent block */
		ext_size = sizeof(struct ext4_extent_header) +
		   (sizeof(struct ext4_extent) * le16_to_cpu(neh->eh_entries));
		memset(bh->b_data + ext_size, 0,
			inode->i_sb->s_blocksize - ext_size);
		ext4_extent_block_csum_set(inode, neh);
		set_buffer_uptodate(bh);
		unlock_buffer(bh);

	ext4_fsblk_t newblock, goal = 0;
	struct ext4_super_block *es = EXT4_SB(inode->i_sb)->s_es;
	int err = 0;
	size_t ext_size = 0;

	/* Try to prepend new index to old one */
	if (ext_depth(inode))
		goal = ext4_idx_pblock(EXT_FIRST_INDEX(ext_inode_hdr(inode)));
		goto out;
	}

	ext_size = sizeof(EXT4_I(inode)->i_data);
	/* move top-level index/leaf into new block */
	memmove(bh->b_data, EXT4_I(inode)->i_data, ext_size);
	/* zero out unused area in the extent block */
	memset(bh->b_data + ext_size, 0, inode->i_sb->s_blocksize - ext_size);

	/* set size of new block */
	neh = ext_block_hdr(bh);
	/* old root could have indexes or leaves