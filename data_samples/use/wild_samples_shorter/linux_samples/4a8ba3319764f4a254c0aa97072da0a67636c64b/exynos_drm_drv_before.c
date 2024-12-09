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