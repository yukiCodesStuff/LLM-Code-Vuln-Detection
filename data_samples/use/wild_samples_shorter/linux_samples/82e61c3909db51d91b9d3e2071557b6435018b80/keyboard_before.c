		return;

	if ((unsigned)value < ARRAY_SIZE(func_table)) {
		if (func_table[value])
			puts_queue(vc, func_table[value]);
	} else
		pr_err("k_fn called with value=%d\n", value);
}

#undef s
#undef v

/* FIXME: This one needs untangling and locking */
int vt_do_kdgkb_ioctl(int cmd, struct kbsentry __user *user_kdgkb, int perm)
{
	struct kbsentry *kbs;
	u_char *q;
	switch (cmd) {
	case KDGKBSENT: {
		/* size should have been a struct member */
		unsigned char *from = func_table[i] ? : "";

		ret = copy_to_user(user_kdgkb->kb_string, from,
				strlen(from) + 1) ? -EFAULT : 0;

		goto reterr;
	}
	case KDSKBSENT: