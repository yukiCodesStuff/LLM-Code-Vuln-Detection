	 */
	dev_net_set(dev, net);

	ret = -EEXIST;
	mutex_lock(&pn->all_ppp_mutex);

	if (unit < 0) {
		unit = unit_get(&pn->units_idr, ppp);
		if (unit < 0) {
			*retp = unit;
			goto out2;
		}
	} else {
		if (unit_find(&pn->units_idr, unit))
			goto out2; /* unit already exists */
		/*
		 * if caller need a specified unit number
		ppp->closing = 1;
		ppp_unlock(ppp);
		unregister_netdev(ppp->dev);
	} else
		ppp_unlock(ppp);

	unit_put(&pn->units_idr, ppp->file.index);
	ppp->file.dead = 1;
	ppp->owner = NULL;
	wake_up_interruptible(&ppp->file.rwait);

 * by holding all_ppp_mutex
 */

/* associate pointer with specified number */
static int unit_set(struct idr *p, void *ptr, int n)
{
	int unit, err;

again:
	}

	err = idr_get_new_above(p, ptr, n, &unit);
	if (err == -EAGAIN)
		goto again;

	if (unit != n) {
		idr_remove(p, unit);
		return -EINVAL;
	}

/* get new free unit number and associate pointer with it */
static int unit_get(struct idr *p, void *ptr)
{
	int unit, err;

again:
	if (!idr_pre_get(p, GFP_KERNEL)) {
		printk(KERN_ERR "PPP: No free memory for idr\n");
		return -ENOMEM;
	}

	err = idr_get_new_above(p, ptr, 0, &unit);
	if (err == -EAGAIN)
		goto again;

	return unit;
}

/* put unit number back to a pool */
static void unit_put(struct idr *p, int n)