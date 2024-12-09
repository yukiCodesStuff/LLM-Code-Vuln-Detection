	struct rc_dev *rdev = data->rc_dev;

	data->rc_dev = NULL;
	rc_unregister_device(rdev);
}
