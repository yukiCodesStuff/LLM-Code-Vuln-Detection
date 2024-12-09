	u8 remote_mac[ETH_ALEN];

	struct list_head rx; /* rx_np list element */
	struct rcu_head rcu;
};

struct netpoll_info {
	atomic_t refcnt;
	struct delayed_work tx_work;

	struct netpoll *netpoll;
	struct rcu_head rcu;
};

void netpoll_send_udp(struct netpoll *np, const char *msg, int len);
void netpoll_print_options(struct netpoll *np);
int netpoll_parse_options(struct netpoll *np, char *opt);
int __netpoll_setup(struct netpoll *np, struct net_device *ndev, gfp_t gfp);
int netpoll_setup(struct netpoll *np);
int netpoll_trap(void);
void netpoll_set_trap(int trap);
void __netpoll_cleanup(struct netpoll *np);
void __netpoll_free_rcu(struct netpoll *np);
void netpoll_cleanup(struct netpoll *np);
int __netpoll_rx(struct sk_buff *skb, struct netpoll_info *npinfo);
void netpoll_send_skb_on_dev(struct netpoll *np, struct sk_buff *skb,
			     struct net_device *dev);
static inline void netpoll_send_skb(struct netpoll *np, struct sk_buff *skb)
{
	unsigned long flags;
	local_irq_save(flags);
	netpoll_send_skb_on_dev(np, skb, np->dev);
	local_irq_restore(flags);
}



#ifdef CONFIG_NETPOLL
static inline bool netpoll_rx_on(struct sk_buff *skb)
{
	struct netpoll_info *npinfo = rcu_dereference_bh(skb->dev->npinfo);

	return npinfo && (!list_empty(&npinfo->rx_np) || npinfo->rx_flags);
}

static inline bool netpoll_rx(struct sk_buff *skb)
{
	struct netpoll_info *npinfo;
	unsigned long flags;
	bool ret = false;

	local_irq_save(flags);

	if (!netpoll_rx_on(skb))
		goto out;

	npinfo = rcu_dereference_bh(skb->dev->npinfo);
	spin_lock(&npinfo->rx_lock);
	/* check rx_flags again with the lock held */
	if (npinfo->rx_flags && __netpoll_rx(skb, npinfo))
		ret = true;
	spin_unlock(&npinfo->rx_lock);

out:
	return ret;
}

static inline int netpoll_receive_skb(struct sk_buff *skb)
{
	if (!list_empty(&skb->dev->napi_list))
		return netpoll_rx(skb);
	}
}

static inline bool netpoll_tx_running(struct net_device *dev)
{
	return irqs_disabled();
}

#else
static inline bool netpoll_rx(struct sk_buff *skb)
{
	return false;
}
static inline bool netpoll_rx_on(struct sk_buff *skb)
{
	return false;
}
static inline int netpoll_receive_skb(struct sk_buff *skb)
{
	return 0;
static inline void netpoll_netdev_init(struct net_device *dev)
{
}
static inline bool netpoll_tx_running(struct net_device *dev)
{
	return false;
}
#endif

#endif