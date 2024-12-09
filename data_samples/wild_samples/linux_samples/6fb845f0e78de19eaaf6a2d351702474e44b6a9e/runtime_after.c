{
	int autosuspend_delay;
	u64 last_busy, expires = 0;
	u64 now = ktime_get_mono_fast_ns();

	if (!dev->power.use_autosuspend)
		goto out;

	autosuspend_delay = READ_ONCE(dev->power.autosuspend_delay);
	if (autosuspend_delay < 0)
		goto out;

	last_busy = READ_ONCE(dev->power.last_busy);

	expires = last_busy + (u64)autosuspend_delay * NSEC_PER_MSEC;
	if (expires <= now)
		expires = 0;	/* Already expired. */

 out:
	return expires;
}
EXPORT_SYMBOL_GPL(pm_runtime_autosuspend_expiration);

static int dev_memalloc_noio(struct device *dev, void *data)
{
	return dev->power.memalloc_noio;
}

/*
 * pm_runtime_set_memalloc_noio - Set a device's memalloc_noio flag.
 * @dev: Device to handle.
 * @enable: True for setting the flag and False for clearing the flag.
 *
 * Set the flag for all devices in the path from the device to the
 * root device in the device tree if @enable is true, otherwise clear
 * the flag for devices in the path whose siblings don't set the flag.
 *
 * The function should only be called by block device, or network
 * device driver for solving the deadlock problem during runtime
 * resume/suspend:
 *
 *     If memory allocation with GFP_KERNEL is called inside runtime
 *     resume/suspend callback of any one of its ancestors(or the
 *     block device itself), the deadlock may be triggered inside the
 *     memory allocation since it might not complete until the block
 *     device becomes active and the involed page I/O finishes. The
 *     situation is pointed out first by Alan Stern. Network device
 *     are involved in iSCSI kind of situation.
 *
 * The lock of dev_hotplug_mutex is held in the function for handling
 * hotplug race because pm_runtime_set_memalloc_noio() may be called
 * in async probe().
 *
 * The function should be called between device_add() and device_del()
 * on the affected device(block/network device).
 */
void pm_runtime_set_memalloc_noio(struct device *dev, bool enable)
{
	static DEFINE_MUTEX(dev_hotplug_mutex);

	mutex_lock(&dev_hotplug_mutex);
	for (;;) {
		bool enabled;

		/* hold power lock since bitfield is not SMP-safe. */
		spin_lock_irq(&dev->power.lock);
		enabled = dev->power.memalloc_noio;
		dev->power.memalloc_noio = enable;
		spin_unlock_irq(&dev->power.lock);

		/*
		 * not need to enable ancestors any more if the device
		 * has been enabled.
		 */
		if (enabled && enable)
			break;

		dev = dev->parent;

		/*
		 * clear flag of the parent device only if all the
		 * children don't set the flag because ancestor's
		 * flag was set by any one of the descendants.
		 */
		if (!dev || (!enable &&
			     device_for_each_child(dev, NULL,
						   dev_memalloc_noio)))
			break;
	}
{
	struct device *dev = container_of(timer, struct device, power.suspend_timer);
	unsigned long flags;
	u64 expires;

	spin_lock_irqsave(&dev->power.lock, flags);

	expires = dev->power.timer_expires;
	/*
	 * If 'expires' is after the current time, we've been called
	 * too early.
	 */
	if (expires > 0 && expires < ktime_get_mono_fast_ns()) {
		dev->power.timer_expires = 0;
		rpm_suspend(dev, dev->power.timer_autosuspends ?
		    (RPM_ASYNC | RPM_AUTO) : RPM_ASYNC);
	}
{
	unsigned long flags;
	u64 expires;
	int retval;

	spin_lock_irqsave(&dev->power.lock, flags);

	if (!delay) {
		retval = rpm_suspend(dev, RPM_ASYNC);
		goto out;
	}

	retval = rpm_check_suspend_allowed(dev);
	if (retval)
		goto out;

	/* Other scheduled or pending requests need to be canceled. */
	pm_runtime_cancel_pending(dev);

	expires = ktime_get_mono_fast_ns() + (u64)delay * NSEC_PER_MSEC;
	dev->power.timer_expires = expires;
	dev->power.timer_autosuspends = 0;
	hrtimer_start(&dev->power.suspend_timer, expires, HRTIMER_MODE_ABS);

 out:
	spin_unlock_irqrestore(&dev->power.lock, flags);

	return retval;
}
	if (!delay) {
		retval = rpm_suspend(dev, RPM_ASYNC);
		goto out;
	}

	retval = rpm_check_suspend_allowed(dev);
	if (retval)
		goto out;

	/* Other scheduled or pending requests need to be canceled. */
	pm_runtime_cancel_pending(dev);

	expires = ktime_get_mono_fast_ns() + (u64)delay * NSEC_PER_MSEC;
	dev->power.timer_expires = expires;
	dev->power.timer_autosuspends = 0;
	hrtimer_start(&dev->power.suspend_timer, expires, HRTIMER_MODE_ABS);

 out:
	spin_unlock_irqrestore(&dev->power.lock, flags);

	return retval;
}
EXPORT_SYMBOL_GPL(pm_schedule_suspend);

/**
 * __pm_runtime_idle - Entry point for runtime idle operations.
 * @dev: Device to send idle notification for.
 * @rpmflags: Flag bits.
 *
 * If the RPM_GET_PUT flag is set, decrement the device's usage count and
 * return immediately if it is larger than zero.  Then carry out an idle
 * notification, either synchronous or asynchronous.
 *
 * This routine may be called in atomic context if the RPM_ASYNC flag is set,
 * or if pm_runtime_irq_safe() has been called.
 */
int __pm_runtime_idle(struct device *dev, int rpmflags)
{
	unsigned long flags;
	int retval;

	if (rpmflags & RPM_GET_PUT) {
		if (!atomic_dec_and_test(&dev->power.usage_count))
			return 0;
	}