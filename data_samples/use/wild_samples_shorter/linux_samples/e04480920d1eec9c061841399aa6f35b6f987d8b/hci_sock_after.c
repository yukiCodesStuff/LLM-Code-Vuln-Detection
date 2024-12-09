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

void hci_sock_set_flag(struct sock *sk, int nr)
{
	set_bit(nr, &hci_pi(sk)->flags);
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
		read_unlock(&hci_sk_list.lock);
	}
}
static int hci_sock_bound_ioctl(struct sock *sk, unsigned int cmd,
				unsigned long arg)
{
	struct hci_dev *hdev = hci_hdev_from_sock(sk);

	if (IS_ERR(hdev))
		return PTR_ERR(hdev);

	if (hci_dev_test_flag(hdev, HCI_USER_CHANNEL))
		return -EBUSY;


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
	hdev = NULL;

	if (sk->sk_state == BT_BOUND) {
		err = -EALREADY;
		goto done;
	}

	lock_sock(sk);

	hdev = hci_hdev_from_sock(sk);
	if (IS_ERR(hdev)) {
		err = PTR_ERR(hdev);
		goto done;
	}

	haddr->hci_family = AF_BLUETOOTH;
		goto done;
	}

	hdev = hci_hdev_from_sock(sk);
	if (IS_ERR(hdev)) {
		err = PTR_ERR(hdev);
		goto done;
	}

	if (!test_bit(HCI_UP, &hdev->flags)) {