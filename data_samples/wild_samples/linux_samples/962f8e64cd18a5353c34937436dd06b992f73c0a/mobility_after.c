	do {
		rc = rtas_call(activate_fw_token, 0, 1, NULL);
	} while (rtas_busy_delay(rc));

	if (rc)
		printk(KERN_ERR "Post-mobility activate-fw failed: %d\n", rc);

	/*
	 * We don't want CPUs to go online/offline while the device
	 * tree is being updated.
	 */
	cpus_read_lock();

	/*
	 * It's common for the destination firmware to replace cache
	 * nodes.  Release all of the cacheinfo hierarchy's references
	 * before updating the device tree.
	 */
	cacheinfo_teardown();

	rc = pseries_devicetree_update(MIGRATION_SCOPE);
	if (rc)
		printk(KERN_ERR "Post-mobility device tree update "
			"failed: %d\n", rc);

	cacheinfo_rebuild();

	cpus_read_unlock();

	/* Possibly switch to a new L1 flush type */
	pseries_setup_security_mitigations();

	/* Reinitialise system information for hv-24x7 */
	read_24x7_sys_info();

	return;
}

static ssize_t migration_store(struct class *class,
			       struct class_attribute *attr, const char *buf,
			       size_t count)
{
	u64 streamid;
	int rc;

	rc = kstrtou64(buf, 0, &streamid);
	if (rc)
		return rc;

	do {
		rc = rtas_ibm_suspend_me(streamid);
		if (rc == -EAGAIN)
			ssleep(1);
	} while (rc == -EAGAIN);

	if (rc)
		return rc;

	post_mobility_fixup();

	return count;
}