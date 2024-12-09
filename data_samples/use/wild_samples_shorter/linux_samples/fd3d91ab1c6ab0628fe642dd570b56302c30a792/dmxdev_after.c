	if (mutex_lock_interruptible(&dmxdev->mutex))
		return -ERESTARTSYS;

	if (dmxdev->exit) {
		mutex_unlock(&dmxdev->mutex);
		return -ENODEV;
	}

	for (i = 0; i < dmxdev->filternum; i++)
		if (dmxdev->filter[i].state == DMXDEV_STATE_FREE)
			break;


void dvb_dmxdev_release(struct dmxdev *dmxdev)
{
	mutex_lock(&dmxdev->mutex);
	dmxdev->exit = 1;
	mutex_unlock(&dmxdev->mutex);

	if (dmxdev->dvbdev->users > 1) {
		wait_event(dmxdev->dvbdev->wait_queue,
				dmxdev->dvbdev->users == 1);
	}