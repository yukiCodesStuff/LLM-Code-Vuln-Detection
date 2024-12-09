		if (skb->len == 0) {
			struct sk_buff *tmp = skb_dequeue(&ser->head);
			WARN_ON(tmp != skb);
			if (in_interrupt())
				dev_kfree_skb_irq(skb);
			else
				kfree_skb(skb);
		}
	}
	/* Send flow off if queue is empty */
	if (ser->head.qlen <= SEND_QUEUE_LOW &&
		test_and_clear_bit(CAIF_FLOW_OFF_SENT, &ser->state) &&
		ser->common.flowctrl != NULL)
				ser->common.flowctrl(ser->dev, ON);
	clear_bit(CAIF_SENDING, &ser->state);
	return 0;
error:
	clear_bit(CAIF_SENDING, &ser->state);
	return tty_wr;
}

static int caif_xmit(struct sk_buff *skb, struct net_device *dev)
{
	struct ser_device *ser;

	BUG_ON(dev == NULL);
	ser = netdev_priv(dev);

	/* Send flow off once, on high water mark */
	if (ser->head.qlen > SEND_QUEUE_HIGH &&
		!test_and_set_bit(CAIF_FLOW_OFF_SENT, &ser->state) &&
		ser->common.flowctrl != NULL)

		ser->common.flowctrl(ser->dev, OFF);

	skb_queue_tail(&ser->head, skb);
	return handle_tx(ser);
}


static void ldisc_tx_wakeup(struct tty_struct *tty)
{
	struct ser_device *ser;

	ser = tty->disc_data;
	BUG_ON(ser == NULL);
	WARN_ON(ser->tty != tty);
	handle_tx(ser);
}


static void ser_release(struct work_struct *work)
{
	struct list_head list;
	struct ser_device *ser, *tmp;

	spin_lock(&ser_lock);
	list_replace_init(&ser_release_list, &list);
	spin_unlock(&ser_lock);

	if (!list_empty(&list)) {
		rtnl_lock();
		list_for_each_entry_safe(ser, tmp, &list, node) {
			dev_close(ser->dev);
			unregister_netdevice(ser->dev);
			debugfs_deinit(ser);
		}
		rtnl_unlock();
	}
}

static DECLARE_WORK(ser_release_work, ser_release);

static int ldisc_open(struct tty_struct *tty)
{
	struct ser_device *ser;
	struct net_device *dev;
	char name[64];
	int result;

	/* No write no play */
	if (tty->ops->write == NULL)
		return -EOPNOTSUPP;
	if (!capable(CAP_SYS_ADMIN) && !capable(CAP_SYS_TTY_CONFIG))
		return -EPERM;

	/* release devices to avoid name collision */
	ser_release(NULL);

	result = snprintf(name, sizeof(name), "cf%s", tty->name);
	if (result >= IFNAMSIZ)
		return -EINVAL;
	dev = alloc_netdev(sizeof(*ser), name, NET_NAME_UNKNOWN,
			   caifdev_setup);
	if (!dev)
		return -ENOMEM;

	ser = netdev_priv(dev);
	ser->tty = tty_kref_get(tty);
	ser->dev = dev;
	debugfs_init(ser, tty);
	tty->receive_room = N_TTY_BUF_SIZE;
	tty->disc_data = ser;
	set_bit(TTY_DO_WRITE_WAKEUP, &tty->flags);
	rtnl_lock();
	result = register_netdevice(dev);
	if (result) {
		rtnl_unlock();
		free_netdev(dev);
		return -ENODEV;
	}

	spin_lock(&ser_lock);
	list_add(&ser->node, &ser_list);
	spin_unlock(&ser_lock);
	rtnl_unlock();
	netif_stop_queue(dev);
	update_tty_status(ser);
	return 0;
}

static void ldisc_close(struct tty_struct *tty)
{
	struct ser_device *ser = tty->disc_data;

	tty_kref_put(ser->tty);

	spin_lock(&ser_lock);
	list_move(&ser->node, &ser_release_list);
	spin_unlock(&ser_lock);
	schedule_work(&ser_release_work);
}

/* The line discipline structure. */
static struct tty_ldisc_ops caif_ldisc = {
	.owner =	THIS_MODULE,
	.magic =	TTY_LDISC_MAGIC,
	.name =		"n_caif",
	.open =		ldisc_open,
	.close =	ldisc_close,
	.receive_buf =	ldisc_receive,
	.write_wakeup =	ldisc_tx_wakeup
};

static int register_ldisc(void)
{
	int result;

	result = tty_register_ldisc(N_CAIF, &caif_ldisc);
	if (result < 0) {
		pr_err("cannot register CAIF ldisc=%d err=%d\n", N_CAIF,
			result);
		return result;
	}
	return result;
}
static const struct net_device_ops netdev_ops = {
	.ndo_open = caif_net_open,
	.ndo_stop = caif_net_close,
	.ndo_start_xmit = caif_xmit
};

static void caifdev_setup(struct net_device *dev)
{
	struct ser_device *serdev = netdev_priv(dev);

	dev->features = 0;
	dev->netdev_ops = &netdev_ops;
	dev->type = ARPHRD_CAIF;
	dev->flags = IFF_POINTOPOINT | IFF_NOARP;
	dev->mtu = CAIF_MAX_MTU;
	dev->priv_flags |= IFF_NO_QUEUE;
	dev->needs_free_netdev = true;
	skb_queue_head_init(&serdev->head);
	serdev->common.link_select = CAIF_LINK_LOW_LATENCY;
	serdev->common.use_frag = true;
	serdev->common.use_stx = ser_use_stx;
	serdev->common.use_fcs = ser_use_fcs;
	serdev->dev = dev;
}


static int caif_net_open(struct net_device *dev)
{
	netif_wake_queue(dev);
	return 0;
}

static int caif_net_close(struct net_device *dev)
{
	netif_stop_queue(dev);
	return 0;
}

static int __init caif_ser_init(void)
{
	int ret;

	ret = register_ldisc();
	debugfsdir = debugfs_create_dir("caif_serial", NULL);
	return ret;
}

static void __exit caif_ser_exit(void)
{
	spin_lock(&ser_lock);
	list_splice(&ser_list, &ser_release_list);
	spin_unlock(&ser_lock);
	ser_release(NULL);
	cancel_work_sync(&ser_release_work);
	tty_unregister_ldisc(N_CAIF);
	debugfs_remove_recursive(debugfsdir);
}

module_init(caif_ser_init);
module_exit(caif_ser_exit);