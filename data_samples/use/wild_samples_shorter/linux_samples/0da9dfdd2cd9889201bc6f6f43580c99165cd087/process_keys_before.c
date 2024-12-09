
	kenter("%p{%u}", user, uid);

	if (user->uid_keyring) {
		kleave(" = 0 [exist]");
		return 0;
	}
