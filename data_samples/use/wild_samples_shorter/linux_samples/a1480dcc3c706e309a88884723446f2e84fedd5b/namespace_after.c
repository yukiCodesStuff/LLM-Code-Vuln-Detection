		 * Special case for "unmounting" root ...
		 * we just try to remount it readonly.
		 */
		if (!capable(CAP_SYS_ADMIN))
			return -EPERM;
		down_write(&sb->s_umount);
		if (!(sb->s_flags & MS_RDONLY))
			retval = do_remount_sb(sb, MS_RDONLY, NULL, 0);
		up_write(&sb->s_umount);