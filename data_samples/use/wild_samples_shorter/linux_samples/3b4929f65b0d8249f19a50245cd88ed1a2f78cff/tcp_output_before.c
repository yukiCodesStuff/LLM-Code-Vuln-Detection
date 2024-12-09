	mss_now -= icsk->icsk_ext_hdr_len;

	/* Then reserve room for full set of TCP options and 8 bytes of data */
	if (mss_now < 48)
		mss_now = 48;
	return mss_now;
}

/* Calculate MSS. Not accounting for SACKs here.  */
		if (next_skb_size <= skb_availroom(skb))
			skb_copy_bits(next_skb, 0, skb_put(skb, next_skb_size),
				      next_skb_size);
		else if (!skb_shift(skb, next_skb, next_skb_size))
			return false;
	}
	tcp_highest_sack_replace(sk, next_skb, skb);
