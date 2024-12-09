{
}
#endif

static inline int team_dev_queue_xmit(struct team *team, struct team_port *port,
				      struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(skb->queue_mapping) !=
		     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	skb_set_queue_mapping(skb, qdisc_skb_cb(skb)->slave_dev_queue_mapping);

	skb->dev = port->dev;
	if (unlikely(netpoll_tx_running(port->dev))) {
		team_netpoll_send_skb(port, skb);
		return 0;
	}
	return dev_queue_xmit(skb);
}
struct team {
	struct net_device *dev; /* associated netdevice */
	struct team_pcpu_stats __percpu *pcpu_stats;

	struct mutex lock; /* used for overall locking, e.g. port lists write */

	/*
	 * List of enabled ports and their count
	 */
	int en_port_count;
	struct hlist_head en_port_hlist[TEAM_PORT_HASHENTRIES];

	struct list_head port_list; /* list of all ports */

	struct list_head option_list;
	struct list_head option_inst_list; /* list of option instances */

	const struct team_mode *mode;
	struct team_mode_ops ops;
	long mode_priv[TEAM_MODE_PRIV_LONGS];
};

static inline struct hlist_head *team_port_index_hash(struct team *team,
						      int port_index)
{
	return &team->en_port_hlist[port_index & (TEAM_PORT_HASHENTRIES - 1)];
}