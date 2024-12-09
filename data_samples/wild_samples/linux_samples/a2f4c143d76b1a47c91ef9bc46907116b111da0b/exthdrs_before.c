	if (hdr->segments_left > n + 1) {
		__IP6_INC_STATS(net, idev, IPSTATS_MIB_INHDRERRORS);
		icmpv6_param_prob(skb, ICMPV6_HDR_FIELD,
				  ((&hdr->segments_left) -
				   skb_network_header(skb)));
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
	if (err) {
		icmpv6_send(skb, ICMPV6_PARAMPROB, 0, 0);
		kfree_skb(skb);
		kfree(buf);
		return -1;
	}

	swap(ipv6_hdr(skb)->daddr, ohdr->rpl_segaddr[i]);

	ipv6_rpl_srh_compress(chdr, ohdr, &ipv6_hdr(skb)->daddr, n);

	oldhdr = ipv6_hdr(skb);

	skb_pull(skb, ((hdr->hdrlen + 1) << 3));
	skb_postpull_rcsum(skb, oldhdr,
			   sizeof(struct ipv6hdr) + ((hdr->hdrlen + 1) << 3));
	skb_push(skb, ((chdr->hdrlen + 1) << 3) + sizeof(struct ipv6hdr));
	skb_reset_network_header(skb);
	skb_mac_header_rebuild(skb);
	skb_set_transport_header(skb, sizeof(struct ipv6hdr));

	memmove(ipv6_hdr(skb), oldhdr, sizeof(struct ipv6hdr));
	memcpy(skb_transport_header(skb), chdr, (chdr->hdrlen + 1) << 3);

	ipv6_hdr(skb)->payload_len = htons(skb->len - sizeof(struct ipv6hdr));
	skb_postpush_rcsum(skb, ipv6_hdr(skb),
			   sizeof(struct ipv6hdr) + ((chdr->hdrlen + 1) << 3));

	kfree(buf);

	skb_dst_drop(skb);

	ip6_route_input(skb);

	if (skb_dst(skb)->error) {
		dst_input(skb);
		return -1;
	}