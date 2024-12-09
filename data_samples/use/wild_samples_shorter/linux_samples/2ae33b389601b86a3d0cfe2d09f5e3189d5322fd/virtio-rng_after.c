{
	int err;

	if (vq) {
		/* We only support one device for now */
		return -EBUSY;
	}
	/* We expect a single virtqueue. */
	vq = virtio_find_single_vq(vdev, random_recv_done, "input");
	if (IS_ERR(vq)) {
		err = PTR_ERR(vq);
		vq = NULL;
		return err;
	}

	err = hwrng_register(&virtio_hwrng);
	if (err) {
		vdev->config->del_vqs(vdev);
		vq = NULL;
		return err;
	}

	return 0;
	busy = false;
	hwrng_unregister(&virtio_hwrng);
	vdev->config->del_vqs(vdev);
	vq = NULL;
}

static int virtrng_probe(struct virtio_device *vdev)
{