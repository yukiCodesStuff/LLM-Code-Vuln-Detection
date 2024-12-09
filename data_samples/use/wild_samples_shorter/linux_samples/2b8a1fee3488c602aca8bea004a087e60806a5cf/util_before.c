		remaining = skb->len - offset;
		if (subframe_len > remaining)
			goto purge;

		offset += sizeof(struct ethhdr);
		last = remaining <= subframe_len + padding;
