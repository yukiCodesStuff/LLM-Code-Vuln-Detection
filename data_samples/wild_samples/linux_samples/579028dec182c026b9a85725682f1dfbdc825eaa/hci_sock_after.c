	if (event == HCI_DEV_UNREG) {
		struct sock *sk;

		/* Detach sockets from device */
		read_lock(&hci_sk_list.lock);
		sk_for_each(sk, &hci_sk_list.head) {
			lock_sock(sk);
			if (hci_pi(sk)->hdev == hdev) {
				hci_pi(sk)->hdev = NULL;
				sk->sk_err = EPIPE;
				sk->sk_state = BT_OPEN;
				sk->sk_state_change(sk);

				hci_dev_put(hdev);
			}
			release_sock(sk);
		}
			if (hci_pi(sk)->hdev == hdev) {
				hci_pi(sk)->hdev = NULL;
				sk->sk_err = EPIPE;
				sk->sk_state = BT_OPEN;
				sk->sk_state_change(sk);

				hci_dev_put(hdev);
			}
			release_sock(sk);
		}
		read_unlock(&hci_sk_list.lock);
	}
}

static struct hci_mgmt_chan *__hci_mgmt_chan_find(unsigned short channel)
{
	struct hci_mgmt_chan *c;

	list_for_each_entry(c, &mgmt_chan_list, list) {
		if (c->channel == channel)
			return c;
	}

	return NULL;
}

static struct hci_mgmt_chan *hci_mgmt_chan_find(unsigned short channel)
{
	struct hci_mgmt_chan *c;

	mutex_lock(&mgmt_chan_list_lock);
	c = __hci_mgmt_chan_find(channel);
	mutex_unlock(&mgmt_chan_list_lock);

	return c;
}

int hci_mgmt_chan_register(struct hci_mgmt_chan *c)
{
	if (c->channel < HCI_CHANNEL_CONTROL)
		return -EINVAL;

	mutex_lock(&mgmt_chan_list_lock);
	if (__hci_mgmt_chan_find(c->channel)) {
		mutex_unlock(&mgmt_chan_list_lock);
		return -EALREADY;
	}

	list_add_tail(&c->list, &mgmt_chan_list);

	mutex_unlock(&mgmt_chan_list_lock);

	return 0;
}
EXPORT_SYMBOL(hci_mgmt_chan_register);

void hci_mgmt_chan_unregister(struct hci_mgmt_chan *c)
{
	mutex_lock(&mgmt_chan_list_lock);
	list_del(&c->list);
	mutex_unlock(&mgmt_chan_list_lock);
}
EXPORT_SYMBOL(hci_mgmt_chan_unregister);

static int hci_sock_release(struct socket *sock)
{
	struct sock *sk = sock->sk;
	struct hci_dev *hdev;
	struct sk_buff *skb;

	BT_DBG("sock %p sk %p", sock, sk);

	if (!sk)
		return 0;

	lock_sock(sk);

	switch (hci_pi(sk)->channel) {
	case HCI_CHANNEL_MONITOR:
		atomic_dec(&monitor_promisc);
		break;
	case HCI_CHANNEL_RAW:
	case HCI_CHANNEL_USER:
	case HCI_CHANNEL_CONTROL:
		/* Send event to monitor */
		skb = create_monitor_ctrl_close(sk);
		if (skb) {
			hci_send_to_channel(HCI_CHANNEL_MONITOR, skb,
					    HCI_SOCK_TRUSTED, NULL);
			kfree_skb(skb);
		}

		hci_sock_free_cookie(sk);
		break;
	}

	bt_sock_unlink(&hci_sk_list, sk);

	hdev = hci_pi(sk)->hdev;
	if (hdev) {
		if (hci_pi(sk)->channel == HCI_CHANNEL_USER) {
			/* When releasing a user channel exclusive access,
			 * call hci_dev_do_close directly instead of calling
			 * hci_dev_close to ensure the exclusive access will
			 * be released and the controller brought back down.
			 *
			 * The checking of HCI_AUTO_OFF is not needed in this
			 * case since it will have been cleared already when
			 * opening the user channel.
			 */
			hci_dev_do_close(hdev);
			hci_dev_clear_flag(hdev, HCI_USER_CHANNEL);
			mgmt_index_added(hdev);
		}

		atomic_dec(&hdev->promisc);
		hci_dev_put(hdev);
	}

	sock_orphan(sk);

	skb_queue_purge(&sk->sk_receive_queue);
	skb_queue_purge(&sk->sk_write_queue);

	release_sock(sk);
	sock_put(sk);
	return 0;
}

