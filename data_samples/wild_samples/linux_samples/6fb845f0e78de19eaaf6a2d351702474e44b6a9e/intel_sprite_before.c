{
	struct drm_i915_private *dev_priv = to_i915(plane->base.dev);
	enum plane_id plane_id = plane->id;
	enum pipe pipe = plane->pipe;
	const struct drm_intel_sprite_colorkey *key = &plane_state->ckey;
	u32 surf_addr = plane_state->color_plane[color_plane].offset;
	u32 stride = skl_plane_stride(plane_state, color_plane);
	u32 aux_stride = skl_plane_stride(plane_state, 1);
	int crtc_x = plane_state->base.dst.x1;
	int crtc_y = plane_state->base.dst.y1;
	uint32_t x = plane_state->color_plane[color_plane].x;
	uint32_t y = plane_state->color_plane[color_plane].y;
	uint32_t src_w = drm_rect_width(&plane_state->base.src) >> 16;
	uint32_t src_h = drm_rect_height(&plane_state->base.src) >> 16;
	struct intel_plane *linked = plane_state->linked_plane;
	const struct drm_framebuffer *fb = plane_state->base.fb;
	u8 alpha = plane_state->base.alpha >> 8;
	unsigned long irqflags;
	u32 keymsk, keymax;

	/* Sizes are 0 based */
	src_w--;
	src_h--;

	keymax = (key->max_value & 0xffffff) | PLANE_KEYMAX_ALPHA(alpha);

	keymsk = key->channel_mask & 0x3ffffff;
	if (alpha < 0xff)
		keymsk |= PLANE_KEYMSK_ALPHA_ENABLE;

	/* The scaler will handle the output position */
	if (plane_state->scaler_id >= 0) {
		crtc_x = 0;
		crtc_y = 0;
	}