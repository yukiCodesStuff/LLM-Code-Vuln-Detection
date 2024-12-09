int
raid5_set_cache_size(struct mddev *mddev, int size)
{
	struct r5conf *conf = mddev->private;

	if (size <= 16 || size > 32768)
		return -EINVAL;

	mutex_lock(&conf->cache_size_mutex);
	while (size > conf->max_nr_stripes)
		if (!grow_one_stripe(conf, GFP_KERNEL))
			break;
	mutex_unlock(&conf->cache_size_mutex);

	return 0;
}
EXPORT_SYMBOL(raid5_set_cache_size);

static ssize_t