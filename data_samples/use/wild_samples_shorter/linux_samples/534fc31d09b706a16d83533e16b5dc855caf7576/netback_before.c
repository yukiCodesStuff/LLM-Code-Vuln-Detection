	struct gnttab_map_grant_ref *gop = queue->tx_map_ops + *map_ops;
	struct xen_netif_tx_request *txp = first;

	nr_slots = shinfo->nr_frags + 1;

	copy_count(skb) = 0;
	XENVIF_TX_CB(skb)->split_mask = 0;

		}
	}

	for (shinfo->nr_frags = 0; shinfo->nr_frags < nr_slots;
	     shinfo->nr_frags++, gop++) {
		index = pending_index(queue->pending_cons++);
		pending_idx = queue->pending_ring[index];
		xenvif_tx_create_map_op(queue, pending_idx, txp,
				        txp == first ? extra_count : 0, gop);
			txp++;
	}

	if (frag_overflow) {

		shinfo = skb_shinfo(nskb);
		frags = shinfo->frags;

		for (shinfo->nr_frags = 0; shinfo->nr_frags < frag_overflow;
		     shinfo->nr_frags++, txp++, gop++) {
			index = pending_index(queue->pending_cons++);
			pending_idx = queue->pending_ring[index];
			xenvif_tx_create_map_op(queue, pending_idx, txp, 0,
		}

		skb_shinfo(skb)->frag_list = nskb;
	}

	(*copy_ops) = cop - queue->tx_copy_ops;
	(*map_ops) = gop - queue->tx_map_ops;