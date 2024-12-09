{
	struct idxd_wq *wq = confdev_to_wq(dev);
	int rc;
	unsigned int retries;

	if (wq_dedicated(wq))
		return -EOPNOTSUPP;

	rc = kstrtouint(buf, 10, &retries);
	if (rc < 0)
		return rc;

	if (retries > IDXD_ENQCMDS_MAX_RETRIES)
		retries = IDXD_ENQCMDS_MAX_RETRIES;

	wq->enqcmds_retries = retries;
	return count;
}

static struct device_attribute dev_attr_wq_enqcmds_retries =
		__ATTR(enqcmds_retries, 0644, wq_enqcmds_retries_show, wq_enqcmds_retries_store);

static ssize_t op_cap_show_common(struct device *dev, char *buf, unsigned long *opcap_bmap)
{
	ssize_t pos;
	int i;

	pos = 0;
	for (i = IDXD_MAX_OPCAP_BITS/64 - 1; i >= 0; i--) {
		unsigned long val = opcap_bmap[i];

		/* On systems where direct user submissions are not safe, we need to clear out
		 * the BATCH capability from the capability mask in sysfs since we cannot support
		 * that command on such systems.
		 */
		if (i == DSA_OPCODE_BATCH/64 && !confdev_to_idxd(dev)->user_submission_safe)
			clear_bit(DSA_OPCODE_BATCH % 64, &val);

		pos += sysfs_emit_at(buf, pos, "%*pb", 64, &val);
		pos += sysfs_emit_at(buf, pos, "%c", i == 0 ? '\n' : ',');
	}

	return pos;
}
{
	struct idxd_device *idxd = confdev_to_idxd(dev);

	return op_cap_show_common(dev, buf, idxd->opcap_bmap);
}
static DEVICE_ATTR_RO(op_cap);

static ssize_t gen_cap_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct idxd_device *idxd = confdev_to_idxd(dev);

	return sysfs_emit(buf, "%#llx\n", idxd->hw.gen_cap.bits);
}
static DEVICE_ATTR_RO(gen_cap);

static ssize_t configurable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct idxd_device *idxd = confdev_to_idxd(dev);

	return sysfs_emit(buf, "%u\n", test_bit(IDXD_FLAG_CONFIGURABLE, &idxd->flags));
}
static DEVICE_ATTR_RO(configurable);

static ssize_t clients_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct idxd_device *idxd = confdev_to_idxd(dev);
	int count = 0, i;

	spin_lock(&idxd->dev_lock);
	for (i = 0; i < idxd->max_wqs; i++) {
		struct idxd_wq *wq = idxd->wqs[i];

		count += wq->client_count;
	}