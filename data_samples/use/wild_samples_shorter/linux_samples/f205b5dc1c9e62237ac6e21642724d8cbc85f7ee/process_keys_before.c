}

/*
 * Install a fresh thread keyring directly to new credentials.  This keyring is
 * allowed to overrun the quota.
 */
int install_thread_keyring_to_cred(struct cred *new)
{
	struct key *keyring;

	keyring = keyring_alloc("_tid", new->uid, new->gid, new,
				KEY_POS_ALL | KEY_USR_VIEW,
				KEY_ALLOC_QUOTA_OVERRUN,
				NULL, NULL);
}

/*
 * Install a fresh thread keyring, discarding the old one.
 */
static int install_thread_keyring(void)
{
	struct cred *new;
	if (!new)
		return -ENOMEM;

	BUG_ON(new->thread_keyring);

	ret = install_thread_keyring_to_cred(new);
	if (ret < 0) {
		abort_creds(new);
		return ret;
}

/*
 * Install a process keyring directly to a credentials struct.
 *
 * Returns -EEXIST if there was already a process keyring, 0 if one installed,
 * and other value on any other error
 */
int install_process_keyring_to_cred(struct cred *new)
{
	struct key *keyring;

	if (new->process_keyring)
		return -EEXIST;

	keyring = keyring_alloc("_pid", new->uid, new->gid, new,
				KEY_POS_ALL | KEY_USR_VIEW,
				KEY_ALLOC_QUOTA_OVERRUN,
}

/*
 * Make sure a process keyring is installed for the current process.  The
 * existing process keyring is not replaced.
 *
 * Returns 0 if there is a process keyring by the end of this function, some
 * error otherwise.
 */
static int install_process_keyring(void)
{
	struct cred *new;
	ret = install_process_keyring_to_cred(new);
	if (ret < 0) {
		abort_creds(new);
		return ret != -EEXIST ? ret : 0;
	}

	return commit_creds(new);
}

/*
 * Install a session keyring directly to a credentials struct.
 */
int install_session_keyring_to_cred(struct cred *cred, struct key *keyring)
{
	unsigned long flags;
}

/*
 * Install a session keyring, discarding the old one.  If a keyring is not
 * supplied, an empty one is invented.
 */
static int install_session_keyring(struct key *keyring)
{
	struct cred *new;