struct device_opp {
	struct list_head node;

	struct device *dev;
	struct srcu_notifier_head srcu_head;
	struct rcu_head rcu_head;
	struct list_head opp_list;
};

/*
 * The root of the list of all devices. All device_opp structures branch off
 * from here, with each device_opp containing the list of opp it supports in
 * various states of availability.
 */
static LIST_HEAD(dev_opp_list);
/* Lock to allow exclusive modification to the device and opp lists */
static DEFINE_MUTEX(dev_opp_list_lock);

/**
 * find_device_opp() - find device_opp struct using device pointer
 * @dev:	device pointer used to lookup device OPPs
 *
 * Search list of device OPPs for one containing matching device. Does a RCU
 * reader operation to grab the pointer needed.
 *
 * Returns pointer to 'struct device_opp' if found, otherwise -ENODEV or
 * -EINVAL based on type of error.
 *
 * Locking: This function must be called under rcu_read_lock(). device_opp
 * is a RCU protected pointer. This means that device_opp is valid as long
 * as we are under RCU lock.
 */
static struct device_opp *find_device_opp(struct device *dev)
{
	struct device_opp *tmp_dev_opp, *dev_opp = ERR_PTR(-ENODEV);

	if (unlikely(IS_ERR_OR_NULL(dev))) {
		pr_err("%s: Invalid parameters\n", __func__);
		return ERR_PTR(-EINVAL);
	}

	list_for_each_entry_rcu(tmp_dev_opp, &dev_opp_list, node) {
		if (tmp_dev_opp->dev == dev) {
			dev_opp = tmp_dev_opp;
			break;
		}
	}

	return dev_opp;
}
{
	struct dev_pm_opp *tmp_opp;
	unsigned long f = 0;

	tmp_opp = rcu_dereference(opp);
	if (unlikely(IS_ERR_OR_NULL(tmp_opp)) || !tmp_opp->available)
		pr_err("%s: Invalid parameters\n", __func__);
	else
		f = tmp_opp->rate;

	return f;
}
EXPORT_SYMBOL_GPL(dev_pm_opp_get_freq);

/**
 * dev_pm_opp_get_opp_count() - Get number of opps available in the opp list
 * @dev:	device for which we do this operation
 *
 * This function returns the number of available opps if there are any,
 * else returns 0 if none or the corresponding error value.
 *
 * Locking: This function must be called under rcu_read_lock(). This function
 * internally references two RCU protected structures: device_opp and opp which
 * are safe as long as we are under a common RCU locked section.
 */
int dev_pm_opp_get_opp_count(struct device *dev)
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp;
	int count = 0;

	dev_opp = find_device_opp(dev);
	if (IS_ERR(dev_opp)) {
		int r = PTR_ERR(dev_opp);
		dev_err(dev, "%s: device OPP not found (%d)\n", __func__, r);
		return r;
	}

	list_for_each_entry_rcu(temp_opp, &dev_opp->opp_list, node) {
		if (temp_opp->available)
			count++;
	}

	return count;
}
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	dev_opp = find_device_opp(dev);
	if (IS_ERR(dev_opp)) {
		int r = PTR_ERR(dev_opp);
		dev_err(dev, "%s: device OPP not found (%d)\n", __func__, r);
		return ERR_PTR(r);
	}
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	if (!dev || !freq) {
		dev_err(dev, "%s: Invalid argument freq=%p\n", __func__, freq);
		return ERR_PTR(-EINVAL);
	}
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	if (!dev || !freq) {
		dev_err(dev, "%s: Invalid argument freq=%p\n", __func__, freq);
		return ERR_PTR(-EINVAL);
	}
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *opp, *tmp;

	/* Check for existing list for 'dev' */
	dev_opp = find_device_opp(dev);
	if (WARN(IS_ERR(dev_opp), "%s: dev_opp: %ld\n", dev_name(dev),
		 PTR_ERR(dev_opp)))
		return;

	/* Hold our list modification lock here */
	mutex_lock(&dev_opp_list_lock);

	/* Free static OPPs */
	list_for_each_entry_safe(opp, tmp, &dev_opp->opp_list, node) {
		if (!opp->dynamic)
			__dev_pm_opp_remove(dev_opp, opp);
	}