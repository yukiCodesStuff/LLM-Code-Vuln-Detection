	if (!p) {
		return ret;
	} else {
		if (boot_cpu_data.microcode >= p->patch_id)
			return ret;

		ret = UCODE_NEW;
	}