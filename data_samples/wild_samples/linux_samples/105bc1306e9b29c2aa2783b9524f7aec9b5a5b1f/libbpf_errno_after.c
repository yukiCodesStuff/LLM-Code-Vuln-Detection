static const char *libbpf_strerror_table[NR_ERRNO] = {
	[ERRCODE_OFFSET(LIBELF)]	= "Something wrong in libelf",
	[ERRCODE_OFFSET(FORMAT)]	= "BPF object format invalid",
	[ERRCODE_OFFSET(KVERSION)]	= "'version' section incorrect or lost",
	[ERRCODE_OFFSET(ENDIAN)]	= "Endian mismatch",
	[ERRCODE_OFFSET(INTERNAL)]	= "Internal error in libbpf",
	[ERRCODE_OFFSET(RELOC)]		= "Relocation failed",
	[ERRCODE_OFFSET(VERIFY)]	= "Kernel verifier blocks program loading",
	[ERRCODE_OFFSET(PROG2BIG)]	= "Program too big",
	[ERRCODE_OFFSET(KVER)]		= "Incorrect kernel version",
	[ERRCODE_OFFSET(PROGTYPE)]	= "Kernel doesn't support this program type",
	[ERRCODE_OFFSET(WRNGPID)]	= "Wrong pid in netlink message",
	[ERRCODE_OFFSET(INVSEQ)]	= "Invalid netlink sequence",
	[ERRCODE_OFFSET(NLPARSE)]	= "Incorrect netlink message parsing",
};

int libbpf_strerror(int err, char *buf, size_t size)
{
	if (!buf || !size)
		return -1;

	err = err > 0 ? err : -err;

	if (err < __LIBBPF_ERRNO__START) {
		int ret;

		ret = strerror_r(err, buf, size);
		buf[size - 1] = '\0';
		return ret;
	}

	if (err < __LIBBPF_ERRNO__END) {
		const char *msg;

		msg = libbpf_strerror_table[ERRNO_OFFSET(err)];
		snprintf(buf, size, "%s", msg);
		buf[size - 1] = '\0';
		return 0;
	}

	snprintf(buf, size, "Unknown libbpf error %d", err);
	buf[size - 1] = '\0';
	return -1;
}