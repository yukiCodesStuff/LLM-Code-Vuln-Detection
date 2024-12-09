	u32 max_len;
	struct netprio_map *map;

	max_len = atomic_read(&max_prioidx) + 1;
	map = rtnl_dereference(dev->priomap);
	if (!map || map->priomap_len < max_len)
		ret = extend_netdev_table(dev, max_len);

	return ret;
}

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
void net_prio_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct task_struct *p;

	cgroup_taskset_for_each(p, cgrp, tset) {
		unsigned int fd;
		struct fdtable *fdt;
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