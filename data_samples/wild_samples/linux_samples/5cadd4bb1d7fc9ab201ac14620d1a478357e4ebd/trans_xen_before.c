			     j++) {
				grant_ref_t ref;

				ref = priv->rings[i].intf->ref[j];
				gnttab_end_foreign_access(ref, 0, 0);
			}
			free_pages((unsigned long)priv->rings[i].data.in,
				   priv->rings[i].intf->ring_order -
				   (PAGE_SHIFT - XEN_PAGE_SHIFT));
		}
		gnttab_end_foreign_access(priv->rings[i].ref, 0, 0);
		free_page((unsigned long)priv->rings[i].intf);
	}
	kfree(priv->rings);
	kfree(priv->tag);
	kfree(priv);
}

static int xen_9pfs_front_remove(struct xenbus_device *dev)
{
	struct xen_9pfs_front_priv *priv = dev_get_drvdata(&dev->dev);

	dev_set_drvdata(&dev->dev, NULL);
	xen_9pfs_front_free(priv);
	return 0;
}

static int xen_9pfs_front_alloc_dataring(struct xenbus_device *dev,
					 struct xen_9pfs_dataring *ring,
					 unsigned int order)
{
	int i = 0;
	int ret = -ENOMEM;
	void *bytes = NULL;

	init_waitqueue_head(&ring->wq);
	spin_lock_init(&ring->lock);
	INIT_WORK(&ring->work, p9_xen_response);

	ring->intf = (struct xen_9pfs_data_intf *)get_zeroed_page(GFP_KERNEL);
	if (!ring->intf)
		return ret;
	ret = gnttab_grant_foreign_access(dev->otherend_id,
					  virt_to_gfn(ring->intf), 0);
	if (ret < 0)
		goto out;
	ring->ref = ret;
	bytes = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
			order - (PAGE_SHIFT - XEN_PAGE_SHIFT));
	if (!bytes) {
		ret = -ENOMEM;
		goto out;
	}
	for (; i < (1 << order); i++) {
		ret = gnttab_grant_foreign_access(
				dev->otherend_id, virt_to_gfn(bytes) + i, 0);
		if (ret < 0)
			goto out;
		ring->intf->ref[i] = ret;
	}
	ring->intf->ring_order = order;
	ring->data.in = bytes;
	ring->data.out = bytes + XEN_FLEX_RING_SIZE(order);

	ret = xenbus_alloc_evtchn(dev, &ring->evtchn);
	if (ret)
		goto out;
	ring->irq = bind_evtchn_to_irqhandler(ring->evtchn,
					      xen_9pfs_front_event_handler,
					      0, "xen_9pfs-frontend", ring);
	if (ring->irq >= 0)
		return 0;

	xenbus_free_evtchn(dev, ring->evtchn);
	ret = ring->irq;
out:
	if (bytes) {
		for (i--; i >= 0; i--)
			gnttab_end_foreign_access(ring->intf->ref[i], 0, 0);
		free_pages((unsigned long)bytes,
			   ring->intf->ring_order -
			   (PAGE_SHIFT - XEN_PAGE_SHIFT));
	}
	gnttab_end_foreign_access(ring->ref, 0, 0);
	free_page((unsigned long)ring->intf);
	return ret;
}

