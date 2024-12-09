{
	struct llc_station_state_ev *ev = llc_station_ev(skb);

	ev->type   = LLC_STATION_EV_TYPE_PDU;
	ev->reason = 0;
	llc_station_state_process(skb);
}

int __init llc_station_init(void)
{
	int rc = -ENOBUFS;
	struct sk_buff *skb;
	struct llc_station_state_ev *ev;

	skb_queue_head_init(&llc_main_station.mac_pdu_q);
	skb_queue_head_init(&llc_main_station.ev_q.list);
	spin_lock_init(&llc_main_station.ev_q.lock);
	setup_timer(&llc_main_station.ack_timer, llc_station_ack_tmr_cb,
			(unsigned long)&llc_main_station);
	llc_main_station.ack_timer.expires  = jiffies +
						sysctl_llc_station_ack_timeout;
	skb = alloc_skb(0, GFP_ATOMIC);
	if (!skb)
		goto out;
	rc = 0;
	llc_set_station_handler(llc_station_rcv);
	ev = llc_station_ev(skb);
	memset(ev, 0, sizeof(*ev));
	llc_main_station.maximum_retry	= 1;
	llc_main_station.state		= LLC_STATION_STATE_DOWN;
	ev->type	= LLC_STATION_EV_TYPE_SIMPLE;
	ev->prim_type	= LLC_STATION_EV_ENABLE_WITHOUT_DUP_ADDR_CHECK;
	rc = llc_station_next_state(skb);
out:
	return rc;
}

void __exit llc_station_exit(void)
{
	llc_set_station_handler(NULL);
}