	unsigned f_intel_pt = kvm_x86_ops->pt_supported() ? F(INTEL_PT) : 0;
	unsigned f_la57 = 0;

	/* cpuid 1.edx */
	const u32 kvm_cpuid_1_edx_x86_features =
		F(FPU) | F(VME) | F(DE) | F(PSE) |
		F(TSC) | F(MSR) | F(PAE) | F(MCE) |
		F(CX8) | F(APIC) | 0 /* Reserved */ | F(SEP) |
		F(MTRR) | F(PGE) | F(MCA) | F(CMOV) |
		F(PAT) | F(PSE36) | 0 /* PSN */ | F(CLFLUSH) |
		0 /* Reserved, DS, ACPI */ | F(MMX) |
		F(FXSR) | F(XMM) | F(XMM2) | F(SELFSNOOP) |
		0 /* HTT, TM, Reserved, PBE */;
	/* cpuid 0x80000001.edx */
	const u32 kvm_cpuid_8000_0001_edx_x86_features =
		F(FPU) | F(VME) | F(DE) | F(PSE) |
		F(TSC) | F(MSR) | F(PAE) | F(MCE) |
		F(CX8) | F(APIC) | 0 /* Reserved */ | F(SYSCALL) |
		F(MTRR) | F(PGE) | F(MCA) | F(CMOV) |
		F(PAT) | F(PSE36) | 0 /* Reserved */ |
		f_nx | 0 /* Reserved */ | F(MMXEXT) | F(MMX) |
		F(FXSR) | F(FXSR_OPT) | f_gbpages | f_rdtscp |
		0 /* Reserved */ | f_lm | F(3DNOWEXT) | F(3DNOW);
	/* cpuid 1.ecx */
	const u32 kvm_cpuid_1_ecx_x86_features =
		/* NOTE: MONITOR (and MWAIT) are emulated as NOP,
		 * but *not* advertised to guests via CPUID ! */
		F(XMM3) | F(PCLMULQDQ) | 0 /* DTES64, MONITOR */ |
		0 /* DS-CPL, VMX, SMX, EST */ |
		0 /* TM2 */ | F(SSSE3) | 0 /* CNXT-ID */ | 0 /* Reserved */ |
		F(FMA) | F(CX16) | 0 /* xTPR Update, PDCM */ |
		F(PCID) | 0 /* Reserved, DCA */ | F(XMM4_1) |
		F(XMM4_2) | F(X2APIC) | F(MOVBE) | F(POPCNT) |
		0 /* Reserved*/ | F(AES) | F(XSAVE) | 0 /* OSXSAVE */ | F(AVX) |
		F(F16C) | F(RDRAND);
	/* cpuid 0x80000001.ecx */
	const u32 kvm_cpuid_8000_0001_ecx_x86_features =
		F(LAHF_LM) | F(CMP_LEGACY) | 0 /*SVM*/ | 0 /* ExtApicSpace */ |
		F(CR8_LEGACY) | F(ABM) | F(SSE4A) | F(MISALIGNSSE) |
		F(3DNOWPREFETCH) | F(OSVW) | 0 /* IBS */ | F(XOP) |
		0 /* SKINIT, WDT, LWP */ | F(FMA4) | F(TBM) |
		F(TOPOEXT) | F(PERFCTR_CORE);

	/* cpuid 0x80000008.ebx */
	const u32 kvm_cpuid_8000_0008_ebx_x86_features =
		F(WBNOINVD) | F(AMD_IBPB) | F(AMD_IBRS) | F(AMD_SSBD) | F(VIRT_SSBD) |
		F(AMD_SSB_NO) | F(AMD_STIBP);

	/* cpuid 0xC0000001.edx */
	const u32 kvm_cpuid_C000_0001_edx_x86_features =
		F(XSTORE) | F(XSTORE_EN) | F(XCRYPT) | F(XCRYPT_EN) |
		F(ACE2) | F(ACE2_EN) | F(PHE) | F(PHE_EN) |
		F(PMM) | F(PMM_EN);

	/* cpuid 7.0.ebx */
	const u32 kvm_cpuid_7_0_ebx_x86_features =
		F(FSGSBASE) | F(BMI1) | F(HLE) | F(AVX2) | F(SMEP) |
		F(BMI2) | F(ERMS) | f_invpcid | F(RTM) | f_mpx | F(RDSEED) |
		F(ADX) | F(SMAP) | F(AVX512IFMA) | F(AVX512F) | F(AVX512PF) |
		F(AVX512ER) | F(AVX512CD) | F(CLFLUSHOPT) | F(CLWB) | F(AVX512DQ) |
		F(SHA_NI) | F(AVX512BW) | F(AVX512VL) | f_intel_pt;

	/* cpuid 0xD.1.eax */
	const u32 kvm_cpuid_D_1_eax_x86_features =
		F(XSAVEOPT) | F(XSAVEC) | F(XGETBV1) | f_xsaves;

	/* cpuid 7.0.ecx*/
	const u32 kvm_cpuid_7_0_ecx_x86_features =
		F(AVX512VBMI) | F(LA57) | F(PKU) | 0 /*OSPKE*/ |
		F(AVX512_VPOPCNTDQ) | F(UMIP) | F(AVX512_VBMI2) | F(GFNI) |
		F(VAES) | F(VPCLMULQDQ) | F(AVX512_VNNI) | F(AVX512_BITALG) |
		F(CLDEMOTE) | F(MOVDIRI) | F(MOVDIR64B);

	/* cpuid 7.0.edx*/
	const u32 kvm_cpuid_7_0_edx_x86_features =
		F(AVX512_4VNNIW) | F(AVX512_4FMAPS) | F(SPEC_CTRL) |
		F(SPEC_CTRL_SSBD) | F(ARCH_CAPABILITIES) | F(INTEL_STIBP) |
		F(MD_CLEAR);

	/* all calls to cpuid_count() should be made on the same cpu */
	get_cpu();

	r = -E2BIG;

	if (*nent >= maxnent)
		goto out;

	do_cpuid_1_ent(entry, function, index);
	++*nent;

	switch (function) {
	case 0:
		entry->eax = min(entry->eax, (u32)(f_intel_pt ? 0x14 : 0xd));
		break;
	case 1:
		entry->edx &= kvm_cpuid_1_edx_x86_features;
		cpuid_mask(&entry->edx, CPUID_1_EDX);
		entry->ecx &= kvm_cpuid_1_ecx_x86_features;
		cpuid_mask(&entry->ecx, CPUID_1_ECX);
		/* we support x2apic emulation even if host does not support
		 * it since we emulate x2apic in software */
		entry->ecx |= F(X2APIC);
		break;
	/* function 2 entries are STATEFUL. That is, repeated cpuid commands
	 * may return different values. This forces us to get_cpu() before
	 * issuing the first command, and also to emulate this annoying behavior
	 * in kvm_emulate_cpuid() using KVM_CPUID_FLAG_STATE_READ_NEXT */
	case 2: {
		int t, times = entry->eax & 0xff;

		entry->flags |= KVM_CPUID_FLAG_STATEFUL_FUNC;
		entry->flags |= KVM_CPUID_FLAG_STATE_READ_NEXT;
		for (t = 1; t < times; ++t) {
			if (*nent >= maxnent)
				goto out;

			do_cpuid_1_ent(&entry[t], function, 0);
			entry[t].flags |= KVM_CPUID_FLAG_STATEFUL_FUNC;
			++*nent;
		}
		break;
	}
	/* function 4 has additional index. */
	case 4: {
		int i, cache_type;

		entry->flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
		/* read more entries until cache_type is zero */
		for (i = 1; ; ++i) {
			if (*nent >= maxnent)
				goto out;

			cache_type = entry[i - 1].eax & 0x1f;
			if (!cache_type)
				break;
			do_cpuid_1_ent(&entry[i], function, i);
			entry[i].flags |=
			       KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
			++*nent;
		}
		break;
	}
	case 6: /* Thermal management */
		entry->eax = 0x4; /* allow ARAT */
		entry->ebx = 0;
		entry->ecx = 0;
		entry->edx = 0;
		break;
	case 7: {
		entry->flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
		/* Mask ebx against host capability word 9 */
		if (index == 0) {
			entry->ebx &= kvm_cpuid_7_0_ebx_x86_features;
			cpuid_mask(&entry->ebx, CPUID_7_0_EBX);
			// TSC_ADJUST is emulated
			entry->ebx |= F(TSC_ADJUST);
			entry->ecx &= kvm_cpuid_7_0_ecx_x86_features;
			f_la57 = entry->ecx & F(LA57);
			cpuid_mask(&entry->ecx, CPUID_7_ECX);
			/* Set LA57 based on hardware capability. */
			entry->ecx |= f_la57;
			entry->ecx |= f_umip;
			/* PKU is not yet implemented for shadow paging. */
			if (!tdp_enabled || !boot_cpu_has(X86_FEATURE_OSPKE))
				entry->ecx &= ~F(PKU);
			entry->edx &= kvm_cpuid_7_0_edx_x86_features;
			cpuid_mask(&entry->edx, CPUID_7_EDX);
			/*
			 * We emulate ARCH_CAPABILITIES in software even
			 * if the host doesn't support it.
			 */
			entry->edx |= F(ARCH_CAPABILITIES);
		} else {
			entry->ebx = 0;
			entry->ecx = 0;
			entry->edx = 0;
		}
		entry->eax = 0;
		break;
	}
	case 9:
		break;
	case 0xa: { /* Architectural Performance Monitoring */
		struct x86_pmu_capability cap;
		union cpuid10_eax eax;
		union cpuid10_edx edx;

		perf_get_x86_pmu_capability(&cap);

		/*
		 * Only support guest architectural pmu on a host
		 * with architectural pmu.
		 */
		if (!cap.version)
			memset(&cap, 0, sizeof(cap));

		eax.split.version_id = min(cap.version, 2);
		eax.split.num_counters = cap.num_counters_gp;
		eax.split.bit_width = cap.bit_width_gp;
		eax.split.mask_length = cap.events_mask_len;

		edx.split.num_counters_fixed = cap.num_counters_fixed;
		edx.split.bit_width_fixed = cap.bit_width_fixed;
		edx.split.reserved = 0;

		entry->eax = eax.full;
		entry->ebx = cap.events_mask;
		entry->ecx = 0;
		entry->edx = edx.full;
		break;
	}
	/* function 0xb has additional index. */
	case 0xb: {
		int i, level_type;

		entry->flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
		/* read more entries until level_type is zero */
		for (i = 1; ; ++i) {
			if (*nent >= maxnent)
				goto out;

			level_type = entry[i - 1].ecx & 0xff00;
			if (!level_type)
				break;
			do_cpuid_1_ent(&entry[i], function, i);
			entry[i].flags |=
			       KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
			++*nent;
		}
		break;
	}
	case 0xd: {
		int idx, i;
		u64 supported = kvm_supported_xcr0();

		entry->eax &= supported;
		entry->ebx = xstate_required_size(supported, false);
		entry->ecx = entry->ebx;
		entry->edx &= supported >> 32;
		entry->flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
		if (!supported)
			break;

		for (idx = 1, i = 1; idx < 64; ++idx) {
			u64 mask = ((u64)1 << idx);
			if (*nent >= maxnent)
				goto out;

			do_cpuid_1_ent(&entry[i], function, idx);
			if (idx == 1) {
				entry[i].eax &= kvm_cpuid_D_1_eax_x86_features;
				cpuid_mask(&entry[i].eax, CPUID_D_1_EAX);
				entry[i].ebx = 0;
				if (entry[i].eax & (F(XSAVES)|F(XSAVEC)))
					entry[i].ebx =
						xstate_required_size(supported,
								     true);
			} else {
				if (entry[i].eax == 0 || !(supported & mask))
					continue;
				if (WARN_ON_ONCE(entry[i].ecx & 1))
					continue;
			}
			entry[i].ecx = 0;
			entry[i].edx = 0;
			entry[i].flags |=
			       KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
			++*nent;
			++i;
		}
		break;
	}
	/* Intel PT */
	case 0x14: {
		int t, times = entry->eax;

		if (!f_intel_pt)
			break;

		entry->flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
		for (t = 1; t <= times; ++t) {
			if (*nent >= maxnent)
				goto out;
			do_cpuid_1_ent(&entry[t], function, t);
			entry[t].flags |= KVM_CPUID_FLAG_SIGNIFCANT_INDEX;
			++*nent;
		}
		break;
	}
	case KVM_CPUID_SIGNATURE: {
		static const char signature[12] = "KVMKVMKVM\0\0";
		const u32 *sigptr = (const u32 *)signature;
		entry->eax = KVM_CPUID_FEATURES;
		entry->ebx = sigptr[0];
		entry->ecx = sigptr[1];
		entry->edx = sigptr[2];
		break;
	}
	case KVM_CPUID_FEATURES:
		entry->eax = (1 << KVM_FEATURE_CLOCKSOURCE) |
			     (1 << KVM_FEATURE_NOP_IO_DELAY) |
			     (1 << KVM_FEATURE_CLOCKSOURCE2) |
			     (1 << KVM_FEATURE_ASYNC_PF) |
			     (1 << KVM_FEATURE_PV_EOI) |
			     (1 << KVM_FEATURE_CLOCKSOURCE_STABLE_BIT) |
			     (1 << KVM_FEATURE_PV_UNHALT) |
			     (1 << KVM_FEATURE_PV_TLB_FLUSH) |
			     (1 << KVM_FEATURE_ASYNC_PF_VMEXIT) |
			     (1 << KVM_FEATURE_PV_SEND_IPI);

		if (sched_info_on())
			entry->eax |= (1 << KVM_FEATURE_STEAL_TIME);

		entry->ebx = 0;
		entry->ecx = 0;
		entry->edx = 0;
		break;
	case 0x80000000:
		entry->eax = min(entry->eax, 0x8000001f);
		break;
	case 0x80000001:
		entry->edx &= kvm_cpuid_8000_0001_edx_x86_features;
		cpuid_mask(&entry->edx, CPUID_8000_0001_EDX);
		entry->ecx &= kvm_cpuid_8000_0001_ecx_x86_features;
		cpuid_mask(&entry->ecx, CPUID_8000_0001_ECX);
		break;
	case 0x80000007: /* Advanced power management */
		/* invariant TSC is CPUID.80000007H:EDX[8] */
		entry->edx &= (1 << 8);
		/* mask against host */
		entry->edx &= boot_cpu_data.x86_power;
		entry->eax = entry->ebx = entry->ecx = 0;
		break;
	case 0x80000008: {
		unsigned g_phys_as = (entry->eax >> 16) & 0xff;
		unsigned virt_as = max((entry->eax >> 8) & 0xff, 48U);
		unsigned phys_as = entry->eax & 0xff;

		if (!g_phys_as)
			g_phys_as = phys_as;
		entry->eax = g_phys_as | (virt_as << 8);
		entry->edx = 0;
		/*
		 * IBRS, IBPB and VIRT_SSBD aren't necessarily present in
		 * hardware cpuid
		 */
		if (boot_cpu_has(X86_FEATURE_AMD_IBPB))
			entry->ebx |= F(AMD_IBPB);
		if (boot_cpu_has(X86_FEATURE_AMD_IBRS))
			entry->ebx |= F(AMD_IBRS);
		if (boot_cpu_has(X86_FEATURE_VIRT_SSBD))
			entry->ebx |= F(VIRT_SSBD);
		entry->ebx &= kvm_cpuid_8000_0008_ebx_x86_features;
		cpuid_mask(&entry->ebx, CPUID_8000_0008_EBX);
		/*
		 * The preference is to use SPEC CTRL MSR instead of the
		 * VIRT_SPEC MSR.
		 */
		if (boot_cpu_has(X86_FEATURE_LS_CFG_SSBD) &&
		    !boot_cpu_has(X86_FEATURE_AMD_SSBD))
			entry->ebx |= F(VIRT_SSBD);
		break;
	}
	case 0x80000019:
		entry->ecx = entry->edx = 0;
		break;
	case 0x8000001a:
		break;
	case 0x8000001d:
		break;
	/*Add support for Centaur's CPUID instruction*/
	case 0xC0000000:
		/*Just support up to 0xC0000004 now*/
		entry->eax = min(entry->eax, 0xC0000004);
		break;
	case 0xC0000001:
		entry->edx &= kvm_cpuid_C000_0001_edx_x86_features;
		cpuid_mask(&entry->edx, CPUID_C000_0001_EDX);
		break;
	case 3: /* Processor serial number */
	case 5: /* MONITOR/MWAIT */
	case 0xC0000002:
	case 0xC0000003:
	case 0xC0000004:
	default:
		entry->eax = entry->ebx = entry->ecx = entry->edx = 0;
		break;
	}

	kvm_x86_ops->set_supported_cpuid(function, entry);

	r = 0;

out:
	put_cpu();

	return r;
}