	struct inode *ns_inode;
	struct path ns_path;
	char __user *uinsns;
	void *res;
	u32 ulen;

	res = ns_get_path_cb(&ns_path, bpf_prog_offload_info_fill_ns, &args);
	if (IS_ERR(res)) {
		if (!info->ifindex)
			return -ENODEV;
		return PTR_ERR(res);
	}

	down_read(&bpf_devs_lock);

	};
	struct inode *ns_inode;
	struct path ns_path;
	void *res;

	res = ns_get_path_cb(&ns_path, bpf_map_offload_info_fill_ns, &args);
	if (IS_ERR(res)) {
		if (!info->ifindex)
			return -ENODEV;
		return PTR_ERR(res);
	}

	ns_inode = ns_path.dentry->d_inode;
	info->netns_dev = new_encode_dev(ns_inode->i_sb->s_dev);