	 */
	dev_net_set(dev, net);

	mutex_lock(&pn->all_ppp_mutex);

	if (unit < 0) {
		unit = unit_get(&pn->units_idr, ppp);
		if (unit < 0) {
			ret = unit;
			goto out2;
		}
	} else {
		ret = -EEXIST;
		if (unit_find(&pn->units_idr, unit))
			goto out2; /* unit already exists */
		/*
		 * if caller need a specified unit number
		ppp->closing = 1;
		ppp_unlock(ppp);
		unregister_netdev(ppp->dev);
		unit_put(&pn->units_idr, ppp->file.index);
	} else
		ppp_unlock(ppp);

	ppp->file.dead = 1;
	ppp->owner = NULL;
	wake_up_interruptible(&ppp->file.rwait);

 * by holding all_ppp_mutex
 */

static int __unit_alloc(struct idr *p, void *ptr, int n)
{
	int unit, err;

again:
	}

	err = idr_get_new_above(p, ptr, n, &unit);
	if (err < 0) {
		if (err == -EAGAIN)
			goto again;
		return err;
	}

	return unit;
}

/* associate pointer with specified number */
static int unit_set(struct idr *p, void *ptr, int n)
{
	int unit;

	unit = __unit_alloc(p, ptr, n);
	if (unit < 0)
		return unit;
	else if (unit != n) {
		idr_remove(p, unit);
		return -EINVAL;
	}

/* get new free unit number and associate pointer with it */
static int unit_get(struct idr *p, void *ptr)
{
	return __unit_alloc(p, ptr, 0);
}

/* put unit number back to a pool */
static void unit_put(struct idr *p, int n)