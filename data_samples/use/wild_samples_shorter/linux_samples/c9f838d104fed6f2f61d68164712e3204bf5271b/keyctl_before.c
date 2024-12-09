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