	u32 max_len;
	struct netprio_map *map;

	rtnl_lock();
	max_len = atomic_read(&max_prioidx) + 1;
	map = rtnl_dereference(dev->priomap);
	if (!map || map->priomap_len < max_len)
		ret = extend_netdev_table(dev, max_len);
	rtnl_unlock();

	return ret;
}

	if (!dev)
		goto out_free_devname;

	ret = write_update_netdev_table(dev);
	if (ret < 0)
		goto out_put_dev;

	rcu_read_lock();
	map = rcu_dereference(dev->priomap);
	if (map)
		map->priomap[prioidx] = priority;
	rcu_read_unlock();

out_put_dev:
	dev_put(dev);

out_free_devname:
	kfree(devname);
void net_prio_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct task_struct *p;
	char *tmp = kzalloc(sizeof(char) * PATH_MAX, GFP_KERNEL);

	if (!tmp) {
		pr_warn("Unable to attach cgrp due to alloc failure!\n");
		return;
	}

	cgroup_taskset_for_each(p, cgrp, tset) {
		unsigned int fd;
		struct fdtable *fdt;
			continue;
		}

		rcu_read_lock();
		fdt = files_fdtable(files);
		for (fd = 0; fd < fdt->max_fds; fd++) {
			char *path;
			struct file *file;
			struct socket *sock;
			unsigned long s;
			int rv, err = 0;

			file = fcheck_files(files, fd);
			if (!file)
				continue;

			path = d_path(&file->f_path, tmp, PAGE_SIZE);
			rv = sscanf(path, "socket:[%lu]", &s);
			if (rv <= 0)
				continue;

			sock = sock_from_file(file, &err);
			if (!err)
				sock_update_netprioidx(sock->sk, p);
		}
		rcu_read_unlock();
		task_unlock(p);
	}
	kfree(tmp);
}

static struct cftype ss_files[] = {
	{