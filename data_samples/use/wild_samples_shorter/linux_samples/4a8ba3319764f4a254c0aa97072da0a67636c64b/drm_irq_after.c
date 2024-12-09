 * vblank events since the system was booted, including lost events due to
 * modesetting activity.
 *
 * This is the legacy version of drm_crtc_vblank_count().
 *
 * Returns:
 * The software vblank counter.
 */
u32 drm_vblank_count(struct drm_device *dev, int crtc)
}
EXPORT_SYMBOL(drm_vblank_count);

/**
 * drm_crtc_vblank_count - retrieve "cooked" vblank counter value
 * @crtc: which counter to retrieve
 *
 * Fetches the "cooked" vblank count value that represents the number of
 * vblank events since the system was booted, including lost events due to
 * modesetting activity.
 *
 * This is the native KMS version of drm_vblank_count().
 *
 * Returns:
 * The software vblank counter.
 */
u32 drm_crtc_vblank_count(struct drm_crtc *crtc)
{
	return drm_vblank_count(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_vblank_count);

/**
 * drm_vblank_count_and_time - retrieve "cooked" vblank counter value
 * and the system timestamp corresponding to that vblank counter value.
 *
 *
 * Updates sequence # and timestamp on event, and sends it to userspace.
 * Caller must hold event lock.
 *
 * This is the legacy version of drm_crtc_send_vblank_event().
 */
void drm_send_vblank_event(struct drm_device *dev, int crtc,
		struct drm_pending_vblank_event *e)
{
}
EXPORT_SYMBOL(drm_send_vblank_event);

/**
 * drm_crtc_send_vblank_event - helper to send vblank event after pageflip
 * @crtc: the source CRTC of the vblank event
 * @e: the event to send
 *
 * Updates sequence # and timestamp on event, and sends it to userspace.
 * Caller must hold event lock.
 *
 * This is the native KMS version of drm_send_vblank_event().
 */
void drm_crtc_send_vblank_event(struct drm_crtc *crtc,
				struct drm_pending_vblank_event *e)
{
	drm_send_vblank_event(crtc->dev, drm_crtc_index(crtc), e);
}
EXPORT_SYMBOL(drm_crtc_send_vblank_event);

/**
 * drm_vblank_enable - enable the vblank interrupt on a CRTC
 * @dev: DRM device
 * @crtc: CRTC in question
 *
 * Drivers should call this routine in their vblank interrupt handlers to
 * update the vblank counter and send any signals that may be pending.
 *
 * This is the legacy version of drm_crtc_handle_vblank().
 */
bool drm_handle_vblank(struct drm_device *dev, int crtc)
{
	struct drm_vblank_crtc *vblank = &dev->vblank[crtc];
	return true;
}
EXPORT_SYMBOL(drm_handle_vblank);

/**
 * drm_crtc_handle_vblank - handle a vblank event
 * @crtc: where this event occurred
 *
 * Drivers should call this routine in their vblank interrupt handlers to
 * update the vblank counter and send any signals that may be pending.
 *
 * This is the native KMS version of drm_handle_vblank().
 *
 * Returns:
 * True if the event was successfully handled, false on failure.
 */
bool drm_crtc_handle_vblank(struct drm_crtc *crtc)
{
	return drm_handle_vblank(crtc->dev, drm_crtc_index(crtc));
}
EXPORT_SYMBOL(drm_crtc_handle_vblank);