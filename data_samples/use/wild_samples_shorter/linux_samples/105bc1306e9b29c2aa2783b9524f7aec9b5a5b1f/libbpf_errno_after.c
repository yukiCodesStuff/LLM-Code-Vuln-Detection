	[ERRCODE_OFFSET(PROGTYPE)]	= "Kernel doesn't support this program type",
	[ERRCODE_OFFSET(WRNGPID)]	= "Wrong pid in netlink message",
	[ERRCODE_OFFSET(INVSEQ)]	= "Invalid netlink sequence",
	[ERRCODE_OFFSET(NLPARSE)]	= "Incorrect netlink message parsing",
};

int libbpf_strerror(int err, char *buf, size_t size)
{