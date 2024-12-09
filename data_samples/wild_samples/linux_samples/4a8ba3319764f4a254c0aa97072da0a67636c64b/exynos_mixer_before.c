{
	struct mixer_context *mixer_ctx = mgr_to_mixer(mgr);

	mutex_lock(&mixer_ctx->mixer_mutex);
	if (!mixer_ctx->powered) {
		mutex_unlock(&mixer_ctx->mixer_mutex);
		return;
	}
	mutex_unlock(&mixer_ctx->mixer_mutex);

	drm_vblank_get(mgr->crtc->dev, mixer_ctx->pipe);

	atomic_set(&mixer_ctx->wait_vsync_event, 1);

	/*
	 * wait for MIXER to signal VSYNC interrupt or return after
	 * timeout which is set to 50ms (refresh rate of 20).
	 */
	if (!wait_event_timeout(mixer_ctx->wait_vsync_queue,
				!atomic_read(&mixer_ctx->wait_vsync_event),
				HZ/20))
		DRM_DEBUG_KMS("vblank wait timed out.\n");

	drm_vblank_put(mgr->crtc->dev, mixer_ctx->pipe);
}
	if (!mixer_ctx->powered) {
		mutex_unlock(&mixer_ctx->mixer_mutex);
		return;
	}
	mutex_unlock(&mixer_ctx->mixer_mutex);

	drm_vblank_get(mgr->crtc->dev, mixer_ctx->pipe);

	atomic_set(&mixer_ctx->wait_vsync_event, 1);

	/*
	 * wait for MIXER to signal VSYNC interrupt or return after
	 * timeout which is set to 50ms (refresh rate of 20).
	 */
	if (!wait_event_timeout(mixer_ctx->wait_vsync_queue,
				!atomic_read(&mixer_ctx->wait_vsync_event),
				HZ/20))
		DRM_DEBUG_KMS("vblank wait timed out.\n");

	drm_vblank_put(mgr->crtc->dev, mixer_ctx->pipe);
}

static void mixer_window_suspend(struct exynos_drm_manager *mgr)
{
	struct mixer_context *ctx = mgr_to_mixer(mgr);
	struct hdmi_win_data *win_data;
	int i;

	for (i = 0; i < MIXER_WIN_NR; i++) {
		win_data = &ctx->win_data[i];
		win_data->resume = win_data->enabled;
		mixer_win_disable(mgr, i);
	}
	if (ret) {
		mixer_mgr_remove(&ctx->manager);
		return ret;
	}

	pm_runtime_enable(dev);

	return 0;
}

static void mixer_unbind(struct device *dev, struct device *master, void *data)
{
	struct mixer_context *ctx = dev_get_drvdata(dev);

	mixer_mgr_remove(&ctx->manager);

	pm_runtime_disable(dev);
}

static const struct component_ops mixer_component_ops = {
	.bind	= mixer_bind,
	.unbind	= mixer_unbind,
};

static int mixer_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mixer_drv_data *drv;
	struct mixer_context *ctx;
	int ret;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		DRM_ERROR("failed to alloc mixer context.\n");
		return -ENOMEM;
	}
{
	struct mixer_context *ctx = dev_get_drvdata(dev);

	mixer_mgr_remove(&ctx->manager);

	pm_runtime_disable(dev);
}

static const struct component_ops mixer_component_ops = {
	.bind	= mixer_bind,
	.unbind	= mixer_unbind,
};

static int mixer_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct mixer_drv_data *drv;
	struct mixer_context *ctx;
	int ret;

	ctx = devm_kzalloc(&pdev->dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx) {
		DRM_ERROR("failed to alloc mixer context.\n");
		return -ENOMEM;
	}

	mutex_init(&ctx->mixer_mutex);

	ctx->manager.type = EXYNOS_DISPLAY_TYPE_HDMI;
	ctx->manager.ops = &mixer_manager_ops;

	if (dev->of_node) {
		const struct of_device_id *match;

		match = of_match_node(mixer_match_types, dev->of_node);
		drv = (struct mixer_drv_data *)match->data;
	} else {
		drv = (struct mixer_drv_data *)
			platform_get_device_id(pdev)->driver_data;
	}

	ctx->pdev = pdev;
	ctx->dev = dev;
	ctx->vp_enabled = drv->is_vp_enabled;
	ctx->has_sclk = drv->has_sclk;
	ctx->mxr_ver = drv->version;
	init_waitqueue_head(&ctx->wait_vsync_queue);
	atomic_set(&ctx->wait_vsync_event, 0);

	platform_set_drvdata(pdev, ctx);

	ret = exynos_drm_component_add(&pdev->dev, EXYNOS_DEVICE_TYPE_CRTC,
					ctx->manager.type);
	if (ret)
		return ret;

	ret = component_add(&pdev->dev, &mixer_component_ops);
	if (ret) {
		exynos_drm_component_del(&pdev->dev, EXYNOS_DEVICE_TYPE_CRTC);
		return ret;
	}

	pm_runtime_enable(dev);

	return ret;
}

static int mixer_remove(struct platform_device *pdev)
{
	pm_runtime_disable(&pdev->dev);

	component_del(&pdev->dev, &mixer_component_ops);
	exynos_drm_component_del(&pdev->dev, EXYNOS_DEVICE_TYPE_CRTC);

	return 0;
}

struct platform_driver mixer_driver = {
	.driver = {
		.name = "exynos-mixer",
		.owner = THIS_MODULE,
		.of_match_table = mixer_match_types,
	},
	.probe = mixer_probe,
	.remove = mixer_remove,
	.id_table	= mixer_driver_types,
};