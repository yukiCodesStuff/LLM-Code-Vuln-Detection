	if (!is_exynos)
		return -ENODEV;

	exynos_drm_pdev = platform_device_register_simple("exynos-drm", -1,
								NULL, 0);
	if (IS_ERR(exynos_drm_pdev))
		return PTR_ERR(exynos_drm_pdev);