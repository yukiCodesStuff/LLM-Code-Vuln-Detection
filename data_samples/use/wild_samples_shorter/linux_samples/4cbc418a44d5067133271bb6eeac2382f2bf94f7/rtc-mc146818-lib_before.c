	save_control = CMOS_READ(RTC_CONTROL);
	CMOS_WRITE((save_control|RTC_SET), RTC_CONTROL);
	save_freq_select = CMOS_READ(RTC_FREQ_SELECT);

#ifdef CONFIG_X86
	if ((boot_cpu_data.x86_vendor == X86_VENDOR_AMD &&
	     boot_cpu_data.x86 == 0x17) ||
	     boot_cpu_data.x86_vendor == X86_VENDOR_HYGON) {
		CMOS_WRITE((save_freq_select & (~RTC_DIV_RESET2)),
			RTC_FREQ_SELECT);
		save_freq_select &= ~RTC_DIV_RESET2;
	} else
		CMOS_WRITE((save_freq_select | RTC_DIV_RESET2),
			RTC_FREQ_SELECT);
#else
	CMOS_WRITE((save_freq_select | RTC_DIV_RESET2), RTC_FREQ_SELECT);
#endif

#ifdef CONFIG_MACH_DECSTATION
	CMOS_WRITE(real_yrs, RTC_DEC_YEAR);
#endif