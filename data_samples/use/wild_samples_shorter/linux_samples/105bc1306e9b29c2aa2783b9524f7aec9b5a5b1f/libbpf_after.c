	case BPF_PROG_TYPE_CGROUP_SOCK_ADDR:
	case BPF_PROG_TYPE_LIRC_MODE2:
	case BPF_PROG_TYPE_SK_REUSEPORT:
	case BPF_PROG_TYPE_FLOW_DISSECTOR:
		return false;
	case BPF_PROG_TYPE_UNSPEC:
	case BPF_PROG_TYPE_KPROBE:
	case BPF_PROG_TYPE_TRACEPOINT:
	BPF_PROG_SEC("sk_skb",		BPF_PROG_TYPE_SK_SKB),
	BPF_PROG_SEC("sk_msg",		BPF_PROG_TYPE_SK_MSG),
	BPF_PROG_SEC("lirc_mode2",	BPF_PROG_TYPE_LIRC_MODE2),
	BPF_PROG_SEC("flow_dissector",	BPF_PROG_TYPE_FLOW_DISSECTOR),
	BPF_SA_PROG_SEC("cgroup/bind4",	BPF_CGROUP_INET4_BIND),
	BPF_SA_PROG_SEC("cgroup/bind6",	BPF_CGROUP_INET6_BIND),
	BPF_SA_PROG_SEC("cgroup/connect4", BPF_CGROUP_INET4_CONNECT),
	BPF_SA_PROG_SEC("cgroup/connect6", BPF_CGROUP_INET6_CONNECT),
		bpf_program__set_expected_attach_type(prog,
						      expected_attach_type);

		if (!first_prog)
			first_prog = prog;
	}

	bpf_map__for_each(map, obj) {