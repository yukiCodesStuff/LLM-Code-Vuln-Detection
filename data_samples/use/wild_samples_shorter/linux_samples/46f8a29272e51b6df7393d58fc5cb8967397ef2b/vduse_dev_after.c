{
	struct vduse_dev *dev = vdpa_to_vduse(vdpa);

	/* Initialize the buffer in case of partial copy. */
	memset(buf, 0, len);

	if (offset > dev->config_size)
		return;

	if (len > dev->config_size - offset)
		len = dev->config_size - offset;

	memcpy(buf, dev->config + offset, len);
}

static void vduse_vdpa_set_config(struct vdpa_device *vdpa, unsigned int offset,