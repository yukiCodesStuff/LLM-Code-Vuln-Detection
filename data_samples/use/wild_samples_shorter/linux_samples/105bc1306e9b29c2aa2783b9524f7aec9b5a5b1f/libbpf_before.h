	LIBBPF_ERRNO__PROGTYPE,	/* Kernel doesn't support this program type */
	LIBBPF_ERRNO__WRNGPID,	/* Wrong pid in netlink message */
	LIBBPF_ERRNO__INVSEQ,	/* Invalid netlink sequence */
	__LIBBPF_ERRNO__END,
};

int libbpf_strerror(int err, char *buf, size_t size);
			       unsigned long page_size,
			       void **buf, size_t *buf_len,
			       bpf_perf_event_print_t fn, void *priv);
#endif