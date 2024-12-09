 * Create and join an anonymous session keyring or join a named session
 * keyring, creating it if necessary.  A named session keyring must have Search
 * permission for it to be joined.  Session keyrings without this permit will
 * be skipped over.
 *
 * If successful, the ID of the joined session keyring will be returned.
 */
long keyctl_join_session_keyring(const char __user *_name)
			ret = PTR_ERR(name);
			goto error;
		}
	}

	/* join the session */
	ret = join_session_keyring(name);
	kfree(name);

error:
	return ret;
}

 * Read or set the default keyring in which request_key() will cache keys and
 * return the old setting.
 *
 * If a process keyring is specified then this will be created if it doesn't
 * yet exist.  The old setting will be returned if successful.
 */
long keyctl_set_reqkey_keyring(int reqkey_defl)
{
	struct cred *new;

	case KEY_REQKEY_DEFL_PROCESS_KEYRING:
		ret = install_process_keyring_to_cred(new);
		if (ret < 0) {
			if (ret != -EEXIST)
				goto error;
			ret = 0;
		}
		goto set;

	case KEY_REQKEY_DEFL_DEFAULT:
	case KEY_REQKEY_DEFL_SESSION_KEYRING: