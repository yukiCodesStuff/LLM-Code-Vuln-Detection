int sock_recv_errqueue(struct sock *sk, struct msghdr *msg, int len, int level,
		       int type);

/*
 *	Enable debug/info messages
 */
extern int net_msg_warn;