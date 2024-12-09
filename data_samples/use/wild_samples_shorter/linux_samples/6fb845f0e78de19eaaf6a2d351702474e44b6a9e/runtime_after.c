{
	int autosuspend_delay;
	u64 last_busy, expires = 0;
	u64 now = ktime_get_mono_fast_ns();

	if (!dev->power.use_autosuspend)
		goto out;

	 * If 'expires' is after the current time, we've been called
	 * too early.
	 */
	if (expires > 0 && expires < ktime_get_mono_fast_ns()) {
		dev->power.timer_expires = 0;
		rpm_suspend(dev, dev->power.timer_autosuspends ?
		    (RPM_ASYNC | RPM_AUTO) : RPM_ASYNC);
	}
int pm_schedule_suspend(struct device *dev, unsigned int delay)
{
	unsigned long flags;
	u64 expires;
	int retval;

	spin_lock_irqsave(&dev->power.lock, flags);

	/* Other scheduled or pending requests need to be canceled. */
	pm_runtime_cancel_pending(dev);

	expires = ktime_get_mono_fast_ns() + (u64)delay * NSEC_PER_MSEC;
	dev->power.timer_expires = expires;
	dev->power.timer_autosuspends = 0;
	hrtimer_start(&dev->power.suspend_timer, expires, HRTIMER_MODE_ABS);

 out: