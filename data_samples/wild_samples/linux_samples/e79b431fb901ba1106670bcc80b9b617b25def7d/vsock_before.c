{
	struct vhost_virtqueue *tx_vq = &vsock->vqs[VSOCK_VQ_TX];
	bool added = false;
	bool restart_tx = false;

	mutex_lock(&vq->mutex);

	if (!vq->private_data)
		goto out;

	/* Avoid further vmexits, we're already processing the virtqueue */
	vhost_disable_notify(&vsock->dev, vq);

	for (;;) {
		struct virtio_vsock_pkt *pkt;
		struct iov_iter iov_iter;
		unsigned out, in;
		size_t nbytes;
		size_t len;
		int head;

		spin_lock_bh(&vsock->send_pkt_list_lock);
		if (list_empty(&vsock->send_pkt_list)) {
			spin_unlock_bh(&vsock->send_pkt_list_lock);
			vhost_enable_notify(&vsock->dev, vq);
			break;
		}

		pkt = list_first_entry(&vsock->send_pkt_list,
				       struct virtio_vsock_pkt, list);
		list_del_init(&pkt->list);
		spin_unlock_bh(&vsock->send_pkt_list_lock);

		head = vhost_get_vq_desc(vq, vq->iov, ARRAY_SIZE(vq->iov),
					 &out, &in, NULL, NULL);
		if (head < 0) {
			spin_lock_bh(&vsock->send_pkt_list_lock);
			list_add(&pkt->list, &vsock->send_pkt_list);
			spin_unlock_bh(&vsock->send_pkt_list_lock);
			break;
		}

		if (head == vq->num) {
			spin_lock_bh(&vsock->send_pkt_list_lock);
			list_add(&pkt->list, &vsock->send_pkt_list);
			spin_unlock_bh(&vsock->send_pkt_list_lock);

			/* We cannot finish yet if more buffers snuck in while
			 * re-enabling notify.
			 */
			if (unlikely(vhost_enable_notify(&vsock->dev, vq))) {
				vhost_disable_notify(&vsock->dev, vq);
				continue;
			}
			break;
		}

		if (out) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Expected 0 output buffers, got %u\n", out);
			break;
		}

		len = iov_length(&vq->iov[out], in);
		iov_iter_init(&iov_iter, READ, &vq->iov[out], in, len);

		nbytes = copy_to_iter(&pkt->hdr, sizeof(pkt->hdr), &iov_iter);
		if (nbytes != sizeof(pkt->hdr)) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Faulted on copying pkt hdr\n");
			break;
		}

		nbytes = copy_to_iter(pkt->buf, pkt->len, &iov_iter);
		if (nbytes != pkt->len) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Faulted on copying pkt buf\n");
			break;
		}

		vhost_add_used(vq, head, sizeof(pkt->hdr) + pkt->len);
		added = true;

		if (pkt->reply) {
			int val;

			val = atomic_dec_return(&vsock->queued_replies);

			/* Do we have resources to resume tx processing? */
			if (val + 1 == tx_vq->num)
				restart_tx = true;
		}

		/* Deliver to monitoring devices all correctly transmitted
		 * packets.
		 */
		virtio_transport_deliver_tap_pkt(pkt);

		virtio_transport_free_pkt(pkt);
	}
	if (added)
		vhost_signal(&vsock->dev, vq);

