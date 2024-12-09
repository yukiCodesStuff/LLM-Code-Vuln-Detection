static struct device_attribute dev_attr_wq_enqcmds_retries =
		__ATTR(enqcmds_retries, 0644, wq_enqcmds_retries_show, wq_enqcmds_retries_store);

static ssize_t wq_op_config_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct idxd_wq *wq = confdev_to_wq(dev);

	return sysfs_emit(buf, "%*pb\n", IDXD_MAX_OPCAP_BITS, wq->opcap_bmap);
}

static int idxd_verify_supported_opcap(struct idxd_device *idxd, unsigned long *opmask)
{
{
	struct idxd_device *idxd = confdev_to_idxd(dev);

	return sysfs_emit(buf, "%*pb\n", IDXD_MAX_OPCAP_BITS, idxd->opcap_bmap);
}
static DEVICE_ATTR_RO(op_cap);

static ssize_t gen_cap_show(struct device *dev,