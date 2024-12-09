		return false;
#ifdef CONFIG_TIPC_CRYPTO
	case MSG_CRYPTO:
		if (TIPC_SKB_CB(skb)->decrypted) {
			tipc_crypto_msg_rcv(l->net, skb);
			return true;
		}
		fallthrough;
#endif
	default:
		pr_warn("Dropping received illegal msg type\n");
		kfree_skb(skb);