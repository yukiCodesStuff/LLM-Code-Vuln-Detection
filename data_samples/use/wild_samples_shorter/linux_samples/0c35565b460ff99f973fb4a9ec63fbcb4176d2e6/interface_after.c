	return err;
}

void xenvif_carrier_off(struct xenvif *vif)
{
	struct net_device *dev = vif->dev;

	rtnl_lock();
	netif_carrier_off(dev); /* discard queued packets */
	if (netif_running(dev))
		xenvif_down(vif);
	rtnl_unlock();
	xenvif_put(vif);
}

void xenvif_disconnect(struct xenvif *vif)
{
	if (netif_carrier_ok(vif->dev))
		xenvif_carrier_off(vif);

	atomic_dec(&vif->refcnt);
	wait_event(vif->waiting_to_free, atomic_read(&vif->refcnt) == 0);
