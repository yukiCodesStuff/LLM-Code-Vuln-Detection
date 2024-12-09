{
	char b[BDEVNAME_SIZE];
	struct r1conf *conf = mddev->private;

	/*
	 * If it is not operational, then we have already marked it as dead
	 * else if it is the last working disks, ignore the error, let the
		return;
	}
	set_bit(Blocked, &rdev->flags);
	if (test_and_clear_bit(In_sync, &rdev->flags)) {
		unsigned long flags;
		spin_lock_irqsave(&conf->device_lock, flags);
		mddev->degraded++;
		set_bit(Faulty, &rdev->flags);
		spin_unlock_irqrestore(&conf->device_lock, flags);
	} else
		set_bit(Faulty, &rdev->flags);
	/*
	 * if recovery is running, make sure it aborts.
	 */
	set_bit(MD_RECOVERY_INTR, &mddev->recovery);
	 * Find all failed disks within the RAID1 configuration
	 * and mark them readable.
	 * Called under mddev lock, so rcu protection not needed.
	 */
	for (i = 0; i < conf->raid_disks; i++) {
		struct md_rdev *rdev = conf->mirrors[i].rdev;
		struct md_rdev *repl = conf->mirrors[conf->raid_disks + i].rdev;
		if (repl
			sysfs_notify_dirent_safe(rdev->sysfs_state);
		}
	}
	spin_lock_irqsave(&conf->device_lock, flags);
	mddev->degraded -= count;
	spin_unlock_irqrestore(&conf->device_lock, flags);

	print_conf(conf);