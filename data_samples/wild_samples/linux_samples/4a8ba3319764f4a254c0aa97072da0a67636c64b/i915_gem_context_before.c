{
	u32 flags = hw_flags | MI_MM_SPACE_GTT;
	int ret;

	/* w/a: If Flush TLB Invalidation Mode is enabled, driver must do a TLB
	 * invalidation prior to MI_SET_CONTEXT. On GEN6 we don't set the value
	 * explicitly, so we rely on the value at ring init, stored in
	 * itlb_before_ctx_switch.
	 */
	if (IS_GEN6(ring->dev)) {
		ret = ring->flush(ring, I915_GEM_GPU_DOMAINS, 0);
		if (ret)
			return ret;
	}

	/* These flags are for resource streamer on HSW+ */
	if (!IS_HASWELL(ring->dev) && INTEL_INFO(ring->dev)->gen < 8)
		flags |= (MI_SAVE_EXT_STATE_EN | MI_RESTORE_EXT_STATE_EN);

	ret = intel_ring_begin(ring, 6);
	if (ret)
		return ret;

	/* WaProgramMiArbOnOffAroundMiSetContext:ivb,vlv,hsw,bdw,chv */
	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_DISABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_emit(ring, MI_NOOP);
	intel_ring_emit(ring, MI_SET_CONTEXT);
	intel_ring_emit(ring, i915_gem_obj_ggtt_offset(new_context->legacy_hw_ctx.rcs_state) |
			flags);
	/*
	 * w/a: MI_SET_CONTEXT must always be followed by MI_NOOP
	 * WaMiSetContext_Hang:snb,ivb,vlv
	 */
	intel_ring_emit(ring, MI_NOOP);

	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_ENABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_advance(ring);

	return ret;
}
	if (IS_GEN6(ring->dev)) {
		ret = ring->flush(ring, I915_GEM_GPU_DOMAINS, 0);
		if (ret)
			return ret;
	}

	/* These flags are for resource streamer on HSW+ */
	if (!IS_HASWELL(ring->dev) && INTEL_INFO(ring->dev)->gen < 8)
		flags |= (MI_SAVE_EXT_STATE_EN | MI_RESTORE_EXT_STATE_EN);

	ret = intel_ring_begin(ring, 6);
	if (ret)
		return ret;

	/* WaProgramMiArbOnOffAroundMiSetContext:ivb,vlv,hsw,bdw,chv */
	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_DISABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_emit(ring, MI_NOOP);
	intel_ring_emit(ring, MI_SET_CONTEXT);
	intel_ring_emit(ring, i915_gem_obj_ggtt_offset(new_context->legacy_hw_ctx.rcs_state) |
			flags);
	/*
	 * w/a: MI_SET_CONTEXT must always be followed by MI_NOOP
	 * WaMiSetContext_Hang:snb,ivb,vlv
	 */
	intel_ring_emit(ring, MI_NOOP);

	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_ENABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_advance(ring);

	return ret;
}

static int do_switch(struct intel_engine_cs *ring,
		     struct intel_context *to)
{
	struct drm_i915_private *dev_priv = ring->dev->dev_private;
	struct intel_context *from = ring->last_context;
	u32 hw_flags = 0;
	bool uninitialized = false;
	struct i915_vma *vma;
	int ret, i;

	if (from != NULL && ring == &dev_priv->ring[RCS]) {
		BUG_ON(from->legacy_hw_ctx.rcs_state == NULL);
		BUG_ON(!i915_gem_obj_is_pinned(from->legacy_hw_ctx.rcs_state));
	}
	if (IS_GEN6(ring->dev)) {
		ret = ring->flush(ring, I915_GEM_GPU_DOMAINS, 0);
		if (ret)
			return ret;
	}

	/* These flags are for resource streamer on HSW+ */
	if (!IS_HASWELL(ring->dev) && INTEL_INFO(ring->dev)->gen < 8)
		flags |= (MI_SAVE_EXT_STATE_EN | MI_RESTORE_EXT_STATE_EN);

	ret = intel_ring_begin(ring, 6);
	if (ret)
		return ret;

	/* WaProgramMiArbOnOffAroundMiSetContext:ivb,vlv,hsw,bdw,chv */
	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_DISABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_emit(ring, MI_NOOP);
	intel_ring_emit(ring, MI_SET_CONTEXT);
	intel_ring_emit(ring, i915_gem_obj_ggtt_offset(new_context->legacy_hw_ctx.rcs_state) |
			flags);
	/*
	 * w/a: MI_SET_CONTEXT must always be followed by MI_NOOP
	 * WaMiSetContext_Hang:snb,ivb,vlv
	 */
	intel_ring_emit(ring, MI_NOOP);

	if (INTEL_INFO(ring->dev)->gen >= 7)
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_ENABLE);
	else
		intel_ring_emit(ring, MI_NOOP);

	intel_ring_advance(ring);

	return ret;
}

static int do_switch(struct intel_engine_cs *ring,
		     struct intel_context *to)
{
	struct drm_i915_private *dev_priv = ring->dev->dev_private;
	struct intel_context *from = ring->last_context;
	u32 hw_flags = 0;
	bool uninitialized = false;
	struct i915_vma *vma;
	int ret, i;

	if (from != NULL && ring == &dev_priv->ring[RCS]) {
		BUG_ON(from->legacy_hw_ctx.rcs_state == NULL);
		BUG_ON(!i915_gem_obj_is_pinned(from->legacy_hw_ctx.rcs_state));
	}