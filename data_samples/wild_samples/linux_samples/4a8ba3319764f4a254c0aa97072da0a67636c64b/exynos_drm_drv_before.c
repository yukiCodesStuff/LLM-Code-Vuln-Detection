		if (of_machine_is_compatible(strings[i])) {
			is_exynos = true;
			break;
		}
	}

	if (!is_exynos)
		return -ENODEV;

	/*
	 * Register device object only in case of Exynos SoC.
	 *
	 * Below codes resolves temporarily infinite loop issue incurred
	 * by Exynos drm driver when using multi-platform kernel.
	 * So these codes will be replaced with more generic way later.
	 */
	if (!of_machine_is_compatible("samsung,exynos3") &&
			!of_machine_is_compatible("samsung,exynos4") &&
			!of_machine_is_compatible("samsung,exynos5"))
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