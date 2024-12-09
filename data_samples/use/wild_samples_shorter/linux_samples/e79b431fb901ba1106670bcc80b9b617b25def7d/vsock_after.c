			    struct vhost_virtqueue *vq)
{
	struct vhost_virtqueue *tx_vq = &vsock->vqs[VSOCK_VQ_TX];
	int pkts = 0, total_len = 0;
	bool added = false;
	bool restart_tx = false;

	mutex_lock(&vq->mutex);
	/* Avoid further vmexits, we're already processing the virtqueue */
	vhost_disable_notify(&vsock->dev, vq);

	do {
		struct virtio_vsock_pkt *pkt;
		struct iov_iter iov_iter;
		unsigned out, in;
		size_t nbytes;
		 */
		virtio_transport_deliver_tap_pkt(pkt);

		total_len += pkt->len;
		virtio_transport_free_pkt(pkt);
	} while(likely(!vhost_exceeds_weight(vq, ++pkts, total_len)));
	if (added)
		vhost_signal(&vsock->dev, vq);

out:
	struct vhost_vsock *vsock = container_of(vq->dev, struct vhost_vsock,
						 dev);
	struct virtio_vsock_pkt *pkt;
	int head, pkts = 0, total_len = 0;
	unsigned int out, in;
	bool added = false;

	mutex_lock(&vq->mutex);
		goto out;

	vhost_disable_notify(&vsock->dev, vq);
	do {
		u32 len;

		if (!vhost_vsock_more_replies(vsock)) {
			/* Stop tx until the device processes already
		else
			virtio_transport_free_pkt(pkt);

		len += sizeof(pkt->hdr);
		vhost_add_used(vq, head, len);
		total_len += len;
		added = true;
	} while(likely(!vhost_exceeds_weight(vq, ++pkts, total_len)));

no_more_replies:
	if (added)
		vhost_signal(&vsock->dev, vq);