extern void unix_notinflight(struct file *fp);
extern void unix_gc(void);
extern void wait_for_unix_gc(void);
extern struct sock *unix_get_socket(struct file *filp);

#define UNIX_HASH_SIZE	256

extern unsigned int unix_tot_inflight;
	spinlock_t		lock;
	unsigned int		gc_candidate : 1;
	unsigned int		gc_maybe_cycle : 1;
	unsigned char		recursion_level;
	struct socket_wq	peer_wq;
};
#define unix_sk(__sk) ((struct unix_sock *)__sk)
