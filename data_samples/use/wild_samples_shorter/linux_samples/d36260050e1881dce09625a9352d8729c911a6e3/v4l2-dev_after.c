#endif
	vdev->minor = i + minor_offset;
	vdev->num = nr;

	/* Should not happen since we thought this minor was free */
	if (WARN_ON(video_device[vdev->minor])) {
		mutex_unlock(&videodev_lock);
		printk(KERN_ERR "video_device not empty!\n");
		return -ENFILE;
	}
	devnode_set(vdev);
	vdev->index = get_index(vdev);
	video_device[vdev->minor] = vdev;
	mutex_unlock(&videodev_lock);
