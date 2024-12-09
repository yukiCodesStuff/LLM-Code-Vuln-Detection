		if (!i915_mmio_reg_offset(rb.reg))
			continue;

		intel_uncore_write_fw(uncore, rb.reg, rb.bit);
		awake |= engine->mask;
	}
