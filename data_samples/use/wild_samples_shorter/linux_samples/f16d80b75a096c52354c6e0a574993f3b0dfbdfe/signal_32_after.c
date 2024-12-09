			goto bad;

		if (MSR_TM_ACTIVE(msr_hi<<32)) {
			/* Trying to start TM on non TM system */
			if (!cpu_has_feature(CPU_FTR_TM))
				goto bad;
			/* We only recheckpoint on return if we're
			 * transaction.
			 */
			tm_restore = 1;