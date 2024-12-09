		if (of_machine_is_compatible(strings[i])) {
			is_exynos = true;
			break;
		}
	}

	if (!is_exynos)
		return -ENODEV;

	exynos_drm_pdev = platform_device_register_simple("exynos-drm", -1,
								NULL, 0);
	if (IS_ERR(exynos_drm_pdev))
		return PTR_ERR(exynos_drm_pdev);

	ret = exynos_drm_probe_vidi();
	if (ret < 0)
		goto err_unregister_pd;

	for (i = 0; i < ARRAY_SIZE(exynos_drm_kms_drivers); ++i) {
		ret = platform_driver_register(exynos_drm_kms_drivers[i]);
		if (ret < 0)
			goto err_unregister_kms_drivers;
	}