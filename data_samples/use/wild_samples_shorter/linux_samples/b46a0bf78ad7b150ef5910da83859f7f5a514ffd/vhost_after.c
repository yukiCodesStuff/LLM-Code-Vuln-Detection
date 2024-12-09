		vq->indirect = kmalloc_array(UIO_MAXIOV,
					     sizeof(*vq->indirect),
					     GFP_KERNEL);
		vq->log = kmalloc_array(dev->iov_limit, sizeof(*vq->log),
					GFP_KERNEL);
		vq->heads = kmalloc_array(dev->iov_limit, sizeof(*vq->heads),
					  GFP_KERNEL);
		if (!vq->indirect || !vq->log || !vq->heads)
			goto err_nomem;
	}
}

void vhost_dev_init(struct vhost_dev *dev,
		    struct vhost_virtqueue **vqs, int nvqs, int iov_limit)
{
	struct vhost_virtqueue *vq;
	int i;

	dev->iotlb = NULL;
	dev->mm = NULL;
	dev->worker = NULL;
	dev->iov_limit = iov_limit;
	init_llist_head(&dev->work_list);
	init_waitqueue_head(&dev->wait);
	INIT_LIST_HEAD(&dev->read_list);
	INIT_LIST_HEAD(&dev->pending_list);