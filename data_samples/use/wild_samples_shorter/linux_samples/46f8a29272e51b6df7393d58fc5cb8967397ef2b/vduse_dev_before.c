{
	struct vduse_dev *dev = vdpa_to_vduse(vdpa);

	if (offset > dev->config_size ||
	    len > dev->config_size - offset)
		return;

	memcpy(buf, dev->config + offset, len);
}

static void vduse_vdpa_set_config(struct vdpa_device *vdpa, unsigned int offset,