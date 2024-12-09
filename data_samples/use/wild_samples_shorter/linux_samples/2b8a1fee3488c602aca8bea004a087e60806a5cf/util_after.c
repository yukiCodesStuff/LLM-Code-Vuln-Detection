		remaining = skb->len - offset;
		if (subframe_len > remaining)
			goto purge;
		/* mitigate A-MSDU aggregation injection attacks */
		if (ether_addr_equal(eth.h_dest, rfc1042_header))
			goto purge;

		offset += sizeof(struct ethhdr);
		last = remaining <= subframe_len + padding;
