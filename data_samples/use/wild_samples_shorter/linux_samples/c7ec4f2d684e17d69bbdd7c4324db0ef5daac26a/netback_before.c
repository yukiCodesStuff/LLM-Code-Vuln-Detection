	}

	for (shinfo->nr_frags = 0; nr_slots > 0 && shinfo->nr_frags < MAX_SKB_FRAGS;
	     shinfo->nr_frags++, gop++, nr_slots--) {
		index = pending_index(queue->pending_cons++);
		pending_idx = queue->pending_ring[index];
		xenvif_tx_create_map_op(queue, pending_idx, txp,
				        txp == first ? extra_count : 0, gop);
		frag_set_pending_idx(&frags[shinfo->nr_frags], pending_idx);

		if (txp == first)
			txp = txfrags;
		else
		shinfo = skb_shinfo(nskb);
		frags = shinfo->frags;

		for (shinfo->nr_frags = 0; shinfo->nr_frags < nr_slots;
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
						gop);
			frag_set_pending_idx(&frags[shinfo->nr_frags],
					     pending_idx);
		}

		skb_shinfo(skb)->frag_list = nskb;
	} else if (nskb) {
		/* A frag_list skb was allocated but it is no longer needed
		 * because enough slots were converted to copy ops above.
		 */
		kfree_skb(nskb);
	}
