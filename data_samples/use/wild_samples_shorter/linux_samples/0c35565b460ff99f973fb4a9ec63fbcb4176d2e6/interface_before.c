	return err;
}

void xenvif_disconnect(struct xenvif *vif)
{
	struct net_device *dev = vif->dev;
	if (netif_carrier_ok(dev)) {
		rtnl_lock();
		netif_carrier_off(dev); /* discard queued packets */
		if (netif_running(dev))
			xenvif_down(vif);
		rtnl_unlock();
		xenvif_put(vif);
	}

	atomic_dec(&vif->refcnt);
	wait_event(vif->waiting_to_free, atomic_read(&vif->refcnt) == 0);
