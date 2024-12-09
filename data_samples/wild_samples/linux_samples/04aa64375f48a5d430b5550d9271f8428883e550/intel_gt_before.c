	for_each_engine(engine, gt, id) {
		struct reg_and_bit rb;

		if (!intel_engine_pm_is_awake(engine))
			continue;

		rb = get_reg_and_bit(engine, regs == gen8_regs, regs, num);
		if (!i915_mmio_reg_offset(rb.reg))
			continue;

		intel_uncore_write_fw(uncore, rb.reg, rb.bit);
		awake |= engine->mask;
	}

	GT_TRACE(gt, "invalidated engines %08x\n", awake);

	/* Wa_2207587034:tgl,dg1,rkl,adl-s,adl-p */
	if (awake &&
	    (IS_TIGERLAKE(i915) ||
	     IS_DG1(i915) ||
	     IS_ROCKETLAKE(i915) ||
	     IS_ALDERLAKE_S(i915) ||
	     IS_ALDERLAKE_P(i915)))
		intel_uncore_write_fw(uncore, GEN12_OA_TLB_INV_CR, 1);

	spin_unlock_irq(&uncore->lock);

	for_each_engine_masked(engine, gt, awake, tmp) {
		struct reg_and_bit rb;

		/*
		 * HW architecture suggest typical invalidation time at 40us,
		 * with pessimistic cases up to 100us and a recommendation to
		 * cap at 1ms. We go a bit higher just in case.
		 */
		const unsigned int timeout_us = 100;
		const unsigned int timeout_ms = 4;

		rb = get_reg_and_bit(engine, regs == gen8_regs, regs, num);
		if (__intel_wait_for_register_fw(uncore,
						 rb.reg, rb.bit, 0,
						 timeout_us, timeout_ms,
						 NULL))
			drm_err_ratelimited(&gt->i915->drm,
					    "%s TLB invalidation did not complete in %ums!\n",
					    engine->name, timeout_ms);
	}

	/*
	 * Use delayed put since a) we mostly expect a flurry of TLB
	 * invalidations so it is good to avoid paying the forcewake cost and
	 * b) it works around a bug in Icelake which cannot cope with too rapid
	 * transitions.
	 */
	intel_uncore_forcewake_put_delayed(uncore, FORCEWAKE_ALL);
}

static bool tlb_seqno_passed(const struct intel_gt *gt, u32 seqno)
{
	u32 cur = intel_gt_tlb_seqno(gt);

	/* Only skip if a *full* TLB invalidate barrier has passed */
	return (s32)(cur - ALIGN(seqno, 2)) > 0;
}

void intel_gt_invalidate_tlb(struct intel_gt *gt, u32 seqno)
{
	intel_wakeref_t wakeref;

	if (I915_SELFTEST_ONLY(gt->awake == -ENODEV))
		return;

	if (intel_gt_is_wedged(gt))
		return;

	if (tlb_seqno_passed(gt, seqno))
		return;

	with_intel_gt_pm_if_awake(gt, wakeref) {
		mutex_lock(&gt->tlb.invalidate_lock);
		if (tlb_seqno_passed(gt, seqno))
			goto unlock;

		mmio_invalidate_full(gt);

		write_seqcount_invalidate(&gt->tlb.seqno);
unlock:
		mutex_unlock(&gt->tlb.invalidate_lock);
	}
}