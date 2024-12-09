	if (!llcp_sock->service_name) {
		nfc_llcp_local_put(llcp_sock->local);
		ret = -ENOMEM;
		goto put_dev;
	}
	if (llcp_sock->ssap == LLCP_SAP_MAX) {
		nfc_llcp_local_put(llcp_sock->local);
		ret = -ENOMEM;
		goto put_dev;
	}
	if (!llcp_sock->service_name) {
		ret = -ENOMEM;
		goto sock_llcp_release;
	}

	nfc_llcp_sock_link(&local->connecting_sockets, sk);

	ret = nfc_llcp_send_connect(llcp_sock);
	if (ret)
		goto sock_unlink;

	sk->sk_state = LLCP_CONNECTING;

	ret = sock_wait_state(sk, LLCP_CONNECTED,
			      sock_sndtimeo(sk, flags & O_NONBLOCK));
	if (ret && ret != -EINPROGRESS)
		goto sock_unlink;

	release_sock(sk);

	return ret;

sock_unlink:
	nfc_llcp_sock_unlink(&local->connecting_sockets, sk);
	kfree(llcp_sock->service_name);
	llcp_sock->service_name = NULL;

sock_llcp_release:
	nfc_llcp_put_ssap(local, llcp_sock->ssap);
	nfc_llcp_local_put(llcp_sock->local);

put_dev:
	nfc_put_device(dev);

error:
	release_sock(sk);
	return ret;
}

static int llcp_sock_sendmsg(struct socket *sock, struct msghdr *msg,
			     size_t len)
{
	struct sock *sk = sock->sk;
	struct nfc_llcp_sock *llcp_sock = nfc_llcp_sock(sk);
	int ret;

	pr_debug("sock %p sk %p", sock, sk);

	ret = sock_error(sk);
	if (ret)
		return ret;

	if (msg->msg_flags & MSG_OOB)
		return -EOPNOTSUPP;

	lock_sock(sk);

	if (sk->sk_type == SOCK_DGRAM) {
		DECLARE_SOCKADDR(struct sockaddr_nfc_llcp *, addr,
				 msg->msg_name);

		if (msg->msg_namelen < sizeof(*addr)) {
			release_sock(sk);
			return -EINVAL;
		}

		release_sock(sk);

		return nfc_llcp_send_ui_frame(llcp_sock, addr->dsap, addr->ssap,
					      msg, len);
	}