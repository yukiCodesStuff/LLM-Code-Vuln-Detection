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
		read_unlock(&hci_sk_list.lock);
	}
}

	lock_sock(sk);

	if (sk->sk_state == BT_BOUND) {
		err = -EALREADY;
		goto done;
	}