static int xen_9pfs_front_probe(struct xenbus_device *dev,
				const struct xenbus_device_id *id)
{
	int ret, i;
	struct xenbus_transaction xbt;
	struct xen_9pfs_front_priv *priv = NULL;
	char *versions;
	unsigned int max_rings, max_ring_order, len = 0;

	versions = xenbus_read(XBT_NIL, dev->otherend, "versions", &len);
	if (IS_ERR(versions))
		return PTR_ERR(versions);
	if (strcmp(versions, "1")) {
		kfree(versions);
		return -EINVAL;
	}
	kfree(versions);
	max_rings = xenbus_read_unsigned(dev->otherend, "max-rings", 0);
	if (max_rings < XEN_9PFS_NUM_RINGS)
		return -EINVAL;
	max_ring_order = xenbus_read_unsigned(dev->otherend,
					      "max-ring-page-order", 0);
	if (max_ring_order > XEN_9PFS_RING_ORDER)
		max_ring_order = XEN_9PFS_RING_ORDER;
	if (p9_xen_trans.maxsize > XEN_FLEX_RING_SIZE(max_ring_order))
		p9_xen_trans.maxsize = XEN_FLEX_RING_SIZE(max_ring_order) / 2;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;
	priv->num_rings = XEN_9PFS_NUM_RINGS;
	priv->rings = kcalloc(priv->num_rings, sizeof(*priv->rings),
			      GFP_KERNEL);
	if (!priv->rings) {
		kfree(priv);
		return -ENOMEM;
	}

	for (i = 0; i < priv->num_rings; i++) {
		priv->rings[i].priv = priv;
		ret = xen_9pfs_front_alloc_dataring(dev, &priv->rings[i],
						    max_ring_order);
		if (ret < 0)
			goto error;
	}

 again:
	ret = xenbus_transaction_start(&xbt);
	if (ret) {
		xenbus_dev_fatal(dev, ret, "starting transaction");
		goto error;
	}
	ret = xenbus_printf(xbt, dev->nodename, "version", "%u", 1);
	if (ret)
		goto error_xenbus;
	ret = xenbus_printf(xbt, dev->nodename, "num-rings", "%u",
			    priv->num_rings);
	if (ret)
		goto error_xenbus;
	for (i = 0; i < priv->num_rings; i++) {
		char str[16];

		BUILD_BUG_ON(XEN_9PFS_NUM_RINGS > 9);
		sprintf(str, "ring-ref%d", i);
		ret = xenbus_printf(xbt, dev->nodename, str, "%d",
				    priv->rings[i].ref);
		if (ret)
			goto error_xenbus;

		sprintf(str, "event-channel-%d", i);
		ret = xenbus_printf(xbt, dev->nodename, str, "%u",
				    priv->rings[i].evtchn);
		if (ret)
			goto error_xenbus;
	}
	priv->tag = xenbus_read(xbt, dev->nodename, "tag", NULL);
	if (IS_ERR(priv->tag)) {
		ret = PTR_ERR(priv->tag);
		goto error_xenbus;
	}
	ret = xenbus_transaction_end(xbt, 0);
	if (ret) {
		if (ret == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, ret, "completing transaction");
		goto error;
	}

	write_lock(&xen_9pfs_lock);
	list_add_tail(&priv->list, &xen_9pfs_devs);
	write_unlock(&xen_9pfs_lock);
	dev_set_drvdata(&dev->dev, priv);
	xenbus_switch_state(dev, XenbusStateInitialised);

	return 0;

 error_xenbus:
	xenbus_transaction_end(xbt, 1);
	xenbus_dev_fatal(dev, ret, "writing xenstore");
 error:
	dev_set_drvdata(&dev->dev, NULL);
	xen_9pfs_front_free(priv);
	return ret;
}

static int xen_9pfs_front_resume(struct xenbus_device *dev)
{
	dev_warn(&dev->dev, "suspend/resume unsupported\n");
	return 0;
}

static void xen_9pfs_front_changed(struct xenbus_device *dev,
				   enum xenbus_state backend_state)
{
	switch (backend_state) {
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateUnknown:
		break;

	case XenbusStateInitWait:
		break;

	case XenbusStateConnected:
		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;	/* Missed the backend's CLOSING state */
	case XenbusStateClosing:
		xenbus_frontend_closed(dev);
		break;
	}
}

static struct xenbus_driver xen_9pfs_front_driver = {
	.ids = xen_9pfs_front_ids,
	.probe = xen_9pfs_front_probe,
	.remove = xen_9pfs_front_remove,
	.resume = xen_9pfs_front_resume,
	.otherend_changed = xen_9pfs_front_changed,
};

