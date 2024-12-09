{
	uint8_t id  = 0;
	struct kfd_dev *dev;
	struct kfd_process_device *pdd;

	/*Iterating over all devices*/
	while ((dev = kfd_topology_enum_kfd_devices(id)) != NULL &&
		id < NUM_OF_SUPPORTED_GPUS) {

		pdd = kfd_get_process_device_data(dev, process, 1);
		if (!pdd)
			return -1;

		/*
		 * For 64 bit process aperture will be statically reserved in
		 * the x86_64 non canonical process address space
		 * amdkfd doesn't currently support apertures for 32 bit process
		 */
		if (process->is_32bit_user_mode) {
			pdd->lds_base = pdd->lds_limit = 0;
			pdd->gpuvm_base = pdd->gpuvm_limit = 0;
			pdd->scratch_base = pdd->scratch_limit = 0;
		} else {
			/*
			 * node id couldn't be 0 - the three MSB bits of
			 * aperture shoudn't be 0
			 */
			pdd->lds_base = MAKE_LDS_APP_BASE(id + 1);

			pdd->lds_limit = MAKE_LDS_APP_LIMIT(pdd->lds_base);

			pdd->gpuvm_base = MAKE_GPUVM_APP_BASE(id + 1);

			pdd->gpuvm_limit =
					MAKE_GPUVM_APP_LIMIT(pdd->gpuvm_base);

			pdd->scratch_base = MAKE_SCRATCH_APP_BASE(id + 1);

			pdd->scratch_limit =
				MAKE_SCRATCH_APP_LIMIT(pdd->scratch_base);
		}

		dev_dbg(kfd_device, "node id %u\n", id);
		dev_dbg(kfd_device, "gpu id %u\n", pdd->dev->id);
		dev_dbg(kfd_device, "lds_base %llX\n", pdd->lds_base);
		dev_dbg(kfd_device, "lds_limit %llX\n", pdd->lds_limit);
		dev_dbg(kfd_device, "gpuvm_base %llX\n", pdd->gpuvm_base);
		dev_dbg(kfd_device, "gpuvm_limit %llX\n", pdd->gpuvm_limit);
		dev_dbg(kfd_device, "scratch_base %llX\n", pdd->scratch_base);
		dev_dbg(kfd_device, "scratch_limit %llX\n", pdd->scratch_limit);

		id++;
	}

	return 0;
}
		} else {
			/*
			 * node id couldn't be 0 - the three MSB bits of
			 * aperture shoudn't be 0
			 */
			pdd->lds_base = MAKE_LDS_APP_BASE(id + 1);

			pdd->lds_limit = MAKE_LDS_APP_LIMIT(pdd->lds_base);

			pdd->gpuvm_base = MAKE_GPUVM_APP_BASE(id + 1);

			pdd->gpuvm_limit =
					MAKE_GPUVM_APP_LIMIT(pdd->gpuvm_base);

			pdd->scratch_base = MAKE_SCRATCH_APP_BASE(id + 1);

			pdd->scratch_limit =
				MAKE_SCRATCH_APP_LIMIT(pdd->scratch_base);
		}

		dev_dbg(kfd_device, "node id %u\n", id);
		dev_dbg(kfd_device, "gpu id %u\n", pdd->dev->id);
		dev_dbg(kfd_device, "lds_base %llX\n", pdd->lds_base);
		dev_dbg(kfd_device, "lds_limit %llX\n", pdd->lds_limit);
		dev_dbg(kfd_device, "gpuvm_base %llX\n", pdd->gpuvm_base);
		dev_dbg(kfd_device, "gpuvm_limit %llX\n", pdd->gpuvm_limit);
		dev_dbg(kfd_device, "scratch_base %llX\n", pdd->scratch_base);
		dev_dbg(kfd_device, "scratch_limit %llX\n", pdd->scratch_limit);

		id++;
	}

	return 0;
}

