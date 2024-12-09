	if (!cpu_feature_enabled(X86_FEATURE_MPX))
		return MPX_INVALID_BOUNDS_DIR;

	/*
	 * The bounds directory pointer is stored in a register
	 * only accessible if we first do an xsave.
	 */