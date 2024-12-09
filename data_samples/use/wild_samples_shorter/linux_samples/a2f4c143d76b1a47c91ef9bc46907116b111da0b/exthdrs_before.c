		return -1;
	}

	if (skb_cloned(skb)) {
		if (pskb_expand_head(skb, IPV6_RPL_SRH_WORST_SWAP_SIZE, 0,
				     GFP_ATOMIC)) {
			__IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)),
					IPSTATS_MIB_OUTDISCARDS);
			kfree_skb(skb);
			return -1;
		}
	} else {
		err = skb_cow_head(skb, IPV6_RPL_SRH_WORST_SWAP_SIZE);
		if (unlikely(err)) {
			kfree_skb(skb);
			return -1;
		}
	}

	hdr = (struct ipv6_rpl_sr_hdr *)skb_transport_header(skb);

	if (!pskb_may_pull(skb, ipv6_rpl_srh_size(n, hdr->cmpri,
						  hdr->cmpre))) {
		kfree_skb(skb);
		return -1;
	skb_pull(skb, ((hdr->hdrlen + 1) << 3));
	skb_postpull_rcsum(skb, oldhdr,
			   sizeof(struct ipv6hdr) + ((hdr->hdrlen + 1) << 3));
	skb_push(skb, ((chdr->hdrlen + 1) << 3) + sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	skb_mac_header_rebuild(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));