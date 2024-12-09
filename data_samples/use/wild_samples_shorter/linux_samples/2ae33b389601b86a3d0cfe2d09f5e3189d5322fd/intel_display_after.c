	 */
}

/**
 * i9xx_fixup_plane - ugly workaround for G45 to fire up the hardware
 * cursor plane briefly if not already running after enabling the display
 * plane.
 * This workaround avoids occasional blank screens when self refresh is
 * enabled.
 */
static void
g4x_fixup_plane(struct drm_i915_private *dev_priv, enum pipe pipe)
{
	u32 cntl = I915_READ(CURCNTR(pipe));

	if ((cntl & CURSOR_MODE) == 0) {
		u32 fw_bcl_self = I915_READ(FW_BLC_SELF);

		I915_WRITE(FW_BLC_SELF, fw_bcl_self & ~FW_BLC_SELF_EN);
		I915_WRITE(CURCNTR(pipe), CURSOR_MODE_64_ARGB_AX);
		intel_wait_for_vblank(dev_priv->dev, pipe);
		I915_WRITE(CURCNTR(pipe), cntl);
		I915_WRITE(CURBASE(pipe), I915_READ(CURBASE(pipe)));
		I915_WRITE(FW_BLC_SELF, fw_bcl_self);
	}
}

static void i9xx_crtc_enable(struct drm_crtc *crtc)
{
	struct drm_device *dev = crtc->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;

	intel_enable_pipe(dev_priv, pipe, false);
	intel_enable_plane(dev_priv, plane, pipe);
	if (IS_G4X(dev))
		g4x_fixup_plane(dev_priv, pipe);

	intel_crtc_load_lut(crtc);
	intel_update_fbc(dev);

{
	struct drm_device *dev = crtc->dev;
	struct drm_i915_private *dev_priv = dev->dev_private;
	struct drm_framebuffer *old_fb = crtc->fb;
	struct drm_i915_gem_object *obj = to_intel_framebuffer(fb)->obj;
	struct intel_crtc *intel_crtc = to_intel_crtc(crtc);
	struct intel_unpin_work *work;
	unsigned long flags;
	int ret;

	work->event = event;
	work->crtc = crtc;
	work->old_fb_obj = to_intel_framebuffer(old_fb)->obj;
	INIT_WORK(&work->work, intel_unpin_work_fn);

	ret = drm_vblank_get(dev, intel_crtc->pipe);
	if (ret)
	intel_crtc->unpin_work = work;
	spin_unlock_irqrestore(&dev->event_lock, flags);

	if (atomic_read(&intel_crtc->unpin_work_count) >= 2)
		flush_workqueue(dev_priv->wq);

	ret = i915_mutex_lock_interruptible(dev);

cleanup_pending:
	atomic_dec(&intel_crtc->unpin_work_count);
	crtc->fb = old_fb;
	drm_gem_object_unreference(&work->old_fb_obj->base);
	drm_gem_object_unreference(&obj->base);
	mutex_unlock(&dev->struct_mutex);
