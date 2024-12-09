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