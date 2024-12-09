	if (!cpu_feature_enabled(X86_FEATURE_MPX))
		return MPX_INVALID_BOUNDS_DIR;

	/*
	 * 32-bit binaries on 64-bit kernels are currently
	 * unsupported.
	 */
	if (IS_ENABLED(CONFIG_X86_64) && test_thread_flag(TIF_IA32))
		return MPX_INVALID_BOUNDS_DIR;
	/*
	 * The bounds directory pointer is stored in a register
	 * only accessible if we first do an xsave.
	 */