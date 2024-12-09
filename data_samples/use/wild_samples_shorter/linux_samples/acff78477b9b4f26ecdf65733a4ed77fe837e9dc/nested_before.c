	}
}

/*
 * Merge L0's and L1's MSR bitmap, return false to indicate that
 * we do not use the hardware.
 */
		return false;

	msr_bitmap_l1 = (unsigned long *)kmap(page);
	if (nested_cpu_has_apic_reg_virt(vmcs12)) {
		/*
		 * L0 need not intercept reads for MSRs between 0x800 and 0x8ff, it
		 * just lets the processor take the value from the virtual-APIC page;
		 * take those 256 bits directly from the L1 bitmap.
		 */
		for (msr = 0x800; msr <= 0x8ff; msr += BITS_PER_LONG) {
			unsigned word = msr / BITS_PER_LONG;
			msr_bitmap_l0[word] = msr_bitmap_l1[word];
			msr_bitmap_l0[word + (0x800 / sizeof(long))] = ~0;
		}
	} else {
		for (msr = 0x800; msr <= 0x8ff; msr += BITS_PER_LONG) {
			unsigned word = msr / BITS_PER_LONG;
			msr_bitmap_l0[word] = ~0;
			msr_bitmap_l0[word + (0x800 / sizeof(long))] = ~0;
		}
	}

	nested_vmx_disable_intercept_for_msr(
		msr_bitmap_l1, msr_bitmap_l0,
		X2APIC_MSR(APIC_TASKPRI),
		MSR_TYPE_W);

	if (nested_cpu_has_vid(vmcs12)) {
		nested_vmx_disable_intercept_for_msr(
			msr_bitmap_l1, msr_bitmap_l0,
			X2APIC_MSR(APIC_EOI),
			MSR_TYPE_W);
		nested_vmx_disable_intercept_for_msr(
			msr_bitmap_l1, msr_bitmap_l0,
			X2APIC_MSR(APIC_SELF_IPI),
			MSR_TYPE_W);
	}

	if (spec_ctrl)
		nested_vmx_disable_intercept_for_msr(