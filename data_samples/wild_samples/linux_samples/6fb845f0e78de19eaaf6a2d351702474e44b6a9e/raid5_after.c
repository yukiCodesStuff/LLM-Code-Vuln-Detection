{
	struct r5conf *conf;
	int ret = 0;
	spin_lock(&mddev->lock);
	conf = mddev->private;
	if (conf)
		ret = sprintf(page, "%d\n", conf->min_nr_stripes);
	spin_unlock(&mddev->lock);
	return ret;
}

int
raid5_set_cache_size(struct mddev *mddev, int size)
{
	int result = 0;
	struct r5conf *conf = mddev->private;

	if (size <= 16 || size > 32768)
		return -EINVAL;

	conf->min_nr_stripes = size;
	mutex_lock(&conf->cache_size_mutex);
	while (size < conf->max_nr_stripes &&
	       drop_one_stripe(conf))
		;
	mutex_unlock(&conf->cache_size_mutex);

	md_allow_write(mddev);

	mutex_lock(&conf->cache_size_mutex);
	while (size > conf->max_nr_stripes)
		if (!grow_one_stripe(conf, GFP_KERNEL)) {
			conf->min_nr_stripes = conf->max_nr_stripes;
			result = -ENOMEM;
			break;
		}
	mutex_unlock(&conf->cache_size_mutex);

	return result;
}
{
	int result = 0;
	struct r5conf *conf = mddev->private;

	if (size <= 16 || size > 32768)
		return -EINVAL;

	conf->min_nr_stripes = size;
	mutex_lock(&conf->cache_size_mutex);
	while (size < conf->max_nr_stripes &&
	       drop_one_stripe(conf))
		;
	mutex_unlock(&conf->cache_size_mutex);

	md_allow_write(mddev);

	mutex_lock(&conf->cache_size_mutex);
	while (size > conf->max_nr_stripes)
		if (!grow_one_stripe(conf, GFP_KERNEL)) {
			conf->min_nr_stripes = conf->max_nr_stripes;
			result = -ENOMEM;
			break;
		}