		if (!i915_mmio_reg_offset(rb.reg))
			continue;

		if (GRAPHICS_VER(i915) == 12 && (engine->class == VIDEO_DECODE_CLASS ||
		    engine->class == VIDEO_ENHANCEMENT_CLASS ||
		    engine->class == COMPUTE_CLASS))
			rb.bit = _MASKED_BIT_ENABLE(rb.bit);

		intel_uncore_write_fw(uncore, rb.reg, rb.bit);
		awake |= engine->mask;
	}
