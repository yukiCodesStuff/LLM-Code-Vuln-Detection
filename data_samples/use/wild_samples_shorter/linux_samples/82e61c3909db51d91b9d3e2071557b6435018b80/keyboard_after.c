		return;

	if ((unsigned)value < ARRAY_SIZE(func_table)) {
		unsigned long flags;

		spin_lock_irqsave(&func_buf_lock, flags);
		if (func_table[value])
			puts_queue(vc, func_table[value]);
		spin_unlock_irqrestore(&func_buf_lock, flags);

	} else
		pr_err("k_fn called with value=%d\n", value);
}

#undef s
#undef v

/* FIXME: This one needs untangling */
int vt_do_kdgkb_ioctl(int cmd, struct kbsentry __user *user_kdgkb, int perm)
{
	struct kbsentry *kbs;
	u_char *q;
	switch (cmd) {
	case KDGKBSENT: {
		/* size should have been a struct member */
		ssize_t len = sizeof(user_kdgkb->kb_string);

		spin_lock_irqsave(&func_buf_lock, flags);
		len = strlcpy(kbs->kb_string, func_table[i] ? : "", len);
		spin_unlock_irqrestore(&func_buf_lock, flags);

		ret = copy_to_user(user_kdgkb->kb_string, kbs->kb_string,
				len + 1) ? -EFAULT : 0;

		goto reterr;
	}
	case KDSKBSENT: