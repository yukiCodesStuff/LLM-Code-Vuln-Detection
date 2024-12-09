	union {
		struct socket_wq __rcu	*sk_wq;
		struct socket_wq	*sk_wq_raw;
	};
#ifdef CONFIG_XFRM
	struct xfrm_policy __rcu *sk_policy[2];
#endif
	struct dst_entry	*sk_rx_dst;
	struct dst_entry __rcu	*sk_dst_cache;
	/* Note: 32bit hole on 64bit arches */
	atomic_t		sk_wmem_alloc;
	atomic_t		sk_omem_alloc;
	int			sk_sndbuf;
	struct sk_buff_head	sk_write_queue;
	kmemcheck_bitfield_begin(flags);
	unsigned int		sk_shutdown  : 2,
				sk_no_check_tx : 1,
				sk_no_check_rx : 1,
				sk_userlocks : 4,
				sk_protocol  : 8,
#define SK_PROTOCOL_MAX U8_MAX
				sk_type      : 16;
	kmemcheck_bitfield_end(flags);
	int			sk_wmem_queued;
	gfp_t			sk_allocation;
	u32			sk_pacing_rate; /* bytes per second */
	u32			sk_max_pacing_rate;
	netdev_features_t	sk_route_caps;
	netdev_features_t	sk_route_nocaps;
	int			sk_gso_type;
	unsigned int		sk_gso_max_size;
	u16			sk_gso_max_segs;
	int			sk_rcvlowat;
	unsigned long	        sk_lingertime;
	struct sk_buff_head	sk_error_queue;
	struct proto		*sk_prot_creator;
	rwlock_t		sk_callback_lock;
	int			sk_err,
				sk_err_soft;
	u32			sk_ack_backlog;
	u32			sk_max_ack_backlog;
	__u32			sk_priority;
#if IS_ENABLED(CONFIG_CGROUP_NET_PRIO)
	__u32			sk_cgrp_prioidx;
#endif
	struct pid		*sk_peer_pid;
	const struct cred	*sk_peer_cred;
	long			sk_rcvtimeo;
	long			sk_sndtimeo;
	struct timer_list	sk_timer;
	ktime_t			sk_stamp;
	u16			sk_tsflags;
	u32			sk_tskey;
	struct socket		*sk_socket;
	void			*sk_user_data;
	struct page_frag	sk_frag;
	struct sk_buff		*sk_send_head;
	__s32			sk_peek_off;
	int			sk_write_pending;
#ifdef CONFIG_SECURITY
	void			*sk_security;
#endif
	__u32			sk_mark;
#ifdef CONFIG_CGROUP_NET_CLASSID
	u32			sk_classid;
#endif
	struct cg_proto		*sk_cgrp;
	void			(*sk_state_change)(struct sock *sk);
	void			(*sk_data_ready)(struct sock *sk);
	void			(*sk_write_space)(struct sock *sk);
	void			(*sk_error_report)(struct sock *sk);
	int			(*sk_backlog_rcv)(struct sock *sk,
						  struct sk_buff *skb);
	void                    (*sk_destruct)(struct sock *sk);
};

#define __sk_user_data(sk) ((*((void __rcu **)&(sk)->sk_user_data)))

#define rcu_dereference_sk_user_data(sk)	rcu_dereference(__sk_user_data((sk)))
#define rcu_assign_sk_user_data(sk, ptr)	rcu_assign_pointer(__sk_user_data((sk)), ptr)

/*
 * SK_CAN_REUSE and SK_NO_REUSE on a socket mean that the socket is OK
 * or not whether his port will be reused by someone else. SK_FORCE_REUSE
 * on a socket means that the socket will reuse everybody else's port
 * without looking at the other's sk_reuse value.
 */

#define SK_NO_REUSE	0
#define SK_CAN_REUSE	1
#define SK_FORCE_REUSE	2

static inline int sk_peek_offset(struct sock *sk, int flags)
{
	if ((flags & MSG_PEEK) && (sk->sk_peek_off >= 0))
		return sk->sk_peek_off;
	else
		return 0;
}