			if (xfs_is_always_cow_inode(ip)) {
				error = -EOPNOTSUPP;
				goto out_unlock;
			}
		}

		if (!xfs_is_always_cow_inode(ip)) {
			error = xfs_alloc_file_space(ip, offset, len);
			if (error)
				goto out_unlock;
		}