static int p9_trans_xen_init(void)
{
	int rc;

	if (!xen_domain())
		return -ENODEV;

	pr_info("Initialising Xen transport for 9pfs\n");

	v9fs_register_trans(&p9_xen_trans);
	rc = xenbus_register_frontend(&xen_9pfs_front_driver);
	if (rc)
		v9fs_unregister_trans(&p9_xen_trans);

	return rc;
}
module_init(p9_trans_xen_init);
MODULE_ALIAS_9P("xen");

static void p9_trans_xen_exit(void)
{
	v9fs_unregister_trans(&p9_xen_trans);
	return xenbus_unregister_driver(&xen_9pfs_front_driver);
}
module_exit(p9_trans_xen_exit);

MODULE_ALIAS("xen:9pfs");
MODULE_AUTHOR("Stefano Stabellini <stefano@aporeto.com>");
MODULE_DESCRIPTION("Xen Transport for 9P");
MODULE_LICENSE("GPL");
{
	int i = 0;
	int ret = -ENOMEM;
	void *bytes = NULL;

	init_waitqueue_head(&ring->wq);
	spin_lock_init(&ring->lock);
	INIT_WORK(&ring->work, p9_xen_response);

	ring->intf = (struct xen_9pfs_data_intf *)get_zeroed_page(GFP_KERNEL);
	if (!ring->intf)
		return ret;
	ret = gnttab_grant_foreign_access(dev->otherend_id,
					  virt_to_gfn(ring->intf), 0);
	if (ret < 0)
		goto out;
	ring->ref = ret;
	bytes = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
			order - (PAGE_SHIFT - XEN_PAGE_SHIFT));
	if (!bytes) {
		ret = -ENOMEM;
		goto out;
	}
	if (bytes) {
		for (i--; i >= 0; i--)
			gnttab_end_foreign_access(ring->intf->ref[i], 0, 0);
		free_pages((unsigned long)bytes,
			   ring->intf->ring_order -
			   (PAGE_SHIFT - XEN_PAGE_SHIFT));
	}
	gnttab_end_foreign_access(ring->ref, 0, 0);
	free_page((unsigned long)ring->intf);
	return ret;
}