out:
	mutex_unlock(&vq->mutex);

	if (restart_tx)
		vhost_poll_queue(&tx_vq->poll);
}
{
	struct vhost_virtqueue *tx_vq = &vsock->vqs[VSOCK_VQ_TX];
	bool added = false;
	bool restart_tx = false;

	mutex_lock(&vq->mutex);

	if (!vq->private_data)
		goto out;

	/* Avoid further vmexits, we're already processing the virtqueue */
	vhost_disable_notify(&vsock->dev, vq);

	for (;;) {
		struct virtio_vsock_pkt *pkt;
		struct iov_iter iov_iter;
		unsigned out, in;
		size_t nbytes;
		size_t len;
		int head;

		spin_lock_bh(&vsock->send_pkt_list_lock);
		if (list_empty(&vsock->send_pkt_list)) {
			spin_unlock_bh(&vsock->send_pkt_list_lock);
			vhost_enable_notify(&vsock->dev, vq);
			break;
		}

		pkt = list_first_entry(&vsock->send_pkt_list,
				       struct virtio_vsock_pkt, list);
		list_del_init(&pkt->list);
		spin_unlock_bh(&vsock->send_pkt_list_lock);

		head = vhost_get_vq_desc(vq, vq->iov, ARRAY_SIZE(vq->iov),
					 &out, &in, NULL, NULL);
		if (head < 0) {
			spin_lock_bh(&vsock->send_pkt_list_lock);
			list_add(&pkt->list, &vsock->send_pkt_list);
			spin_unlock_bh(&vsock->send_pkt_list_lock);
			break;
		}

		if (head == vq->num) {
			spin_lock_bh(&vsock->send_pkt_list_lock);
			list_add(&pkt->list, &vsock->send_pkt_list);
			spin_unlock_bh(&vsock->send_pkt_list_lock);

			/* We cannot finish yet if more buffers snuck in while
			 * re-enabling notify.
			 */
			if (unlikely(vhost_enable_notify(&vsock->dev, vq))) {
				vhost_disable_notify(&vsock->dev, vq);
				continue;
			}
			break;
		}

		if (out) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Expected 0 output buffers, got %u\n", out);
			break;
		}

		len = iov_length(&vq->iov[out], in);
		iov_iter_init(&iov_iter, READ, &vq->iov[out], in, len);

		nbytes = copy_to_iter(&pkt->hdr, sizeof(pkt->hdr), &iov_iter);
		if (nbytes != sizeof(pkt->hdr)) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Faulted on copying pkt hdr\n");
			break;
		}

		nbytes = copy_to_iter(pkt->buf, pkt->len, &iov_iter);
		if (nbytes != pkt->len) {
			virtio_transport_free_pkt(pkt);
			vq_err(vq, "Faulted on copying pkt buf\n");
			break;
		}

		vhost_add_used(vq, head, sizeof(pkt->hdr) + pkt->len);
		added = true;

		if (pkt->reply) {
			int val;

			val = atomic_dec_return(&vsock->queued_replies);

			/* Do we have resources to resume tx processing? */
			if (val + 1 == tx_vq->num)
				restart_tx = true;
		}

		/* Deliver to monitoring devices all correctly transmitted
		 * packets.
		 */
		virtio_transport_deliver_tap_pkt(pkt);

		virtio_transport_free_pkt(pkt);
	}
		if (pkt->reply) {
			int val;

			val = atomic_dec_return(&vsock->queued_replies);

			/* Do we have resources to resume tx processing? */
			if (val + 1 == tx_vq->num)
				restart_tx = true;
		}

		/* Deliver to monitoring devices all correctly transmitted
		 * packets.
		 */
		virtio_transport_deliver_tap_pkt(pkt);

		virtio_transport_free_pkt(pkt);
	}
	if (added)
		vhost_signal(&vsock->dev, vq);

out:
	mutex_unlock(&vq->mutex);

	if (restart_tx)
		vhost_poll_queue(&tx_vq->poll);
}

static void vhost_transport_send_pkt_work(struct vhost_work *work)
{
	struct vhost_virtqueue *vq;
	struct vhost_vsock *vsock;

	vsock = container_of(work, struct vhost_vsock, send_pkt_work);
	vq = &vsock->vqs[VSOCK_VQ_RX];

	vhost_transport_do_send_pkt(vsock, vq);
}

static int
vhost_transport_send_pkt(struct virtio_vsock_pkt *pkt)
{
	struct vhost_vsock *vsock;
	int len = pkt->len;

	rcu_read_lock();

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(le64_to_cpu(pkt->hdr.dst_cid));
	if (!vsock) {
		rcu_read_unlock();
		virtio_transport_free_pkt(pkt);
		return -ENODEV;
	}

	if (pkt->reply)
		atomic_inc(&vsock->queued_replies);

	spin_lock_bh(&vsock->send_pkt_list_lock);
	list_add_tail(&pkt->list, &vsock->send_pkt_list);
	spin_unlock_bh(&vsock->send_pkt_list_lock);

	vhost_work_queue(&vsock->dev, &vsock->send_pkt_work);

	rcu_read_unlock();
	return len;
}

