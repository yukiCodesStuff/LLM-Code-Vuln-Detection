			queue->tx_link[id] = TX_LINK_NONE;
			skb = queue->tx_skbs[id];
			queue->tx_skbs[id] = NULL;
			if (unlikely(gnttab_query_foreign_access(
				queue->grant_tx_ref[id]) != 0)) {
				dev_alert(dev,
					  "Grant still in use by backend domain\n");
				goto err;
			}
			gnttab_end_foreign_access_ref(
				queue->grant_tx_ref[id], GNTMAP_readonly);
			gnttab_release_grant_reference(
				&queue->gref_tx_head, queue->grant_tx_ref[id]);
			queue->grant_tx_ref[id] = GRANT_INVALID_REF;
			queue->grant_tx_page[id] = NULL;