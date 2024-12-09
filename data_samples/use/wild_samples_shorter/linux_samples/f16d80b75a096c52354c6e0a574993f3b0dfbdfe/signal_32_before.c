			goto bad;

		if (MSR_TM_ACTIVE(msr_hi<<32)) {
			/* We only recheckpoint on return if we're
			 * transaction.
			 */
			tm_restore = 1;