		return false;
#ifdef CONFIG_TIPC_CRYPTO
	case MSG_CRYPTO:
		tipc_crypto_msg_rcv(l->net, skb);
		return true;
#endif
	default:
		pr_warn("Dropping received illegal msg type\n");
		kfree_skb(skb);