static int x25_asy_close(struct net_device *dev)
{
	struct x25_asy *sl = netdev_priv(dev);

	spin_lock(&sl->lock);
	if (sl->tty)
		clear_bit(TTY_DO_WRITE_WAKEUP, &sl->tty->flags);
	netif_stop_queue(dev);
	sl->rcount = 0;
	sl->xleft  = 0;
	spin_unlock(&sl->lock);
	return 0;
}

static void x25_asy_close_tty(struct tty_struct *tty)
{
	struct x25_asy *sl = tty->disc_data;
	int err;

	/* First make sure we're connected. */
	if (!sl || sl->magic != X25_ASY_MAGIC)
		return;
		dev_close(sl->dev);
	rtnl_unlock();

	err = lapb_unregister(sl->dev);
	if (err != LAPB_OK)
		printk(KERN_ERR "x25_asy_close: lapb_unregister error -%d\n",
			err);

	tty->disc_data = NULL;
	sl->tty = NULL;
	x25_asy_free(sl);
}