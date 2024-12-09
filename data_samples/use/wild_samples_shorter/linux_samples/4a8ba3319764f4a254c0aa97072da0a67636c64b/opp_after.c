/* Lock to allow exclusive modification to the device and opp lists */
static DEFINE_MUTEX(dev_opp_list_lock);

#define opp_rcu_lockdep_assert()					\
do {									\
	rcu_lockdep_assert(rcu_read_lock_held() ||			\
				lockdep_is_held(&dev_opp_list_lock),	\
			   "Missing rcu_read_lock() or "		\
			   "dev_opp_list_lock protection");		\
} while (0)

/**
 * find_device_opp() - find device_opp struct using device pointer
 * @dev:	device pointer used to lookup device OPPs
 *
 * This function returns the number of available opps if there are any,
 * else returns 0 if none or the corresponding error value.
 *
 * Locking: This function takes rcu_read_lock().
 */
int dev_pm_opp_get_opp_count(struct device *dev)
{
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp;
	int count = 0;

	rcu_read_lock();

	dev_opp = find_device_opp(dev);
	if (IS_ERR(dev_opp)) {
		count = PTR_ERR(dev_opp);
		dev_err(dev, "%s: device OPP not found (%d)\n",
			__func__, count);
		goto out_unlock;
	}

	list_for_each_entry_rcu(temp_opp, &dev_opp->opp_list, node) {
		if (temp_opp->available)
			count++;
	}

out_unlock:
	rcu_read_unlock();
	return count;
}
EXPORT_SYMBOL_GPL(dev_pm_opp_get_opp_count);

	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	opp_rcu_lockdep_assert();

	dev_opp = find_device_opp(dev);
	if (IS_ERR(dev_opp)) {
		int r = PTR_ERR(dev_opp);
		dev_err(dev, "%s: device OPP not found (%d)\n", __func__, r);
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	opp_rcu_lockdep_assert();

	if (!dev || !freq) {
		dev_err(dev, "%s: Invalid argument freq=%p\n", __func__, freq);
		return ERR_PTR(-EINVAL);
	}
	struct device_opp *dev_opp;
	struct dev_pm_opp *temp_opp, *opp = ERR_PTR(-ERANGE);

	opp_rcu_lockdep_assert();

	if (!dev || !freq) {
		dev_err(dev, "%s: Invalid argument freq=%p\n", __func__, freq);
		return ERR_PTR(-EINVAL);
	}

	/* Check for existing list for 'dev' */
	dev_opp = find_device_opp(dev);
	if (IS_ERR(dev_opp)) {
		int error = PTR_ERR(dev_opp);
		if (error != -ENODEV)
			WARN(1, "%s: dev_opp: %d\n",
			     IS_ERR_OR_NULL(dev) ?
					"Invalid device" : dev_name(dev),
			     error);
		return;
	}

	/* Hold our list modification lock here */
	mutex_lock(&dev_opp_list_lock);
