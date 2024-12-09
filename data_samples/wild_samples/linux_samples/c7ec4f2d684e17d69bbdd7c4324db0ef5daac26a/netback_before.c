		} else {
			/* The copy op partially covered the tx_request.
			 * The remainder will be mapped or copied in the next
			 * iteration.
			 */
			txp->offset += amount;
			txp->size -= amount;
		}
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
			txp++;
	}
	if (nr_slots > 0) {

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