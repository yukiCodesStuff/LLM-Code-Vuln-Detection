			if (queue->tx_link[id] != TX_PENDING) {
				dev_alert(dev,
					  "Response for inactive request\n");
				goto err;
			}

			queue->tx_link[id] = TX_LINK_NONE;
			skb = queue->tx_skbs[id];
			queue->tx_skbs[id] = NULL;
			if (unlikely(!gnttab_end_foreign_access_ref(
				queue->grant_tx_ref[id], GNTMAP_readonly))) {
				dev_alert(dev,
					  "Grant still in use by backend domain\n");
				goto err;
			}