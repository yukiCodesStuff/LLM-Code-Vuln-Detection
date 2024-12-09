{
	if (!dev->power.early_init) {
		spin_lock_init(&dev->power.lock);
		dev->power.power_state = PMSG_INVALID;
		dev->power.early_init = true;
	}
}


static inline void device_pm_sleep_init(struct device *dev) {}

static inline void device_pm_add(struct device *dev)
{
	dev_pm_qos_constraints_init(dev);
}

static inline void device_pm_remove(struct device *dev)
{
	dev_pm_qos_constraints_destroy(dev);
	pm_runtime_remove(dev);
}

static inline void device_pm_move_before(struct device *deva,