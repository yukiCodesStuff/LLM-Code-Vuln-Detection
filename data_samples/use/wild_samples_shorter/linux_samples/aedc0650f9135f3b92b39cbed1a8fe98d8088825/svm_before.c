		if (npt_enabled)
			entry->edx |= F(NPT);

		break;
	case 0x8000001F:
		/* Support memory encryption cpuid if host supports it */
		if (boot_cpu_has(X86_FEATURE_SEV))
			cpuid(0x8000001f, &entry->eax, &entry->ebx,
				&entry->ecx, &entry->edx);

	}
}

static int svm_get_lpage_level(void)