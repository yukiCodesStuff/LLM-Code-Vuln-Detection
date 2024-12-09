		}
	}

	pp = call_gro_receive(eth_gro_receive, head, skb);
	flush = 0;

out:
	skb_gro_remcsum_cleanup(skb, &grc);