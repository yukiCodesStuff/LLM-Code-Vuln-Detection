	int skb_len = skb->len;
	unsigned int snaplen, res;
	unsigned long status = TP_STATUS_USER;
	unsigned short macoff, hdrlen;
	unsigned int netoff;
	struct sk_buff *copy_skb = NULL;
	struct timespec64 ts;
	__u32 ts_status;
	bool is_drop_n_account = false;
		}
		macoff = netoff - maclen;
	}
	if (netoff > USHRT_MAX) {
		atomic_inc(&po->tp_drops);
		goto drop_n_restore;
	}
	if (po->tp_version <= TPACKET_V2) {
		if (macoff + snaplen > po->rx_ring.frame_size) {
			if (po->copy_thresh &&
			    atomic_read(&sk->sk_rmem_alloc) < sk->sk_rcvbuf) {