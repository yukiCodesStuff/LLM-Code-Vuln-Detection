	 */
}

static void i9xx_crtc_enable(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;

	intel_enable_pipe(dev_priv, pipe, false);
	intel_enable_plane(dev_priv, plane, pipe);

	intel_crtc_load_lut(crtc);
	intel_update_fbc(dev);

{
	struct drm_device *dev = crtc->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct intel_framebuffer *intel_fb;
	struct drm_i915_gem_object *obj;
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	struct intel_unpin_work *work;
	unsigned long flags;
	int ret;

	work->event = event;
	work->crtc = crtc;
	intel_fb = to_intel_framebuffer(crtc->fb);
	work->old_fb_obj = intel_fb->obj;
	INIT_WORK(&work->work, intel_unpin_work_fn);

	ret = drm_vblank_get(dev, intel_crtc->pipe);
	if (ret)
	intel_crtc->unpin_work = work;
	spin_unlock_irqrestore(&dev->event_lock, flags);

	intel_fb = to_intel_framebuffer(fb);
	obj = intel_fb->obj;

	if (atomic_read(&intel_crtc->unpin_work_count) >= 2)
		flush_workqueue(dev_priv->wq);

	ret = i915_mutex_lock_interruptible(dev);

cleanup_pending:
	atomic_dec(&intel_crtc->unpin_work_count);
	drm_gem_object_unreference(&work->old_fb_obj->base);
	drm_gem_object_unreference(&obj->base);
	mutex_unlock(&dev->struct_mutex);
