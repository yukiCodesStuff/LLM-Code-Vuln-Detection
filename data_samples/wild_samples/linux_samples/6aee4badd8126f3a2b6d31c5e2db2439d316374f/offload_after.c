	struct ns_get_path_bpf_prog_args args = {
		.prog	= prog,
		.info	= info,
	};
	struct bpf_prog_aux *aux = prog->aux;
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
	struct ns_get_path_bpf_map_args args = {
		.offmap	= map_to_offmap(map),
		.info	= info,
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