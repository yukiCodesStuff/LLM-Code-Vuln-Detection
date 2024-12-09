
	neigh->cm = tx;
	tx->neigh = neigh;
	tx->dev = dev;
	list_add(&tx->list, &priv->cm.start_list);
	set_bit(IPOIB_FLAG_INITIALIZED, &tx->flags);
	queue_work(priv->wq, &priv->cm.start_task);
				neigh->daddr + QPN_AND_OPTIONS_OFFSET);
			goto free_neigh;
		}
		memcpy(&pathrec, &path->pathrec, sizeof(pathrec));

		spin_unlock_irqrestore(&priv->lock, flags);
		netif_tx_unlock_bh(dev);
