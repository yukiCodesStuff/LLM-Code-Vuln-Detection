	unsigned f_intel_pt = kvm_x86_ops->pt_supported() ? F(INTEL_PT) : 0;

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
		F(CLZERO) | F(XSAVEERPTR) |
		F(WBNOINVD) | F(AMD_IBPB) | F(AMD_IBRS) | F(AMD_SSBD) | F(VIRT_SSBD) |
		F(AMD_SSB_NO) | F(AMD_STIBP) | F(AMD_STIBP_ALWAYS_ON);

	/* cpuid 0xC0000001.edx */
	const u32 kvm_cpuid_C000_0001_edx_x86_features =
		F(XSTORE) | F(XSTORE_EN) | F(XCRYPT) | F(XCRYPT_EN) |
		F(ACE2) | F(ACE2_EN) | F(PHE) | F(PHE_EN) |
		F(PMM) | F(PMM_EN);

	/* cpuid 0xD.1.eax */
	const u32 kvm_cpuid_D_1_eax_x86_features =
		F(XSAVEOPT) | F(XSAVEC) | F(XGETBV1) | f_xsaves;

	/* all calls to cpuid_count() should be made on the same cpu */
	get_cpu();

	r = -E2BIG;

	if (WARN_ON(*nent >= maxnent))
		goto out;

	do_host_cpuid(entry, function, 0);
	++*nent;

	switch (function) {
	case 0:
		/* Limited to the highest leaf implemented in KVM. */
		entry->eax = min(entry->eax, 0x1fU);
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

		entry->flags |= KVM_CPUID_FLAG_STATE_READ_NEXT;
		for (t = 1; t < times; ++t) {
			if (*nent >= maxnent)
				goto out;

			do_host_cpuid(&entry[t], function, 0);
			++*nent;
		}
		break;
	}
	/* functions 4 and 0x8000001d have additional index. */
	case 4:
	case 0x8000001d: {
		int i, cache_type;

		/* read more entries until cache_type is zero */
		for (i = 1; ; ++i) {
			if (*nent >= maxnent)
				goto out;

			cache_type = entry[i - 1].eax & 0x1f;
			if (!cache_type)
				break;
			do_host_cpuid(&entry[i], function, i);
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
	/* function 7 has additional index. */
	case 7: {
		int i;

		for (i = 0; ; ) {
			do_cpuid_7_mask(&entry[i], i);
			if (i == entry->eax)
				break;
			if (*nent >= maxnent)
				goto out;

			++i;
			do_host_cpuid(&entry[i], function, i);
			++*nent;
		}
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
	case 0x80000008: {
		unsigned g_phys_as = (entry->eax >> 16) & 0xff;
		unsigned virt_as = max((entry->eax >> 8) & 0xff, 48U);
		unsigned phys_as = entry->eax & 0xff;

		if (!g_phys_as)
			g_phys_as = phys_as;
		entry->eax = g_phys_as | (virt_as << 8);
		entry->edx = 0;
		entry->ebx &= kvm_cpuid_8000_0008_ebx_x86_features;
		cpuid_mask(&entry->ebx, CPUID_8000_0008_EBX);
		/*
		 * AMD has separate bits for each SPEC_CTRL bit.
		 * arch/x86/kernel/cpu/bugs.c is kind enough to
		 * record that in cpufeatures so use them.
		 */
		if (boot_cpu_has(X86_FEATURE_IBPB))
			entry->ebx |= F(AMD_IBPB);
		if (boot_cpu_has(X86_FEATURE_IBRS))
			entry->ebx |= F(AMD_IBRS);
		if (boot_cpu_has(X86_FEATURE_STIBP))
			entry->ebx |= F(AMD_STIBP);
		if (boot_cpu_has(X86_FEATURE_SSBD))
			entry->ebx |= F(AMD_SSBD);
		if (!boot_cpu_has_bug(X86_BUG_SPEC_STORE_BYPASS))
			entry->ebx |= F(AMD_SSB_NO);
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
	case 0x8000001e:
		break;
	/* Support memory encryption cpuid if host supports it */
	case 0x8000001F:
		if (!boot_cpu_has(X86_FEATURE_SEV))
			entry->eax = entry->ebx = entry->ecx = entry->edx = 0;
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

static int do_cpuid_func(struct kvm_cpuid_entry2 *entry, u32 func,
			 int *nent, int maxnent, unsigned int type)
{
	if (*nent >= maxnent)
		return -E2BIG;

	if (type == KVM_GET_EMULATED_CPUID)
		return __do_cpuid_func_emulated(entry, func, nent, maxnent);

	return __do_cpuid_func(entry, func, nent, maxnent);
}