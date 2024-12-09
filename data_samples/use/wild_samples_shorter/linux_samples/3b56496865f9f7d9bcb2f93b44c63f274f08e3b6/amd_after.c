			set_cpu_cap(c, X86_FEATURE_EXTD_APICID);
	}
#endif

	/* F16h erratum 793, CVE-2013-6885 */
	if (c->x86 == 0x16 && c->x86_model <= 0xf) {
		u64 val;

		rdmsrl(MSR_AMD64_LS_CFG, val);
		if (!(val & BIT(15)))
			wrmsrl(MSR_AMD64_LS_CFG, val | BIT(15));
	}

}

static const int amd_erratum_383[];
static const int amd_erratum_400[];