	if (hdr->segments_left > n + 1) {
		__IP6_INC_STATS(net, idev, IPSTATS_MIB_INHDRERRORS);
		icmpv6_param_prob(skb, ICMPV6_HDR_FIELD,
				  ((&hdr->segments_left) -
				   skb_network_header(skb)));
		return -1;
	}

	if (!pskb_may_pull(skb, ipv6_rpl_srh_size(n, hdr->cmpri,
						  hdr->cmpre))) {
		kfree_skb(skb);
		return -1;
	}

	hdr->segments_left--;
	i = n - hdr->segments_left;

	buf = kcalloc(struct_size(hdr, segments.addr, n + 2), 2, GFP_ATOMIC);
	if (unlikely(!buf)) {
		kfree_skb(skb);
		return -1;
	}

	ohdr = (struct ipv6_rpl_sr_hdr *)buf;
	ipv6_rpl_srh_decompress(ohdr, hdr, &ipv6_hdr(skb)->daddr, n);
	chdr = (struct ipv6_rpl_sr_hdr *)(buf + ((ohdr->hdrlen + 1) << 3));

	if ((ipv6_addr_type(&ipv6_hdr(skb)->daddr) & IPV6_ADDR_MULTICAST) ||
	    (ipv6_addr_type(&ohdr->rpl_segaddr[i]) & IPV6_ADDR_MULTICAST)) {
		kfree_skb(skb);
		kfree(buf);
		return -1;
	}

	err = ipv6_chk_rpl_srh_loop(net, ohdr->rpl_segaddr, n + 1);
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
	if (unlikely(!hdr->segments_left)) {
		if (pskb_expand_head(skb, sizeof(struct ipv6hdr) + ((chdr->hdrlen + 1) << 3), 0,
				     GFP_ATOMIC)) {
			__IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)), IPSTATS_MIB_OUTDISCARDS);
			kfree_skb(skb);
			kfree(buf);
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
	if (unlikely(!hdr->segments_left)) {
		if (pskb_expand_head(skb, sizeof(struct ipv6hdr) + ((chdr->hdrlen + 1) << 3), 0,
				     GFP_ATOMIC)) {
			__IP6_INC_STATS(net, ip6_dst_idev(skb_dst(skb)), IPSTATS_MIB_OUTDISCARDS);
			kfree_skb(skb);
			kfree(buf);
			return -1;
		}

		oldhdr = ipv6_hdr(skb);
	}