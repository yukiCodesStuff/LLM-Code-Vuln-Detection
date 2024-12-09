			r = -ENOBUFS;
			goto err;
		}
		d = vhost_get_vq_desc(vq->dev, vq, vq->iov + seg,
				      ARRAY_SIZE(vq->iov) - seg, &out,
				      &in, log, log_num);
		if (d == vq->num) {
			r = 0;
			goto err;
		}