static int hci_sock_blacklist_add(struct hci_dev *hdev, void __user *arg)
{
	bdaddr_t bdaddr;
	int err;

	if (copy_from_user(&bdaddr, arg, sizeof(bdaddr)))
		return -EFAULT;

	hci_dev_lock(hdev);

	err = hci_bdaddr_list_add(&hdev->blacklist, &bdaddr, BDADDR_BREDR);

	hci_dev_unlock(hdev);

	return err;
}

static int hci_sock_blacklist_del(struct hci_dev *hdev, void __user *arg)
{
	bdaddr_t bdaddr;
	int err;

	if (copy_from_user(&bdaddr, arg, sizeof(bdaddr)))
		return -EFAULT;

	hci_dev_lock(hdev);

	err = hci_bdaddr_list_del(&hdev->blacklist, &bdaddr, BDADDR_BREDR);

	hci_dev_unlock(hdev);

	return err;
}

/* Ioctls that require bound socket */
static int hci_sock_bound_ioctl(struct sock *sk, unsigned int cmd,
				unsigned long arg)
{
	struct hci_dev *hdev = hci_pi(sk)->hdev;

	if (!hdev)
		return -EBADFD;

	if (hci_dev_test_flag(hdev, HCI_USER_CHANNEL))
		return -EBUSY;

	if (hci_dev_test_flag(hdev, HCI_UNCONFIGURED))
		return -EOPNOTSUPP;

	if (hdev->dev_type != HCI_PRIMARY)
		return -EOPNOTSUPP;

	switch (cmd) {
	case HCISETRAW:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return -EOPNOTSUPP;

	case HCIGETCONNINFO:
		return hci_get_conn_info(hdev, (void __user *)arg);

	case HCIGETAUTHINFO:
		return hci_get_auth_info(hdev, (void __user *)arg);

	case HCIBLOCKADDR:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_sock_blacklist_add(hdev, (void __user *)arg);

	case HCIUNBLOCKADDR:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_sock_blacklist_del(hdev, (void __user *)arg);
	}

	return -ENOIOCTLCMD;
}

static int hci_sock_ioctl(struct socket *sock, unsigned int cmd,
			  unsigned long arg)
{
	void __user *argp = (void __user *)arg;
	struct sock *sk = sock->sk;
	int err;

	BT_DBG("cmd %x arg %lx", cmd, arg);

	lock_sock(sk);

	if (hci_pi(sk)->channel != HCI_CHANNEL_RAW) {
		err = -EBADFD;
		goto done;
	}

	/* When calling an ioctl on an unbound raw socket, then ensure
	 * that the monitor gets informed. Ensure that the resulting event
	 * is only send once by checking if the cookie exists or not. The
	 * socket cookie will be only ever generated once for the lifetime
	 * of a given socket.
	 */
	if (hci_sock_gen_cookie(sk)) {
		struct sk_buff *skb;

		if (capable(CAP_NET_ADMIN))
			hci_sock_set_flag(sk, HCI_SOCK_TRUSTED);

		/* Send event to monitor */
		skb = create_monitor_ctrl_open(sk);
		if (skb) {
			hci_send_to_channel(HCI_CHANNEL_MONITOR, skb,
					    HCI_SOCK_TRUSTED, NULL);
			kfree_skb(skb);
		}
	}

	release_sock(sk);

	switch (cmd) {
	case HCIGETDEVLIST:
		return hci_get_dev_list(argp);

	case HCIGETDEVINFO:
		return hci_get_dev_info(argp);

	case HCIGETCONNLIST:
		return hci_get_conn_list(argp);

	case HCIDEVUP:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_dev_open(arg);

	case HCIDEVDOWN:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_dev_close(arg);

	case HCIDEVRESET:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_dev_reset(arg);

	case HCIDEVRESTAT:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_dev_reset_stat(arg);

	case HCISETSCAN:
	case HCISETAUTH:
	case HCISETENCRYPT:
	case HCISETPTYPE:
	case HCISETLINKPOL:
	case HCISETLINKMODE:
	case HCISETACLMTU:
	case HCISETSCOMTU:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_dev_cmd(cmd, argp);

	case HCIINQUIRY:
		return hci_inquiry(argp);
	}

	lock_sock(sk);

	err = hci_sock_bound_ioctl(sk, cmd, arg);

done:
	release_sock(sk);
	return err;
}