static int
vhost_transport_cancel_pkt(struct vsock_sock *vsk)
{
	struct vhost_vsock *vsock;
	struct virtio_vsock_pkt *pkt, *n;
	int cnt = 0;
	int ret = -ENODEV;
	LIST_HEAD(freeme);

	rcu_read_lock();

	/* Find the vhost_vsock according to guest context id  */
	vsock = vhost_vsock_get(vsk->remote_addr.svm_cid);
	if (!vsock)
		goto out;

	spin_lock_bh(&vsock->send_pkt_list_lock);
	list_for_each_entry_safe(pkt, n, &vsock->send_pkt_list, list) {
		if (pkt->vsk != vsk)
			continue;
		list_move(&pkt->list, &freeme);
	}
	spin_unlock_bh(&vsock->send_pkt_list_lock);

	list_for_each_entry_safe(pkt, n, &freeme, list) {
		if (pkt->reply)
			cnt++;
		list_del(&pkt->list);
		virtio_transport_free_pkt(pkt);
	}

	if (cnt) {
		struct vhost_virtqueue *tx_vq = &vsock->vqs[VSOCK_VQ_TX];
		int new_cnt;

		new_cnt = atomic_sub_return(cnt, &vsock->queued_replies);
		if (new_cnt + cnt >= tx_vq->num && new_cnt < tx_vq->num)
			vhost_poll_queue(&tx_vq->poll);
	}

	ret = 0;
out:
	rcu_read_unlock();
	return ret;
}

static struct virtio_vsock_pkt *
vhost_vsock_alloc_pkt(struct vhost_virtqueue *vq,
		      unsigned int out, unsigned int in)
{
	struct virtio_vsock_pkt *pkt;
	struct iov_iter iov_iter;
	size_t nbytes;
	size_t len;

	if (in != 0) {
		vq_err(vq, "Expected 0 input buffers, got %u\n", in);
		return NULL;
	}

	pkt = kzalloc(sizeof(*pkt), GFP_KERNEL);
	if (!pkt)
		return NULL;

	len = iov_length(vq->iov, out);
	iov_iter_init(&iov_iter, WRITE, vq->iov, out, len);

	nbytes = copy_from_iter(&pkt->hdr, sizeof(pkt->hdr), &iov_iter);
	if (nbytes != sizeof(pkt->hdr)) {
		vq_err(vq, "Expected %zu bytes for pkt->hdr, got %zu bytes\n",
		       sizeof(pkt->hdr), nbytes);
		kfree(pkt);
		return NULL;
	}

	if (le16_to_cpu(pkt->hdr.type) == VIRTIO_VSOCK_TYPE_STREAM)
		pkt->len = le32_to_cpu(pkt->hdr.len);

	/* No payload */
	if (!pkt->len)
		return pkt;

	/* The pkt is too big */
	if (pkt->len > VIRTIO_VSOCK_MAX_PKT_BUF_SIZE) {
		kfree(pkt);
		return NULL;
	}

	pkt->buf = kmalloc(pkt->len, GFP_KERNEL);
	if (!pkt->buf) {
		kfree(pkt);
		return NULL;
	}

	nbytes = copy_from_iter(pkt->buf, pkt->len, &iov_iter);
	if (nbytes != pkt->len) {
		vq_err(vq, "Expected %u byte payload, got %zu bytes\n",
		       pkt->len, nbytes);
		virtio_transport_free_pkt(pkt);
		return NULL;
	}

	return pkt;
}

/* Is there space left for replies to rx packets? */
static bool vhost_vsock_more_replies(struct vhost_vsock *vsock)
{
	struct vhost_virtqueue *vq = &vsock->vqs[VSOCK_VQ_TX];
	int val;

	smp_rmb(); /* paired with atomic_inc() and atomic_dec_return() */
	val = atomic_read(&vsock->queued_replies);

	return val < vq->num;
}

