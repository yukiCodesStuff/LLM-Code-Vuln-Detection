
	r = -E2BIG;

	if (WARN_ON(*nent >= maxnent))
		goto out;

	do_host_cpuid(entry, function, 0);
	++*nent;
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
static int do_cpuid_func(struct kvm_cpuid_entry2 *entry, u32 func,
			 int *nent, int maxnent, unsigned int type)
{
	if (*nent >= maxnent)
		return -E2BIG;

	if (type == KVM_GET_EMULATED_CPUID)
		return __do_cpuid_func_emulated(entry, func, nent, maxnent);

	return __do_cpuid_func(entry, func, nent, maxnent);