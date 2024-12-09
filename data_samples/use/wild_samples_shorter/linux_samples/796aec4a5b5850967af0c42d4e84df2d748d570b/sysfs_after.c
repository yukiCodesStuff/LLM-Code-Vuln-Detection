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

static ssize_t wq_op_config_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct idxd_wq *wq = confdev_to_wq(dev);

	return op_cap_show_common(dev, buf, wq->opcap_bmap);
}

static int idxd_verify_supported_opcap(struct idxd_device *idxd, unsigned long *opmask)
{
{
	struct idxd_device *idxd = confdev_to_idxd(dev);

	return op_cap_show_common(dev, buf, idxd->opcap_bmap);
}
static DEVICE_ATTR_RO(op_cap);

static ssize_t gen_cap_show(struct device *dev,