#ifdef CONFIG_COMPAT
static int hci_sock_compat_ioctl(struct socket *sock, unsigned int cmd,
				 unsigned long arg)
{
	switch (cmd) {
	case HCIDEVUP:
	case HCIDEVDOWN:
	case HCIDEVRESET:
	case HCIDEVRESTAT:
		return hci_sock_ioctl(sock, cmd, arg);
	}

	return hci_sock_ioctl(sock, cmd, (unsigned long)compat_ptr(arg));
}
#endif

static int hci_sock_bind(struct socket *sock, struct sockaddr *addr,
			 int addr_len)
{
	struct sockaddr_hci haddr;
	struct sock *sk = sock->sk;
	struct hci_dev *hdev = NULL;
	struct sk_buff *skb;
	int len, err = 0;

	BT_DBG("sock %p sk %p", sock, sk);

	if (!addr)
		return -EINVAL;

	memset(&haddr, 0, sizeof(haddr));
	len = min_t(unsigned int, sizeof(haddr), addr_len);
	memcpy(&haddr, addr, len);

	if (haddr.hci_family != AF_BLUETOOTH)
		return -EINVAL;

	lock_sock(sk);

	if (sk->sk_state == BT_BOUND) {
		err = -EALREADY;
		goto done;
	}

	switch (haddr.hci_channel) {
	case HCI_CHANNEL_RAW:
		if (hci_pi(sk)->hdev) {
			err = -EALREADY;
			goto done;
		}

		if (haddr.hci_dev != HCI_DEV_NONE) {
			hdev = hci_dev_get(haddr.hci_dev);
			if (!hdev) {
				err = -ENODEV;
				goto done;
			}

			atomic_inc(&hdev->promisc);
		}

		hci_pi(sk)->channel = haddr.hci_channel;

		if (!hci_sock_gen_cookie(sk)) {
			/* In the case when a cookie has already been assigned,
			 * then there has been already an ioctl issued against
			 * an unbound socket and with that triggerd an open
			 * notification. Send a close notification first to
			 * allow the state transition to bounded.
			 */
			skb = create_monitor_ctrl_close(sk);
			if (skb) {
				hci_send_to_channel(HCI_CHANNEL_MONITOR, skb,
						    HCI_SOCK_TRUSTED, NULL);
				kfree_skb(skb);
			}
		}

		if (capable(CAP_NET_ADMIN))
			hci_sock_set_flag(sk, HCI_SOCK_TRUSTED);

		hci_pi(sk)->hdev = hdev;

		/* Send event to monitor */
		skb = create_monitor_ctrl_open(sk);
		if (skb) {
			hci_send_to_channel(HCI_CHANNEL_MONITOR, skb,
					    HCI_SOCK_TRUSTED, NULL);
			kfree_skb(skb);
		}
		break;

	case HCI_CHANNEL_USER:
		if (hci_pi(sk)->hdev) {
			err = -EALREADY;
			goto done;
		}

		if (haddr.hci_dev == HCI_DEV_NONE) {
			err = -EINVAL;
			goto done;
		}

		if (!capable(CAP_NET_ADMIN)) {
			err = -EPERM;
			goto done;
		}

		hdev = hci_dev_get(haddr.hci_dev);
		if (!hdev) {
			err = -ENODEV;
			goto done;
		}

		if (test_bit(HCI_INIT, &hdev->flags) ||
		    hci_dev_test_flag(hdev, HCI_SETUP) ||
		    hci_dev_test_flag(hdev, HCI_CONFIG) ||
		    (!hci_dev_test_flag(hdev, HCI_AUTO_OFF) &&
		     test_bit(HCI_UP, &hdev->flags))) {
			err = -EBUSY;
			hci_dev_put(hdev);
			goto done;
		}

		if (hci_dev_test_and_set_flag(hdev, HCI_USER_CHANNEL)) {
			err = -EUSERS;
			hci_dev_put(hdev);
			goto done;
		}

		mgmt_index_removed(hdev);

		err = hci_dev_open(hdev->id);
		if (err) {
			if (err == -EALREADY) {
				/* In case the transport is already up and
				 * running, clear the error here.
				 *
				 * This can happen when opening a user
				 * channel and HCI_AUTO_OFF grace period
				 * is still active.
				 */
				err = 0;
			} else {
				hci_dev_clear_flag(hdev, HCI_USER_CHANNEL);
				mgmt_index_added(hdev);
				hci_dev_put(hdev);
				goto done;
			}