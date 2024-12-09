	llc_station_state_process(skb);
}

void __init llc_station_init(void)
{
	skb_queue_head_init(&llc_main_station.mac_pdu_q);
	skb_queue_head_init(&llc_main_station.ev_q.list);
	spin_lock_init(&llc_main_station.ev_q.lock);
	setup_timer(&llc_main_station.ack_timer, llc_station_ack_tmr_cb,
			(unsigned long)&llc_main_station);
	llc_main_station.ack_timer.expires  = jiffies +
						sysctl_llc_station_ack_timeout;
	llc_main_station.maximum_retry	= 1;
	llc_main_station.state		= LLC_STATION_STATE_UP;
	llc_set_station_handler(llc_station_rcv);
}

void llc_station_exit(void)
{
	llc_set_station_handler(NULL);
}