	int sent_pkts = 0;
	bool sock_can_batch = (sock->sk->sk_sndbuf == INT_MAX);

	for (;;) {
		bool busyloop_intr = false;

		if (nvq->done_idx == VHOST_NET_BATCH)
			vhost_tx_batch(net, nvq, sock, &msg);
		vq->heads[nvq->done_idx].id = cpu_to_vhost32(vq, head);
		vq->heads[nvq->done_idx].len = 0;
		++nvq->done_idx;
		if (vhost_exceeds_weight(vq, ++sent_pkts, total_len))
			break;
	}

	vhost_tx_batch(net, nvq, sock, &msg);
}

	bool zcopy_used;
	int sent_pkts = 0;

	for (;;) {
		bool busyloop_intr;

		/* Release DMAs done buffers first */
		vhost_zerocopy_signal_used(net, vq);
		else
			vhost_zerocopy_signal_used(net, vq);
		vhost_net_tx_packet(net);
		if (unlikely(vhost_exceeds_weight(vq, ++sent_pkts,
						  total_len)))
			break;
	}
}

/* Expects to be always run from workqueue - which acts as
 * read-size critical section for our kind of RCU. */
		vq->log : NULL;
	mergeable = vhost_has_feature(vq, VIRTIO_NET_F_MRG_RXBUF);

	while ((sock_len = vhost_net_rx_peek_head_len(net, sock->sk,
						      &busyloop_intr))) {
		sock_len += sock_hlen;
		vhost_len = sock_len + vhost_hlen;
		headcount = get_rx_bufs(vq, vq->heads + nvq->done_idx,
					vhost_len, &in, vq_log, &log,
			vhost_log_write(vq, vq_log, log, vhost_len,
					vq->iov, in);
		total_len += vhost_len;
		if (unlikely(vhost_exceeds_weight(vq, ++recv_pkts, total_len)))
			goto out;
	}
	if (unlikely(busyloop_intr))
		vhost_poll_queue(&vq->poll);
	else
		vhost_net_enable_vq(net, vq);
out:
	vhost_net_signal_used(nvq);
	mutex_unlock(&vq->mutex);
	}
	vhost_dev_init(dev, vqs, VHOST_NET_VQ_MAX,
		       UIO_MAXIOV + VHOST_NET_BATCH,
		       VHOST_NET_WEIGHT, VHOST_NET_PKT_WEIGHT);

	vhost_poll_init(n->poll + VHOST_NET_VQ_TX, handle_tx_net, EPOLLOUT, dev);
	vhost_poll_init(n->poll + VHOST_NET_VQ_RX, handle_rx_net, EPOLLIN, dev);
