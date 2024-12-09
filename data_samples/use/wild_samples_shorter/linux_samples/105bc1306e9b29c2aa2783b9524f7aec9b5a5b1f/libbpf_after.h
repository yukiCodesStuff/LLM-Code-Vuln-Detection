	LIBBPF_ERRNO__PROGTYPE,	/* Kernel doesn't support this program type */
	LIBBPF_ERRNO__WRNGPID,	/* Wrong pid in netlink message */
	LIBBPF_ERRNO__INVSEQ,	/* Invalid netlink sequence */
	LIBBPF_ERRNO__NLPARSE,	/* netlink parsing error */
	__LIBBPF_ERRNO__END,
};

int libbpf_strerror(int err, char *buf, size_t size);
			       unsigned long page_size,
			       void **buf, size_t *buf_len,
			       bpf_perf_event_print_t fn, void *priv);

struct nlmsghdr;
struct nlattr;
typedef int (*dump_nlmsg_t)(void *cookie, void *msg, struct nlattr **tb);
typedef int (*__dump_nlmsg_t)(struct nlmsghdr *nlmsg, dump_nlmsg_t,
			      void *cookie);
int bpf_netlink_open(unsigned int *nl_pid);
int nl_get_link(int sock, unsigned int nl_pid, dump_nlmsg_t dump_link_nlmsg,
		void *cookie);
int nl_get_class(int sock, unsigned int nl_pid, int ifindex,
		 dump_nlmsg_t dump_class_nlmsg, void *cookie);
int nl_get_qdisc(int sock, unsigned int nl_pid, int ifindex,
		 dump_nlmsg_t dump_qdisc_nlmsg, void *cookie);
int nl_get_filter(int sock, unsigned int nl_pid, int ifindex, int handle,
		  dump_nlmsg_t dump_filter_nlmsg, void *cookie);
#endif