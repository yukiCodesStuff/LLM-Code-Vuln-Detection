	struct iov_iter in_iter, prot_iter, data_iter;
	u64 tag;
	u32 exp_data_len, data_direction;
	int ret, prot_bytes;
	u16 lun;
	u8 task_attr;
	bool t10_pi = vhost_has_feature(vq, VIRTIO_SCSI_F_T10_PI);
	void *cdb;

	vhost_disable_notify(&vs->dev, vq);

	for (;;) {
		ret = vhost_scsi_get_desc(vs, vq, &vc);
		if (ret)
			goto err;

			break;
		else if (ret == -EIO)
			vhost_scsi_send_bad_target(vs, vq, vc.head, vc.out);
	}
out:
	mutex_unlock(&vq->mutex);
}

	} v_req;
	struct vhost_scsi_ctx vc;
	size_t typ_size;
	int ret;

	mutex_lock(&vq->mutex);
	/*
	 * We can handle the vq only after the endpoint is setup by calling the

	vhost_disable_notify(&vs->dev, vq);

	for (;;) {
		ret = vhost_scsi_get_desc(vs, vq, &vc);
		if (ret)
			goto err;

			break;
		else if (ret == -EIO)
			vhost_scsi_send_bad_target(vs, vq, vc.head, vc.out);
	}
out:
	mutex_unlock(&vq->mutex);
}
