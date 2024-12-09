	struct kfd_dev *dev;
	struct kfd_process_device *pdd;

	mutex_lock(&process->mutex);

	/*Iterating over all devices*/
	while ((dev = kfd_topology_enum_kfd_devices(id)) != NULL &&
		id < NUM_OF_SUPPORTED_GPUS) {

		pdd = kfd_get_process_device_data(dev, process, 1);

		/*
		 * For 64 bit process aperture will be statically reserved in
		 * the x86_64 non canonical process address space
		id++;
	}

	mutex_unlock(&process->mutex);

	return 0;
}

