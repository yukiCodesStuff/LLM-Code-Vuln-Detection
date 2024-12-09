			    struct vhost_virtqueue *vq)
{
	struct vhost_virtqueue *tx_vq = &vsock->vqs[VSOCK_VQ_TX];
	bool added = false;
	bool restart_tx = false;

	mutex_lock(&vq->mutex);
	/* Avoid further vmexits, we're already processing the virtqueue */
	vhost_disable_notify(&vsock->dev, vq);

	for (;;) {
		struct virtio_vsock_pkt *pkt;
		struct iov_iter iov_iter;
		unsigned out, in;
		size_t nbytes;
		 */
		virtio_transport_deliver_tap_pkt(pkt);

		virtio_transport_free_pkt(pkt);
	}
	if (added)
		vhost_signal(&vsock->dev, vq);

out:
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);
	struct virtio_vsock_pkt *pkt;
	int head;
	unsigned int out, in;
	bool added = false;

	mutex_lock(&vq->mutex);
		goto out;

	vhost_disable_notify(&vsock->dev, vq);
	for (;;) {
		u32 len;

		if (!vhost_vsock_more_replies(vsock)) {
			/* Stop tx until the device processes already
		else
			virtio_transport_free_pkt(pkt);

		vhost_add_used(vq, head, sizeof(pkt->hdr) + len);
		added = true;
	}

no_more_replies:
	if (added)
		vhost_signal(&vsock->dev, vq);