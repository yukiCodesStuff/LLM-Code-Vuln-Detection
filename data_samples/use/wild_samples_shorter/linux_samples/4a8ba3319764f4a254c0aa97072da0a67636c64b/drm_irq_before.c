 * vblank events since the system was booted, including lost events due to
 * modesetting activity.
 *
 * Returns:
 * The software vblank counter.
 */
u32 drm_vblank_count(struct drm_device *dev, int crtc)
}
EXPORT_SYMBOL(drm_vblank_count);

/**
 * drm_vblank_count_and_time - retrieve "cooked" vblank counter value
 * and the system timestamp corresponding to that vblank counter value.
 *
 *
 * Updates sequence # and timestamp on event, and sends it to userspace.
 * Caller must hold event lock.
 */
void drm_send_vblank_event(struct drm_device *dev, int crtc,
		struct drm_pending_vblank_event *e)
{
}
EXPORT_SYMBOL(drm_send_vblank_event);

/**
 * drm_vblank_enable - enable the vblank interrupt on a CRTC
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * Drivers should call this routine in their vblank interrupt handlers to
 * update the vblank counter and send any signals that may be pending.
 */
bool drm_handle_vblank(struct drm_device *dev, int crtc)
{
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];
	return true;
}
EXPORT_SYMBOL(drm_handle_vblank);