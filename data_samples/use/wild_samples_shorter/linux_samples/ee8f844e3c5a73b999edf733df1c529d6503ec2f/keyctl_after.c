 * Create and join an anonymous session keyring or join a named session
 * keyring, creating it if necessary.  A named session keyring must have Search
 * permission for it to be joined.  Session keyrings without this permit will
 * be skipped over.  It is not permitted for userspace to create or join
 * keyrings whose name begin with a dot.
 *
 * If successful, the ID of the joined session keyring will be returned.
 */
long keyctl_join_session_keyring(const char __user *_name)
			ret = PTR_ERR(name);
			goto error;
		}

		ret = -EPERM;
		if (name[0] == '.')
			goto error_name;
	}

	/* join the session */
	ret = join_session_keyring(name);
error_name:
	kfree(name);
error:
	return ret;
}
