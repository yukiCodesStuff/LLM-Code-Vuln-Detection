		/* let's register it anyway to preserve ordering */
		slave->offset = 0;
		slave->mtd.size = 0;
		printk(KERN_ERR"mtd: partition \"%s\" is out of reach -- disabled\n",
			part->name);
		goto out_register;
	}
	mutex_unlock(&mtd_partitions_mutex);

	free_partition(new);
	pr_info("%s:%i\n", __func__, __LINE__);

	return ret;
}
EXPORT_SYMBOL_GPL(mtd_add_partition);