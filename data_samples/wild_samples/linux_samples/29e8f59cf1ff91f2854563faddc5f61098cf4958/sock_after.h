	if (skb->sk) {
		struct sock *sk = skb->sk;

		skb->destructor = NULL;
		skb->sk = NULL;
		return sk;
	}
	return NULL;
}

void sock_enable_timestamp(struct sock *sk, int flag);
int sock_get_timestamp(struct sock *, struct timeval __user *);
int sock_get_timestampns(struct sock *, struct timespec __user *);
int sock_recv_errqueue(struct sock *sk, struct msghdr *msg, int len, int level,
		       int type);

bool sk_ns_capable(const struct sock *sk,
		   struct user_namespace *user_ns, int cap);
bool sk_capable(const struct sock *sk, int cap);
bool sk_net_capable(const struct sock *sk, int cap);

/*
 *	Enable debug/info messages
 */
extern int net_msg_warn;
#define NETDEBUG(fmt, args...) \
	do { if (net_msg_warn) printk(fmt,##args); } while (0)

#define LIMIT_NETDEBUG(fmt, args...) \
	do { if (net_msg_warn && net_ratelimit()) printk(fmt,##args); } while(0)

extern __u32 sysctl_wmem_max;
extern __u32 sysctl_rmem_max;

extern int sysctl_optmem_max;

extern __u32 sysctl_wmem_default;
extern __u32 sysctl_rmem_default;

#endif	/* _SOCK_H */