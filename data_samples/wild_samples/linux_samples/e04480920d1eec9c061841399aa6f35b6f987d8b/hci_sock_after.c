struct hci_pinfo {
	struct bt_sock    bt;
	struct hci_dev    *hdev;
	struct hci_filter filter;
	__u8              cmsg_mask;
	unsigned short    channel;
	unsigned long     flags;
	__u32             cookie;
	char              comm[TASK_COMM_LEN];
};

static struct hci_dev *hci_hdev_from_sock(struct sock *sk)
{
	struct hci_dev *hdev = hci_pi(sk)->hdev;

	if (!hdev)
		return ERR_PTR(-EBADFD);
	if (hci_dev_test_flag(hdev, HCI_UNREGISTER))
		return ERR_PTR(-EPIPE);
	return hdev;
}
	if (event == HCI_DEV_UNREG) {
		struct sock *sk;

		/* Wake up sockets using this dead device */
		read_lock(&hci_sk_list.lock);
		sk_for_each(sk, &hci_sk_list.head) {
			if (hci_pi(sk)->hdev == hdev) {
				sk->sk_err = EPIPE;
				sk->sk_state_change(sk);
			}
		}
{
	bdaddr_t bdaddr;
	int err;

	if (copy_from_user(&bdaddr, arg, sizeof(bdaddr)))
		return -EFAULT;

	hci_dev_lock(hdev);

	err = hci_bdaddr_list_del(&hdev->reject_list, &bdaddr, BDADDR_BREDR);

	hci_dev_unlock(hdev);

	return err;
}

/* Ioctls that require bound socket */
static int hci_sock_bound_ioctl(struct sock *sk, unsigned int cmd,
				unsigned long arg)
{
	struct hci_dev *hdev = hci_hdev_from_sock(sk);

	if (IS_ERR(hdev))
		return PTR_ERR(hdev);

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
		return hci_sock_reject_list_add(hdev, (void __user *)arg);

	case HCIUNBLOCKADDR:
		if (!capable(CAP_NET_ADMIN))
			return -EPERM;
		return hci_sock_reject_list_del(hdev, (void __user *)arg);
	}

	return -ENOIOCTLCMD;
}
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

	/* Allow detaching from dead device and attaching to alive device, if
	 * the caller wants to re-bind (instead of close) this socket in
	 * response to hci_sock_dev_event(HCI_DEV_UNREG) notification.
	 */
	hdev = hci_pi(sk)->hdev;
	if (hdev && hci_dev_test_flag(hdev, HCI_UNREGISTER)) {
		hci_pi(sk)->hdev = NULL;
		sk->sk_state = BT_OPEN;
		hci_dev_put(hdev);
	}
{
	struct sockaddr_hci *haddr = (struct sockaddr_hci *)addr;
	struct sock *sk = sock->sk;
	struct hci_dev *hdev;
	int err = 0;

	BT_DBG("sock %p sk %p", sock, sk);

	if (peer)
		return -EOPNOTSUPP;

	lock_sock(sk);

	hdev = hci_hdev_from_sock(sk);
	if (IS_ERR(hdev)) {
		err = PTR_ERR(hdev);
		goto done;
	}
	switch (hci_pi(sk)->channel) {
	case HCI_CHANNEL_RAW:
	case HCI_CHANNEL_USER:
		break;
	case HCI_CHANNEL_MONITOR:
		err = -EOPNOTSUPP;
		goto done;
	case HCI_CHANNEL_LOGGING:
		err = hci_logging_frame(sk, msg, len);
		goto done;
	default:
		mutex_lock(&mgmt_chan_list_lock);
		chan = __hci_mgmt_chan_find(hci_pi(sk)->channel);
		if (chan)
			err = hci_mgmt_cmd(chan, sk, msg, len);
		else
			err = -EINVAL;

		mutex_unlock(&mgmt_chan_list_lock);
		goto done;
	}

	hdev = hci_hdev_from_sock(sk);
	if (IS_ERR(hdev)) {
		err = PTR_ERR(hdev);
		goto done;
	}

	if (!test_bit(HCI_UP, &hdev->flags)) {
		err = -ENETDOWN;
		goto done;
	}

	skb = bt_skb_send_alloc(sk, len, msg->msg_flags & MSG_DONTWAIT, &err);
	if (!skb)
		goto done;

	if (memcpy_from_msg(skb_put(skb, len), msg, len)) {
		err = -EFAULT;
		goto drop;
	}

	hci_skb_pkt_type(skb) = skb->data[0];
	skb_pull(skb, 1);

	if (hci_pi(sk)->channel == HCI_CHANNEL_USER) {
		/* No permission check is needed for user channel
		 * since that gets enforced when binding the socket.
		 *
		 * However check that the packet type is valid.
		 */
		if (hci_skb_pkt_type(skb) != HCI_COMMAND_PKT &&
		    hci_skb_pkt_type(skb) != HCI_ACLDATA_PKT &&
		    hci_skb_pkt_type(skb) != HCI_SCODATA_PKT &&
		    hci_skb_pkt_type(skb) != HCI_ISODATA_PKT) {
			err = -EINVAL;
			goto drop;
		}