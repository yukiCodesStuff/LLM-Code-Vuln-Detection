{
	struct sock *sk = sock->sk;
	struct sockaddr_ec *saddr=(struct sockaddr_ec *)msg->msg_name;
	struct net_device *dev;
	struct ec_addr addr;
	int err;
	unsigned char port, cb;
#if defined(CONFIG_ECONET_AUNUDP) || defined(CONFIG_ECONET_NATIVE)
	struct sk_buff *skb;
	struct ec_cb *eb;
#endif
#ifdef CONFIG_ECONET_AUNUDP
	struct msghdr udpmsg;
	struct iovec iov[msg->msg_iovlen+1];
	struct aunhdr ah;
	struct sockaddr_in udpdest;
	__kernel_size_t size;
	int i;
	mm_segment_t oldfs;
#endif

	/*
	 *	Check the flags.
	 */

	if (msg->msg_flags & ~(MSG_DONTWAIT|MSG_CMSG_COMPAT))
		return -EINVAL;

	/*
	 *	Get and verify the address.
	 */

	mutex_lock(&econet_mutex);

        if (saddr == NULL || msg->msg_namelen < sizeof(struct sockaddr_ec)) {
                mutex_unlock(&econet_mutex);
                return -EINVAL;
        }
	if (dev->type == ARPHRD_ECONET) {
		/* Real hardware Econet.  We're not worthy etc. */
#ifdef CONFIG_ECONET_NATIVE
		unsigned short proto = 0;
		int res;

		dev_hold(dev);

		skb = sock_alloc_send_skb(sk, len+LL_ALLOCATED_SPACE(dev),
					  msg->msg_flags & MSG_DONTWAIT, &err);
		if (skb==NULL)
			goto out_unlock;

		skb_reserve(skb, LL_RESERVED_SPACE(dev));
		skb_reset_network_header(skb);

		eb = (struct ec_cb *)&skb->cb;

		eb->cookie = saddr->cookie;
		eb->sec = *saddr;
		eb->sent = ec_tx_done;

		err = -EINVAL;
		res = dev_hard_header(skb, dev, ntohs(proto), &addr, NULL, len);
		if (res < 0)
			goto out_free;
		if (res > 0) {
			struct ec_framehdr *fh;
			/* Poke in our control byte and
			   port number.  Hack, hack.  */
			fh = (struct ec_framehdr *)(skb->data);
			fh->cb = cb;
			fh->port = port;
			if (sock->type != SOCK_DGRAM) {
				skb_reset_tail_pointer(skb);
				skb->len = 0;
			}
		}