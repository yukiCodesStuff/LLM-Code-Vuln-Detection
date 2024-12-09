{
	int ret = 0;
	u32 max_len;
	struct netprio_map *map;

	max_len = atomic_read(&max_prioidx) + 1;
	map = rtnl_dereference(dev->priomap);
	if (!map || map->priomap_len < max_len)
		ret = extend_netdev_table(dev, max_len);

	return ret;
}

static int update_netdev_tables(void)
{
	int ret = 0;
	struct net_device *dev;
	u32 max_len;
	struct netprio_map *map;

	rtnl_lock();
	max_len = atomic_read(&max_prioidx) + 1;
	for_each_netdev(&init_net, dev) {
		map = rtnl_dereference(dev->priomap);
		/*
		 * don't allocate priomap if we didn't
		 * change net_prio.ifpriomap (map == NULL),
		 * this will speed up skb_update_prio.
		 */
		if (map && map->priomap_len < max_len) {
			ret = extend_netdev_table(dev, max_len);
			if (ret < 0)
				break;
		}
	}
{
	char *devname = kstrdup(buffer, GFP_KERNEL);
	int ret = -EINVAL;
	u32 prioidx = cgrp_netprio_state(cgrp)->prioidx;
	unsigned long priority;
	char *priostr;
	struct net_device *dev;
	struct netprio_map *map;

	if (!devname)
		return -ENOMEM;

	/*
	 * Minimally sized valid priomap string
	 */
	if (strlen(devname) < 3)
		goto out_free_devname;

	priostr = strstr(devname, " ");
	if (!priostr)
		goto out_free_devname;

	/*
	 *Separate the devname from the associated priority
	 *and advance the priostr pointer to the priority value
	 */
	*priostr = '\0';
	priostr++;

	/*
	 * If the priostr points to NULL, we're at the end of the passed
	 * in string, and its not a valid write
	 */
	if (*priostr == '\0')
		goto out_free_devname;

	ret = kstrtoul(priostr, 10, &priority);
	if (ret < 0)
		goto out_free_devname;

	ret = -ENODEV;

	dev = dev_get_by_name(&init_net, devname);
	if (!dev)
		goto out_free_devname;

	rtnl_lock();
	ret = write_update_netdev_table(dev);
	if (ret < 0)
		goto out_put_dev;

	map = rtnl_dereference(dev->priomap);
	if (map)
		map->priomap[prioidx] = priority;

out_put_dev:
	rtnl_unlock();
	dev_put(dev);

out_free_devname:
	kfree(devname);
	return ret;
}

void net_prio_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct task_struct *p;

	cgroup_taskset_for_each(p, cgrp, tset) {
		unsigned int fd;
		struct fdtable *fdt;
		struct files_struct *files;

		task_lock(p);
		files = p->files;
		if (!files) {
			task_unlock(p);
			continue;
		}

		spin_lock(&files->file_lock);
		fdt = files_fdtable(files);
		for (fd = 0; fd < fdt->max_fds; fd++) {
			struct file *file;
			struct socket *sock;
			int err;

			file = fcheck_files(files, fd);
			if (!file)
				continue;

			sock = sock_from_file(file, &err);
			if (sock)
				sock_update_netprioidx(sock->sk, p);
		}
		spin_unlock(&files->file_lock);
		task_unlock(p);
	}
{
	struct task_struct *p;

	cgroup_taskset_for_each(p, cgrp, tset) {
		unsigned int fd;
		struct fdtable *fdt;
		struct files_struct *files;

		task_lock(p);
		files = p->files;
		if (!files) {
			task_unlock(p);
			continue;
		}

		spin_lock(&files->file_lock);
		fdt = files_fdtable(files);
		for (fd = 0; fd < fdt->max_fds; fd++) {
			struct file *file;
			struct socket *sock;
			int err;

			file = fcheck_files(files, fd);
			if (!file)
				continue;

			sock = sock_from_file(file, &err);
			if (sock)
				sock_update_netprioidx(sock->sk, p);
		}
		spin_unlock(&files->file_lock);
		task_unlock(p);
	}
}
		if (!files) {
			task_unlock(p);
			continue;
		}

		spin_lock(&files->file_lock);
		fdt = files_fdtable(files);
		for (fd = 0; fd < fdt->max_fds; fd++) {
			struct file *file;
			struct socket *sock;
			int err;

			file = fcheck_files(files, fd);
			if (!file)
				continue;

			sock = sock_from_file(file, &err);
			if (sock)
				sock_update_netprioidx(sock->sk, p);
		}
		spin_unlock(&files->file_lock);
		task_unlock(p);
	}
}

static struct cftype ss_files[] = {
	{
		.name = "prioidx",
		.read_u64 = read_prioidx,
	},
	{
		.name = "ifpriomap",
		.read_map = read_priomap,
		.write_string = write_priomap,
	},
	{ }	/* terminate */
};

struct cgroup_subsys net_prio_subsys = {
	.name		= "net_prio",
	.create		= cgrp_create,
	.destroy	= cgrp_destroy,
	.attach		= net_prio_attach,
#ifdef CONFIG_NETPRIO_CGROUP
	.subsys_id	= net_prio_subsys_id,
#endif
	.base_cftypes	= ss_files,
	.module		= THIS_MODULE
};

static int netprio_device_event(struct notifier_block *unused,
				unsigned long event, void *ptr)
{
	struct net_device *dev = ptr;
	struct netprio_map *old;

	/*
	 * Note this is called with rtnl_lock held so we have update side
	 * protection on our rcu assignments
	 */

	switch (event) {
	case NETDEV_UNREGISTER:
		old = rtnl_dereference(dev->priomap);
		RCU_INIT_POINTER(dev->priomap, NULL);
		if (old)
			kfree_rcu(old, rcu);
		break;
	}
	return NOTIFY_DONE;
}

static struct notifier_block netprio_device_notifier = {
	.notifier_call = netprio_device_event
};

static int __init init_cgroup_netprio(void)
{
	int ret;

	ret = cgroup_load_subsys(&net_prio_subsys);
	if (ret)
		goto out;
#ifndef CONFIG_NETPRIO_CGROUP
	smp_wmb();
	net_prio_subsys_id = net_prio_subsys.subsys_id;
#endif

	register_netdevice_notifier(&netprio_device_notifier);

out:
	return ret;
}

static void __exit exit_cgroup_netprio(void)
{
	struct netprio_map *old;
	struct net_device *dev;

	unregister_netdevice_notifier(&netprio_device_notifier);

	cgroup_unload_subsys(&net_prio_subsys);

#ifndef CONFIG_NETPRIO_CGROUP
	net_prio_subsys_id = -1;
	synchronize_rcu();
#endif

	rtnl_lock();
	for_each_netdev(&init_net, dev) {
		old = rtnl_dereference(dev->priomap);
		RCU_INIT_POINTER(dev->priomap, NULL);
		if (old)
			kfree_rcu(old, rcu);
	}
	rtnl_unlock();
}

module_init(init_cgroup_netprio);
module_exit(exit_cgroup_netprio);
MODULE_LICENSE("GPL v2");