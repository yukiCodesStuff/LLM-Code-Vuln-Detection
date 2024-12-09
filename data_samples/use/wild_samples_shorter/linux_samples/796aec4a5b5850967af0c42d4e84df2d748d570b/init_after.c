		.align = 32,
		.dev_type = &dsa_device_type,
		.evl_cr_off = offsetof(struct dsa_evl_entry, cr),
		.user_submission_safe = false, /* See INTEL-SA-01084 security advisory */
		.cr_status_off = offsetof(struct dsa_completion_record, status),
		.cr_result_off = offsetof(struct dsa_completion_record, result),
	},
	[IDXD_TYPE_IAX] = {
		.align = 64,
		.dev_type = &iax_device_type,
		.evl_cr_off = offsetof(struct iax_evl_entry, cr),
		.user_submission_safe = false, /* See INTEL-SA-01084 security advisory */
		.cr_status_off = offsetof(struct iax_completion_record, status),
		.cr_result_off = offsetof(struct iax_completion_record, error_code),
		.load_device_defaults = idxd_load_iaa_device_defaults,
	},
	dev_info(&pdev->dev, "Intel(R) Accelerator Device (v%x)\n",
		 idxd->hw.version);

	idxd->user_submission_safe = data->user_submission_safe;

	return 0;

 err_dev_register:
	idxd_cleanup(idxd);