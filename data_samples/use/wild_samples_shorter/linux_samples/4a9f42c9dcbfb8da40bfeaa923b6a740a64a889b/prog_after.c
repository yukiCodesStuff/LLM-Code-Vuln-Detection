	[BPF_PROG_TYPE_RAW_TRACEPOINT]	= "raw_tracepoint",
	[BPF_PROG_TYPE_CGROUP_SOCK_ADDR] = "cgroup_sock_addr",
	[BPF_PROG_TYPE_LIRC_MODE2]	= "lirc_mode2",
	[BPF_PROG_TYPE_FLOW_DISSECTOR]	= "flow_dissector",
};

static void print_boot_time(__u64 nsecs, char *buf, unsigned int size)
{