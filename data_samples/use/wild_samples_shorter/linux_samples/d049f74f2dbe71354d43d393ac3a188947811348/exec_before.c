	return (ret > SUID_DUMP_USER) ? SUID_DUMP_ROOT : ret;
}

int get_dumpable(struct mm_struct *mm)
{
	return __get_dumpable(mm->flags);
}