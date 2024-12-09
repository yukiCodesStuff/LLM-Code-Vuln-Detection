void sock_diag_save_cookie(void *sk, __u32 *cookie);

int sock_diag_put_meminfo(struct sock *sk, struct sk_buff *skb, int attr);
int sock_diag_put_filterinfo(struct sock *sk,
			     struct sk_buff *skb, int attrtype);

#endif