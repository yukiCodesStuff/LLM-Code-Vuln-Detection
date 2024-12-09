{
	struct user_struct *user;
	const struct cred *cred;
	struct key *uid_keyring, *session_keyring;
	key_perm_t user_keyring_perm;
	char buf[20];
	int ret;
	uid_t uid;

	user_keyring_perm = (KEY_POS_ALL & ~KEY_POS_SETATTR) | KEY_USR_ALL;
	cred = current_cred();
	user = cred->user;
	uid = from_kuid(cred->user_ns, user->uid);

	kenter("%p{%u}", user, uid);

	if (user->uid_keyring) {
		kleave(" = 0 [exist]");
		return 0;
	}