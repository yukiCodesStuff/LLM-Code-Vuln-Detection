		if (unlikely(msg_in_group(hdr) || msg_mcast(hdr))) {
			skb_queue_tail(mc_inputq, skb);
			return true;
		}
		fallthrough;
	case CONN_MANAGER:
		skb_queue_tail(inputq, skb);
		return true;
	case GROUP_PROTOCOL:
		skb_queue_tail(mc_inputq, skb);
		return true;
	case NAME_DISTRIBUTOR:
		l->bc_rcvlink->state = LINK_ESTABLISHED;
		skb_queue_tail(l->namedq, skb);
		return true;
	case MSG_BUNDLER:
	case TUNNEL_PROTOCOL:
	case MSG_FRAGMENTER:
	case BCAST_PROTOCOL:
		return false;
#ifdef CONFIG_TIPC_CRYPTO
	case MSG_CRYPTO:
		if (TIPC_SKB_CB(skb)->decrypted) {
			tipc_crypto_msg_rcv(l->net, skb);
			return true;
		}