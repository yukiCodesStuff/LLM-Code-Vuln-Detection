
#ifdef HAVE_DTRACE
	if (DTRACE_EXCEPTION_CAUGHT_ENABLED()) {
		DTRACE_EXCEPTION_CAUGHT(ce->name);
	}
#endif /* HAVE_DTRACE */

	if (ce != catch_ce) {