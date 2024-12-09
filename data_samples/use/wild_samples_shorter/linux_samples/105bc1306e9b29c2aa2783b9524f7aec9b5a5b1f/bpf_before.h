	BPF_PROG_TYPE_LWT_SEG6LOCAL,
	BPF_PROG_TYPE_LIRC_MODE2,
	BPF_PROG_TYPE_SK_REUSEPORT,
};

enum bpf_attach_type {
	BPF_CGROUP_INET_INGRESS,
	BPF_CGROUP_UDP4_SENDMSG,
	BPF_CGROUP_UDP6_SENDMSG,
	BPF_LIRC_MODE2,
	__MAX_BPF_ATTACH_TYPE
};

#define MAX_BPF_ATTACH_TYPE __MAX_BPF_ATTACH_TYPE
	/* ... here. */

	__u32 data_meta;
};

struct bpf_tunnel_key {
	__u32 tunnel_id;
	BPF_FD_TYPE_URETPROBE,		/* filename + offset */
};

#endif /* _UAPI__LINUX_BPF_H__ */