			if (new_ea_inode) {
				int err;

				err = ext4_xattr_inode_dec_ref(handle,
							       new_ea_inode);
				if (err)
					ext4_warning_inode(new_ea_inode,
						  "dec ref new_ea_inode err=%d",
						  err);
				ext4_xattr_inode_free_quota(inode, new_ea_inode,
							    i->value_len);
			}
			goto out;
		}

		ext4_xattr_inode_free_quota(inode, old_ea_inode,
					    le32_to_cpu(here->e_value_size));
	}

	/* No failures allowed past this point. */

	if (!s->not_found && here->e_value_offs) {
		/* Remove the old value. */
		void *first_val = s->base + min_offs;
		size_t offs = le16_to_cpu(here->e_value_offs);
		void *val = s->base + offs;

		memmove(first_val + old_size, first_val, val - first_val);
		memset(first_val, 0, old_size);
		min_offs += old_size;

		/* Adjust all value offsets. */
		last = s->first;
		while (!IS_LAST_ENTRY(last)) {
			size_t o = le16_to_cpu(last->e_value_offs);

			if (!last->e_value_inum &&
			    last->e_value_size && o < offs)
				last->e_value_offs = cpu_to_le16(o + old_size);
			last = EXT4_XATTR_NEXT(last);
		}
	}