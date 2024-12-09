static void mixer_wait_for_vblank(struct exynos_drm_manager *mgr)
{
	struct mixer_context *mixer_ctx = mgr_to_mixer(mgr);

	mutex_lock(&mixer_ctx->mixer_mutex);
	if (!mixer_ctx->powered) {
		mutex_unlock(&mixer_ctx->mixer_mutex);
	}
	mutex_unlock(&mixer_ctx->mixer_mutex);

	drm_vblank_get(mgr->crtc->dev, mixer_ctx->pipe);

	atomic_set(&mixer_ctx->wait_vsync_event, 1);

	/*
		return ret;
	}

	pm_runtime_enable(dev);

	return 0;
}

static void mixer_unbind(struct device *dev, struct device *master, void *data)
	struct mixer_context *ctx = dev_get_drvdata(dev);

	mixer_mgr_remove(&ctx->manager);

	pm_runtime_disable(dev);
}

static const struct component_ops mixer_component_ops = {
	.bind	= mixer_bind,