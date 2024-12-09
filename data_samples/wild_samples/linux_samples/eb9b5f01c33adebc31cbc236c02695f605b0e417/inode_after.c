	} else {
		return le32_to_cpu(raw_inode->i_blocks_lo);
	}
}

static inline int ext4_iget_extra_inode(struct inode *inode,
					 struct ext4_inode *raw_inode,
					 struct ext4_inode_info *ei)
{
	__le32 *magic = (void *)raw_inode +
			EXT4_GOOD_OLD_INODE_SIZE + ei->i_extra_isize;

	if (EXT4_GOOD_OLD_INODE_SIZE + ei->i_extra_isize + sizeof(__le32) <=
	    EXT4_INODE_SIZE(inode->i_sb) &&
	    *magic == cpu_to_le32(EXT4_XATTR_MAGIC)) {
		ext4_set_inode_state(inode, EXT4_STATE_XATTR);
		return ext4_find_inline_data_nolock(inode);
	} else
		EXT4_I(inode)->i_inline_off = 0;
	return 0;
}
		if (ei->i_extra_isize == 0) {
			/* The extra space is currently unused. Use it. */
			BUILD_BUG_ON(sizeof(struct ext4_inode) & 3);
			ei->i_extra_isize = sizeof(struct ext4_inode) -
					    EXT4_GOOD_OLD_INODE_SIZE;
		} else {
			ret = ext4_iget_extra_inode(inode, raw_inode, ei);
			if (ret)
				goto bad_inode;
		}