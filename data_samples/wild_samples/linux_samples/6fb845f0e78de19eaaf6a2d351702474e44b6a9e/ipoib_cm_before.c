{
	struct ipoib_dev_priv *priv = ipoib_priv(dev);
	struct ipoib_cm_tx *tx;

	tx = kzalloc(sizeof(*tx), GFP_ATOMIC);
	if (!tx)
		return NULL;

	neigh->cm = tx;
	tx->neigh = neigh;
	tx->path = path;
	tx->dev = dev;
	list_add(&tx->list, &priv->cm.start_list);
	set_bit(IPOIB_FLAG_INITIALIZED, &tx->flags);
	queue_work(priv->wq, &priv->cm.start_task);
	return tx;
}

void ipoib_cm_destroy_tx(struct ipoib_cm_tx *tx)
{
	struct ipoib_dev_priv *priv = ipoib_priv(tx->dev);
	unsigned long flags;
	if (test_and_clear_bit(IPOIB_FLAG_INITIALIZED, &tx->flags)) {
		spin_lock_irqsave(&priv->lock, flags);
		list_move(&tx->list, &priv->cm.reap_list);
		queue_work(priv->wq, &priv->cm.reap_task);
		ipoib_dbg(priv, "Reap connection for gid %pI6\n",
			  tx->neigh->daddr + 4);
		tx->neigh = NULL;
		spin_unlock_irqrestore(&priv->lock, flags);
	}
		if (!path) {
			pr_info("%s ignore not valid path %pI6\n",
				__func__,
				neigh->daddr + QPN_AND_OPTIONS_OFFSET);
			goto free_neigh;
		}
		memcpy(&pathrec, &p->path->pathrec, sizeof(pathrec));

		spin_unlock_irqrestore(&priv->lock, flags);
		netif_tx_unlock_bh(dev);

		ret = ipoib_cm_tx_init(p, qpn, &pathrec);

		netif_tx_lock_bh(dev);
		spin_lock_irqsave(&priv->lock, flags);

		if (ret) {
free_neigh:
			neigh = p->neigh;
			if (neigh) {
				neigh->cm = NULL;
				ipoib_neigh_free(neigh);
			}