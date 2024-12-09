		netpoll_send_skb(np, skb);
}

extern int br_netpoll_enable(struct net_bridge_port *p);
extern void br_netpoll_disable(struct net_bridge_port *p);
#else
static inline struct netpoll_info *br_netpoll_info(struct net_bridge *br)
{
{
}

static inline int br_netpoll_enable(struct net_bridge_port *p)
{
	return 0;
}
