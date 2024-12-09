{
	u32 flags = hw_flags | MI_MM_SPACE_GTT;
	const int num_rings =
		/* Use an extended w/a on ivb+ if signalling from other rings */
		i915_semaphore_is_enabled(ring->dev) ?
		hweight32(INTEL_INFO(ring->dev)->ring_mask) - 1 :
		0;
	int len, i, ret;

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


	len = 4;
	if (INTEL_INFO(ring->dev)->gen >= 7)
		len += 2 + (num_rings ? 4*num_rings + 2 : 0);

	ret = intel_ring_begin(ring, len);
	if (ret)
		return ret;

	/* WaProgramMiArbOnOffAroundMiSetContext:ivb,vlv,hsw,bdw,chv */
	if (INTEL_INFO(ring->dev)->gen >= 7) {
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_DISABLE);
		if (num_rings) {
			struct intel_engine_cs *signaller;

			intel_ring_emit(ring, MI_LOAD_REGISTER_IMM(num_rings));
			for_each_ring(signaller, to_i915(ring->dev), i) {
				if (signaller == ring)
					continue;

				intel_ring_emit(ring, RING_PSMI_CTL(signaller->mmio_base));
				intel_ring_emit(ring, _MASKED_BIT_ENABLE(GEN6_PSMI_SLEEP_MSG_DISABLE));
			}
		}
	}

	intel_ring_emit(ring, MI_NOOP);
	intel_ring_emit(ring, MI_SET_CONTEXT);
	intel_ring_emit(ring, i915_gem_obj_ggtt_offset(new_context->legacy_hw_ctx.rcs_state) |
			flags);
	/*
	 * w/a: MI_SET_CONTEXT must always be followed by MI_NOOP
	 * WaMiSetContext_Hang:snb,ivb,vlv
	 */
	intel_ring_emit(ring, MI_NOOP);

	if (INTEL_INFO(ring->dev)->gen >= 7) {
		if (num_rings) {
			struct intel_engine_cs *signaller;

			intel_ring_emit(ring, MI_LOAD_REGISTER_IMM(num_rings));
			for_each_ring(signaller, to_i915(ring->dev), i) {
				if (signaller == ring)
					continue;

				intel_ring_emit(ring, RING_PSMI_CTL(signaller->mmio_base));
				intel_ring_emit(ring, _MASKED_BIT_DISABLE(GEN6_PSMI_SLEEP_MSG_DISABLE));
			}
		}
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_ENABLE);
	}

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


	len = 4;
	if (INTEL_INFO(ring->dev)->gen >= 7)
		len += 2 + (num_rings ? 4*num_rings + 2 : 0);

	ret = intel_ring_begin(ring, len);
	if (ret)
		return ret;

	/* WaProgramMiArbOnOffAroundMiSetContext:ivb,vlv,hsw,bdw,chv */
	if (INTEL_INFO(ring->dev)->gen >= 7) {
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_DISABLE);
		if (num_rings) {
			struct intel_engine_cs *signaller;

			intel_ring_emit(ring, MI_LOAD_REGISTER_IMM(num_rings));
			for_each_ring(signaller, to_i915(ring->dev), i) {
				if (signaller == ring)
					continue;

				intel_ring_emit(ring, RING_PSMI_CTL(signaller->mmio_base));
				intel_ring_emit(ring, _MASKED_BIT_ENABLE(GEN6_PSMI_SLEEP_MSG_DISABLE));
			}
		}
	}
			for_each_ring(signaller, to_i915(ring->dev), i) {
				if (signaller == ring)
					continue;

				intel_ring_emit(ring, RING_PSMI_CTL(signaller->mmio_base));
				intel_ring_emit(ring, _MASKED_BIT_ENABLE(GEN6_PSMI_SLEEP_MSG_DISABLE));
			}
		}
	}

	intel_ring_emit(ring, MI_NOOP);
	intel_ring_emit(ring, MI_SET_CONTEXT);
	intel_ring_emit(ring, i915_gem_obj_ggtt_offset(new_context->legacy_hw_ctx.rcs_state) |
			flags);
	/*
	 * w/a: MI_SET_CONTEXT must always be followed by MI_NOOP
	 * WaMiSetContext_Hang:snb,ivb,vlv
	 */
	intel_ring_emit(ring, MI_NOOP);

	if (INTEL_INFO(ring->dev)->gen >= 7) {
		if (num_rings) {
			struct intel_engine_cs *signaller;

			intel_ring_emit(ring, MI_LOAD_REGISTER_IMM(num_rings));
			for_each_ring(signaller, to_i915(ring->dev), i) {
				if (signaller == ring)
					continue;

				intel_ring_emit(ring, RING_PSMI_CTL(signaller->mmio_base));
				intel_ring_emit(ring, _MASKED_BIT_DISABLE(GEN6_PSMI_SLEEP_MSG_DISABLE));
			}
		}
		intel_ring_emit(ring, MI_ARB_ON_OFF | MI_ARB_ENABLE);
	}