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
			const void *buf, unsigned int len)
{
	/* Now we only support read-only configuration space */
}

static int vduse_vdpa_reset(struct vdpa_device *vdpa)
{
	struct vduse_dev *dev = vdpa_to_vduse(vdpa);
	int ret = vduse_dev_set_status(dev, 0);

	vduse_dev_reset(dev);

	return ret;
}

static u32 vduse_vdpa_get_generation(struct vdpa_device *vdpa)
{
	struct vduse_dev *dev = vdpa_to_vduse(vdpa);

	return dev->generation;
}

static int vduse_vdpa_set_map(struct vdpa_device *vdpa,
				unsigned int asid,
				struct vhost_iotlb *iotlb)
{
	struct vduse_dev *dev = vdpa_to_vduse(vdpa);
	int ret;

	ret = vduse_domain_set_map(dev->domain, iotlb);
	if (ret)
		return ret;

	ret = vduse_dev_update_iotlb(dev, 0ULL, ULLONG_MAX);
	if (ret) {
		vduse_domain_clear_map(dev->domain, iotlb);
		return ret;
	}