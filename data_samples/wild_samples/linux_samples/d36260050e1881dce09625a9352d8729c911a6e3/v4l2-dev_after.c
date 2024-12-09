	if (i == VIDEO_NUM_DEVICES) {
		mutex_unlock(&videodev_lock);
		printk(KERN_ERR "could not get a free minor\n");
		return -ENFILE;
	}
#endif
	vdev->minor = i + minor_offset;
	vdev->num = nr;

	/* Should not happen since we thought this minor was free */
	if (WARN_ON(video_device[vdev->minor])) {
		mutex_unlock(&videodev_lock);
		printk(KERN_ERR "video_device not empty!\n");
		return -ENFILE;
	}