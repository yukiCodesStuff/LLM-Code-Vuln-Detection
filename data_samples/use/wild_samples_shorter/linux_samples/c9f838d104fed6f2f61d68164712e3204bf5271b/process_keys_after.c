}

/*
 * Install a thread keyring to the given credentials struct if it didn't have
 * one already.  This is allowed to overrun the quota.
 *
 * Return: 0 if a thread keyring is now present; -errno on failure.
 */
int install_thread_keyring_to_cred(struct cred *new)
{
	struct key *keyring;

	if (new->thread_keyring)
		return 0;

	keyring = keyring_alloc("_tid", new->uid, new->gid, new,
				KEY_POS_ALL | KEY_USR_VIEW,
				KEY_ALLOC_QUOTA_OVERRUN,
				NULL, NULL);
}

/*
 * Install a thread keyring to the current task if it didn't have one already.
 *
 * Return: 0 if a thread keyring is now present; -errno on failure.
 */
static int install_thread_keyring(void)
{
	struct cred *new;
	if (!new)
		return -ENOMEM;

	ret = install_thread_keyring_to_cred(new);
	if (ret < 0) {
		abort_creds(new);
		return ret;
}

/*
 * Install a process keyring to the given credentials struct if it didn't have
 * one already.  This is allowed to overrun the quota.
 *
 * Return: 0 if a process keyring is now present; -errno on failure.
 */
int install_process_keyring_to_cred(struct cred *new)
{
	struct key *keyring;

	if (new->process_keyring)
		return 0;

	keyring = keyring_alloc("_pid", new->uid, new->gid, new,
				KEY_POS_ALL | KEY_USR_VIEW,
				KEY_ALLOC_QUOTA_OVERRUN,
}

/*
 * Install a process keyring to the current task if it didn't have one already.
 *
 * Return: 0 if a process keyring is now present; -errno on failure.
 */
static int install_process_keyring(void)
{
	struct cred *new;
	ret = install_process_keyring_to_cred(new);
	if (ret < 0) {
		abort_creds(new);
		return ret;
	}

	return commit_creds(new);
}

/*
 * Install the given keyring as the session keyring of the given credentials
 * struct, replacing the existing one if any.  If the given keyring is NULL,
 * then install a new anonymous session keyring.
 *
 * Return: 0 on success; -errno on failure.
 */
int install_session_keyring_to_cred(struct cred *cred, struct key *keyring)
{
	unsigned long flags;
}

/*
 * Install the given keyring as the session keyring of the current task,
 * replacing the existing one if any.  If the given keyring is NULL, then
 * install a new anonymous session keyring.
 *
 * Return: 0 on success; -errno on failure.
 */
static int install_session_keyring(struct key *keyring)
{
	struct cred *new;