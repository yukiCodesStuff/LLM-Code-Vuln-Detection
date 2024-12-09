	if (current != p) {
		/* SELinux only allows a process to change its own
		   security attributes. */
		return -EACCES;
	}

	/*
	 * Basic control over ability to set these attributes at all.
	 * current == p, but we'll pass them separately in case the
	 * above restriction is ever removed.
	 */
	if (!strcmp(name, "exec"))
		error = current_has_perm(p, PROCESS__SETEXEC);
	else if (!strcmp(name, "fscreate"))
		error = current_has_perm(p, PROCESS__SETFSCREATE);
	else if (!strcmp(name, "keycreate"))
		error = current_has_perm(p, PROCESS__SETKEYCREATE);
	else if (!strcmp(name, "sockcreate"))
		error = current_has_perm(p, PROCESS__SETSOCKCREATE);
	else if (!strcmp(name, "current"))
		error = current_has_perm(p, PROCESS__SETCURRENT);
	else
		error = -EINVAL;
	if (error)
		return error;

	/* Obtain a SID for the context, if one was specified. */
	if (size && str[1] && str[1] != '\n') {
		if (str[size-1] == '\n') {
			str[size-1] = 0;
			size--;
		}
		error = security_context_to_sid(value, size, &sid, GFP_KERNEL);
		if (error == -EINVAL && !strcmp(name, "fscreate")) {
			if (!capable(CAP_MAC_ADMIN)) {
				struct audit_buffer *ab;
				size_t audit_size;

				/* We strip a nul only if it is at the end, otherwise the
				 * context contains a nul and we should audit that */
				if (str[size - 1] == '\0')
					audit_size = size - 1;
				else
					audit_size = size;
				ab = audit_log_start(current->audit_context, GFP_ATOMIC, AUDIT_SELINUX_ERR);
				audit_log_format(ab, "op=fscreate invalid_context=");
				audit_log_n_untrustedstring(ab, value, audit_size);
				audit_log_end(ab);

				return error;
			}
			error = security_context_to_sid_force(value, size,
							      &sid);
		}
		if (error)
			return error;
	}