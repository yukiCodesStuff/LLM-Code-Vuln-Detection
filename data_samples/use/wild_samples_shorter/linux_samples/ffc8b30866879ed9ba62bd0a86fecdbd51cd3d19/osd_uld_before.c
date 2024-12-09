	oud->class_dev.class = &osd_uld_class;
	oud->class_dev.parent = dev;
	oud->class_dev.release = __remove;
	error = dev_set_name(&oud->class_dev, disk->disk_name);
	if (error) {
		OSD_ERR("dev_set_name failed => %d\n", error);
		goto err_put_cdev;
	}