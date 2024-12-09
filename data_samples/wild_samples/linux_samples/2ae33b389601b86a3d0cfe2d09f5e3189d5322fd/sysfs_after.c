{
	sysfs_unmerge_group(&dev->kobj, &pm_runtime_attr_group);
}

void dpm_sysfs_remove(struct device *dev)
{
	dev_pm_qos_constraints_destroy(dev);
	rpm_sysfs_remove(dev);
	sysfs_unmerge_group(&dev->kobj, &pm_wakeup_attr_group);
	sysfs_remove_group(&dev->kobj, &pm_attr_group);
}