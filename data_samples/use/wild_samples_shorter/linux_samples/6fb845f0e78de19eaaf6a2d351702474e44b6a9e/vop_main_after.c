 * @dc: Virtio device control
 * @vpdev: VOP device which is the parent for this virtio device
 * @vr: Buffer for accessing the VRING
 * @used_virt: Virtual address of used ring
 * @used: DMA address of used ring
 * @used_size: Size of the used buffer
 * @reset_done: Track whether VOP reset is complete
 * @virtio_cookie: Cookie returned upon requesting a interrupt
 * @c2h_vdev_db: The doorbell used by the guest to interrupt the host
	struct mic_device_ctrl __iomem *dc;
	struct vop_device *vpdev;
	void __iomem *vr[VOP_MAX_VRINGS];
	void *used_virt[VOP_MAX_VRINGS];
	dma_addr_t used[VOP_MAX_VRINGS];
	int used_size[VOP_MAX_VRINGS];
	struct completion reset_done;
	struct mic_irq *virtio_cookie;
static void vop_del_vq(struct virtqueue *vq, int n)
{
	struct _vop_vdev *vdev = to_vopvdev(vq->vdev);
	struct vop_device *vpdev = vdev->vpdev;

	dma_unmap_single(&vpdev->dev, vdev->used[n],
			 vdev->used_size[n], DMA_BIDIRECTIONAL);
	free_pages((unsigned long)vdev->used_virt[n],
		   get_order(vdev->used_size[n]));
	vring_del_virtqueue(vq);
	vpdev->hw_ops->iounmap(vpdev, vdev->vr[n]);
	vdev->vr[n] = NULL;
}
		vop_del_vq(vq, idx++);
}

static struct virtqueue *vop_new_virtqueue(unsigned int index,
				      unsigned int num,
				      struct virtio_device *vdev,
				      bool context,
				      void *pages,
				      bool (*notify)(struct virtqueue *vq),
				      void (*callback)(struct virtqueue *vq),
				      const char *name,
				      void *used)
{
	bool weak_barriers = false;
	struct vring vring;

	vring_init(&vring, num, pages, MIC_VIRTIO_RING_ALIGN);
	vring.used = used;

	return __vring_new_virtqueue(index, vring, vdev, weak_barriers, context,
				     notify, callback, name);
}

/*
 * This routine will assign vring's allocated in host/io memory. Code in
 * virtio_ring.c however continues to access this io memory as if it were local
 * memory without io accessors.
	struct _mic_vring_info __iomem *info;
	void *used;
	int vr_size, _vr_size, err, magic;
	u8 type = ioread8(&vdev->desc->type);

	if (index >= ioread8(&vdev->desc->num_vq))
		return ERR_PTR(-ENOENT);
		return ERR_PTR(-ENOMEM);
	vdev->vr[index] = va;
	memset_io(va, 0x0, _vr_size);

	info = va + _vr_size;
	magic = ioread32(&info->magic);

	if (WARN(magic != MIC_MAGIC + type + index, "magic mismatch")) {
		goto unmap;
	}

	vdev->used_size[index] = PAGE_ALIGN(sizeof(__u16) * 3 +
					     sizeof(struct vring_used_elem) *
					     le16_to_cpu(config.num));
	used = (void *)__get_free_pages(GFP_KERNEL | __GFP_ZERO,
					get_order(vdev->used_size[index]));
	vdev->used_virt[index] = used;
	if (!used) {
		err = -ENOMEM;
		dev_err(_vop_dev(vdev), "%s %d err %d\n",
			__func__, __LINE__, err);
		goto unmap;
	}

	vq = vop_new_virtqueue(index, le16_to_cpu(config.num), dev, ctx,
			       (void __force *)va, vop_notify, callback,
			       name, used);
	if (!vq) {
		err = -ENOMEM;
		goto free_used;
	}

	vdev->used[index] = dma_map_single(&vpdev->dev, used,
					    vdev->used_size[index],
					    DMA_BIDIRECTIONAL);
	if (dma_mapping_error(&vpdev->dev, vdev->used[index])) {
		err = -ENOMEM;
		dev_err(_vop_dev(vdev), "%s %d err %d\n",
			__func__, __LINE__, err);
		goto del_vq;
	}
	writeq(vdev->used[index], &vqconfig->used_address);

	vq->priv = vdev;
	return vq;
del_vq:
	vring_del_virtqueue(vq);
free_used:
	free_pages((unsigned long)used,
		   get_order(vdev->used_size[index]));
unmap:
	vpdev->hw_ops->iounmap(vpdev, vdev->vr[index]);
	return ERR_PTR(err);
}
	int ret = -1;

	if (ioread8(&dc->config_change) == MIC_VIRTIO_PARAM_DEV_REMOVE) {
		struct device *dev = get_device(&vdev->vdev.dev);

		dev_dbg(&vpdev->dev,
			"%s %d config_change %d type %d vdev %p\n",
			__func__, __LINE__,
			ioread8(&dc->config_change), ioread8(&d->type), vdev);
		iowrite8(-1, &dc->h2c_vdev_db);
		if (status & VIRTIO_CONFIG_S_DRIVER_OK)
			wait_for_completion(&vdev->reset_done);
		put_device(dev);
		iowrite8(1, &dc->guest_ack);
		dev_dbg(&vpdev->dev, "%s %d guest_ack %d\n",
			__func__, __LINE__, ioread8(&dc->guest_ack));
		iowrite8(-1, &d->type);