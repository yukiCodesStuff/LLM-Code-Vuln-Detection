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

/*
 * Handle the 'receiver data ready' interrupt.
 * This function is called by the 'tty_io' module in the kernel when
 * a block of X.25 data has been received, which can now be decapsulated
 * and sent on to some IP layer for further processing.
 */

static void x25_asy_receive_buf(struct tty_struct *tty,
				const unsigned char *cp, char *fp, int count)
{
	struct x25_asy *sl = tty->disc_data;

	if (!sl || sl->magic != X25_ASY_MAGIC || !netif_running(sl->dev))
		return;


	/* Read the characters out of the buffer */
	while (count--) {
		if (fp && *fp++) {
			if (!test_and_set_bit(SLF_ERROR, &sl->flags))
				sl->dev->stats.rx_errors++;
			cp++;
			continue;
		}
		x25_asy_unesc(sl, *cp++);
	}
{
	struct x25_asy *sl = tty->disc_data;
	int err;

	/* First make sure we're connected. */
	if (!sl || sl->magic != X25_ASY_MAGIC)
		return;

	rtnl_lock();
	if (sl->dev->flags & IFF_UP)
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
{
	struct x25_asy *sl = tty->disc_data;
	int err;

	/* First make sure we're connected. */
	if (!sl || sl->magic != X25_ASY_MAGIC)
		return;

	rtnl_lock();
	if (sl->dev->flags & IFF_UP)
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

 /************************************************************************
  *			STANDARD X.25 ENCAPSULATION		  	 *
  ************************************************************************/

static int x25_asy_esc(unsigned char *s, unsigned char *d, int len)
{
	unsigned char *ptr = d;
	unsigned char c;

	/*
	 * Send an initial END character to flush out any
	 * data that may have accumulated in the receiver
	 * due to line noise.
	 */

	*ptr++ = X25_END;	/* Send 10111110 bit seq */

	/*
	 * For each byte in the packet, send the appropriate
	 * character sequence, according to the X.25 protocol.
	 */

	while (len-- > 0) {
		switch (c = *s++) {
		case X25_END:
			*ptr++ = X25_ESC;
			*ptr++ = X25_ESCAPE(X25_END);
			break;
		case X25_ESC:
			*ptr++ = X25_ESC;
			*ptr++ = X25_ESCAPE(X25_ESC);
			break;
		default:
			*ptr++ = c;
			break;
		}
	}