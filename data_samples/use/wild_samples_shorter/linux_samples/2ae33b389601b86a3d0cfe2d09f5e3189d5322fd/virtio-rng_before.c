{
	int err;

	/* We expect a single virtqueue. */
	vq = virtio_find_single_vq(vdev, random_recv_done, "input");
	if (IS_ERR(vq))
		return PTR_ERR(vq);

	err = hwrng_register(&virtio_hwrng);
	if (err) {
		vdev->config->del_vqs(vdev);
		return err;
	}

	return 0;
	busy = false;
	hwrng_unregister(&virtio_hwrng);
	vdev->config->del_vqs(vdev);
}

static int virtrng_probe(struct virtio_device *vdev)
{