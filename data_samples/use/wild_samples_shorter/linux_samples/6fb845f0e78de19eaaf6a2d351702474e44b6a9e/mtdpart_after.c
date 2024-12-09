		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;

		/* Initialize ->erasesize to make add_mtd_device() happy. */
		slave->mtd.erasesize = parent->erasesize;

		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	mutex_unlock(&mtd_partitions_mutex);

	free_partition(new);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);