static int xen_9pfs_front_probe(struct xenbus_device *dev,
				const struct xenbus_device_id *id)
{
	int ret, i;
	struct xenbus_transaction xbt;
	struct xen_9pfs_front_priv *priv = NULL;
	char *versions;
	unsigned int max_rings, max_ring_order, len = 0;

	versions = xenbus_read(XBT_NIL, dev->otherend, "versions", &len);
	if (IS_ERR(versions))
		return PTR_ERR(versions);
	if (strcmp(versions, "1")) {
		kfree(versions);
		return -EINVAL;
	}
	kfree(versions);
	max_rings = xenbus_read_unsigned(dev->otherend, "max-rings", 0);
	if (max_rings < XEN_9PFS_NUM_RINGS)
		return -EINVAL;
	max_ring_order = xenbus_read_unsigned(dev->otherend,
					      "max-ring-page-order", 0);
	if (max_ring_order > XEN_9PFS_RING_ORDER)
		max_ring_order = XEN_9PFS_RING_ORDER;
	if (p9_xen_trans.maxsize > XEN_FLEX_RING_SIZE(max_ring_order))
		p9_xen_trans.maxsize = XEN_FLEX_RING_SIZE(max_ring_order) / 2;

	priv = kzalloc(sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = dev;
	priv->num_rings = XEN_9PFS_NUM_RINGS;
	priv->rings = kcalloc(priv->num_rings, sizeof(*priv->rings),
			      GFP_KERNEL);
	if (!priv->rings) {
		kfree(priv);
		return -ENOMEM;
	}

	for (i = 0; i < priv->num_rings; i++) {
		priv->rings[i].priv = priv;
		ret = xen_9pfs_front_alloc_dataring(dev, &priv->rings[i],
						    max_ring_order);
		if (ret < 0)
			goto error;
	}

 again:
	ret = xenbus_transaction_start(&xbt);
	if (ret) {
		xenbus_dev_fatal(dev, ret, "starting transaction");
		goto error;
	}
	ret = xenbus_printf(xbt, dev->nodename, "version", "%u", 1);
	if (ret)
		goto error_xenbus;
	ret = xenbus_printf(xbt, dev->nodename, "num-rings", "%u",
			    priv->num_rings);
	if (ret)
		goto error_xenbus;
	for (i = 0; i < priv->num_rings; i++) {
		char str[16];

		BUILD_BUG_ON(XEN_9PFS_NUM_RINGS > 9);
		sprintf(str, "ring-ref%d", i);
		ret = xenbus_printf(xbt, dev->nodename, str, "%d",
				    priv->rings[i].ref);
		if (ret)
			goto error_xenbus;

		sprintf(str, "event-channel-%d", i);
		ret = xenbus_printf(xbt, dev->nodename, str, "%u",
				    priv->rings[i].evtchn);
		if (ret)
			goto error_xenbus;
	}
	priv->tag = xenbus_read(xbt, dev->nodename, "tag", NULL);
	if (IS_ERR(priv->tag)) {
		ret = PTR_ERR(priv->tag);
		goto error_xenbus;
	}
	ret = xenbus_transaction_end(xbt, 0);
	if (ret) {
		if (ret == -EAGAIN)
			goto again;
		xenbus_dev_fatal(dev, ret, "completing transaction");
		goto error;
	}

	write_lock(&xen_9pfs_lock);
	list_add_tail(&priv->list, &xen_9pfs_devs);
	write_unlock(&xen_9pfs_lock);
	dev_set_drvdata(&dev->dev, priv);
	xenbus_switch_state(dev, XenbusStateInitialised);

	return 0;

 error_xenbus:
	xenbus_transaction_end(xbt, 1);
	xenbus_dev_fatal(dev, ret, "writing xenstore");
 error:
	dev_set_drvdata(&dev->dev, NULL);
	xen_9pfs_front_free(priv);
	return ret;
}

static int xen_9pfs_front_resume(struct xenbus_device *dev)
{
	dev_warn(&dev->dev, "suspend/resume unsupported\n");
	return 0;
}

static void xen_9pfs_front_changed(struct xenbus_device *dev,
				   enum xenbus_state backend_state)
{
	switch (backend_state) {
	case XenbusStateReconfiguring:
	case XenbusStateReconfigured:
	case XenbusStateInitialising:
	case XenbusStateInitialised:
	case XenbusStateUnknown:
		break;

	case XenbusStateInitWait:
		break;

	case XenbusStateConnected:
		xenbus_switch_state(dev, XenbusStateConnected);
		break;

	case XenbusStateClosed:
		if (dev->state == XenbusStateClosed)
			break;
		fallthrough;	/* Missed the backend's CLOSING state */
	case XenbusStateClosing:
		xenbus_frontend_closed(dev);
		break;
	}
}

static struct xenbus_driver xen_9pfs_front_driver = {
	.ids = xen_9pfs_front_ids,
	.probe = xen_9pfs_front_probe,
	.remove = xen_9pfs_front_remove,
	.resume = xen_9pfs_front_resume,
	.otherend_changed = xen_9pfs_front_changed,
};

static int p9_trans_xen_init(void)
{
	int rc;

	if (!xen_domain())
		return -ENODEV;

	pr_info("Initialising Xen transport for 9pfs\n");

	v9fs_register_trans(&p9_xen_trans);
	rc = xenbus_register_frontend(&xen_9pfs_front_driver);
	if (rc)
		v9fs_unregister_trans(&p9_xen_trans);

	return rc;
}
module_init(p9_trans_xen_init);
MODULE_ALIAS_9P("xen");

static void p9_trans_xen_exit(void)
{
	v9fs_unregister_trans(&p9_xen_trans);
	return xenbus_unregister_driver(&xen_9pfs_front_driver);
}
module_exit(p9_trans_xen_exit);

MODULE_ALIAS("xen:9pfs");
MODULE_AUTHOR("Stefano Stabellini <stefano@aporeto.com>");
MODULE_DESCRIPTION("Xen Transport for 9P");
MODULE_LICENSE("GPL");