	struct inode *ns_inode;
	struct path ns_path;
	char __user *uinsns;
	int res;
	u32 ulen;

	res = ns_get_path_cb(&ns_path, bpf_prog_offload_info_fill_ns, &args);
	if (res) {
		if (!info->ifindex)
			return -ENODEV;
		return res;
	}

	down_read(&bpf_devs_lock);

	};
	struct inode *ns_inode;
	struct path ns_path;
	int res;

	res = ns_get_path_cb(&ns_path, bpf_map_offload_info_fill_ns, &args);
	if (res) {
		if (!info->ifindex)
			return -ENODEV;
		return res;
	}

	ns_inode = ns_path.dentry->d_inode;
	info->netns_dev = new_encode_dev(ns_inode->i_sb->s_dev);