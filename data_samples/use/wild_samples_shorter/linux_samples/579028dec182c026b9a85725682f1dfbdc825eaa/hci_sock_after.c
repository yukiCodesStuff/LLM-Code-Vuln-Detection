		/* Detach sockets from device */
		read_lock(&hci_sk_list.lock);
		sk_for_each(sk, &hci_sk_list.head) {
			lock_sock(sk);
			if (hci_pi(sk)->hdev == hdev) {
				hci_pi(sk)->hdev = NULL;
				sk->sk_err = EPIPE;
				sk->sk_state = BT_OPEN;

				hci_dev_put(hdev);
			}
			release_sock(sk);
		}
		read_unlock(&hci_sk_list.lock);
	}
}