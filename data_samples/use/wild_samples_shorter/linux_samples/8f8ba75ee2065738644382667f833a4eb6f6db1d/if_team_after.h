}
#endif

struct team_mode_ops {
	int (*init)(struct team *team);
	void (*exit)(struct team *team);
	rx_handler_result_t (*receive)(struct team *team,
	long mode_priv[TEAM_MODE_PRIV_LONGS];
};

static inline int team_dev_queue_xmit(struct team *team, struct team_port *port,
				      struct sk_buff *skb)
{
	BUILD_BUG_ON(sizeof(skb->queue_mapping) !=
		     sizeof(qdisc_skb_cb(skb)->slave_dev_queue_mapping));
	skb_set_queue_mapping(skb, qdisc_skb_cb(skb)->slave_dev_queue_mapping);

	skb->dev = port->dev;
	if (unlikely(netpoll_tx_running(team->dev))) {
		team_netpoll_send_skb(port, skb);
		return 0;
	}
	return dev_queue_xmit(skb);
}

static inline struct hlist_head *team_port_index_hash(struct team *team,
						      int port_index)
{
	return &team->en_port_hlist[port_index & (TEAM_PORT_HASHENTRIES - 1)];