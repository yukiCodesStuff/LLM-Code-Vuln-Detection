	return (ret > SUID_DUMP_USER) ? SUID_DUMP_ROOT : ret;
}

/*
 * This returns the actual value of the suid_dumpable flag. For things
 * that are using this for checking for privilege transitions, it must
 * test against SUID_DUMP_USER rather than treating it as a boolean
 * value.
 */
int get_dumpable(struct mm_struct *mm)
{
	return __get_dumpable(mm->flags);
}