static void vhost_vsock_handle_tx_kick(struct vhost_work *work)
{
	struct vhost_virtqueue *vq = container_of(work, struct vhost_virtqueue,
						  poll.work);
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);
	struct virtio_vsock_pkt *pkt;
	int head;
	unsigned int out, in;
	bool added = false;

	mutex_lock(&vq->mutex);

	if (!vq->private_data)
		goto out;

	vhost_disable_notify(&vsock->dev, vq);
	for (;;) {
		u32 len;

		if (!vhost_vsock_more_replies(vsock)) {
			/* Stop tx until the device processes already
			 * pending replies.  Leave tx virtqueue
			 * callbacks disabled.
			 */
			goto no_more_replies;
		}
{
	struct vhost_virtqueue *vq = container_of(work, struct vhost_virtqueue,
						  poll.work);
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);
	struct virtio_vsock_pkt *pkt;
	int head;
	unsigned int out, in;
	bool added = false;

	mutex_lock(&vq->mutex);

	if (!vq->private_data)
		goto out;

	vhost_disable_notify(&vsock->dev, vq);
	for (;;) {
		u32 len;

		if (!vhost_vsock_more_replies(vsock)) {
			/* Stop tx until the device processes already
			 * pending replies.  Leave tx virtqueue
			 * callbacks disabled.
			 */
			goto no_more_replies;
		}

		head = vhost_get_vq_desc(vq, vq->iov, ARRAY_SIZE(vq->iov),
					 &out, &in, NULL, NULL);
		if (head < 0)
			break;

		if (head == vq->num) {
			if (unlikely(vhost_enable_notify(&vsock->dev, vq))) {
				vhost_disable_notify(&vsock->dev, vq);
				continue;
			}
			break;
		}

		pkt = vhost_vsock_alloc_pkt(vq, out, in);
		if (!pkt) {
			vq_err(vq, "Faulted on pkt\n");
			continue;
		}

		len = pkt->len;

		/* Deliver to monitoring devices all received packets */
		virtio_transport_deliver_tap_pkt(pkt);

		/* Only accept correctly addressed packets */
		if (le64_to_cpu(pkt->hdr.src_cid) == vsock->guest_cid)
			virtio_transport_recv_pkt(pkt);
		else
			virtio_transport_free_pkt(pkt);

		vhost_add_used(vq, head, sizeof(pkt->hdr) + len);
		added = true;
	}
{
	struct vhost_virtqueue *vq = container_of(work, struct vhost_virtqueue,
						  poll.work);
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);
	struct virtio_vsock_pkt *pkt;
	int head;
	unsigned int out, in;
	bool added = false;

	mutex_lock(&vq->mutex);

	if (!vq->private_data)
		goto out;

	vhost_disable_notify(&vsock->dev, vq);
	for (;;) {
		u32 len;

		if (!vhost_vsock_more_replies(vsock)) {
			/* Stop tx until the device processes already
			 * pending replies.  Leave tx virtqueue
			 * callbacks disabled.
			 */
			goto no_more_replies;
		}

		head = vhost_get_vq_desc(vq, vq->iov, ARRAY_SIZE(vq->iov),
					 &out, &in, NULL, NULL);
		if (head < 0)
			break;

		if (head == vq->num) {
			if (unlikely(vhost_enable_notify(&vsock->dev, vq))) {
				vhost_disable_notify(&vsock->dev, vq);
				continue;
			}
			break;
		}

		pkt = vhost_vsock_alloc_pkt(vq, out, in);
		if (!pkt) {
			vq_err(vq, "Faulted on pkt\n");
			continue;
		}

		len = pkt->len;

		/* Deliver to monitoring devices all received packets */
		virtio_transport_deliver_tap_pkt(pkt);

		/* Only accept correctly addressed packets */
		if (le64_to_cpu(pkt->hdr.src_cid) == vsock->guest_cid)
			virtio_transport_recv_pkt(pkt);
		else
			virtio_transport_free_pkt(pkt);

		vhost_add_used(vq, head, sizeof(pkt->hdr) + len);
		added = true;
	}
		if (!pkt) {
			vq_err(vq, "Faulted on pkt\n");
			continue;
		}

		len = pkt->len;

		/* Deliver to monitoring devices all received packets */
		virtio_transport_deliver_tap_pkt(pkt);

		/* Only accept correctly addressed packets */
		if (le64_to_cpu(pkt->hdr.src_cid) == vsock->guest_cid)
			virtio_transport_recv_pkt(pkt);
		else
			virtio_transport_free_pkt(pkt);

		vhost_add_used(vq, head, sizeof(pkt->hdr) + len);
		added = true;
	}

no_more_replies:
	if (added)
		vhost_signal(&vsock->dev, vq);

out:
	mutex_unlock(&vq->mutex);
}

static void vhost_vsock_handle_rx_kick(struct vhost_work *work)
{
	struct vhost_virtqueue *vq = container_of(work, struct vhost_virtqueue,
						poll.work);
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);

	vhost_transport_do_send_pkt(vsock, vq);
}

static int vhost_vsock_start(struct vhost_vsock *vsock)
{
	struct vhost_virtqueue *vq;
	size_t i;
	int ret;

	mutex_lock(&vsock->dev.mutex);

	ret = vhost_dev_check_owner(&vsock->dev);
	if (ret)
		goto err;

	for (i = 0; i < ARRAY_SIZE(vsock->vqs); i++) {
		vq = &vsock->vqs[i];

		mutex_lock(&vq->mutex);

		if (!vhost_vq_access_ok(vq)) {
			ret = -EFAULT;
			goto err_vq;
		}