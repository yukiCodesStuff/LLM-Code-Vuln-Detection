		ret = PTR_ERR(keyring);
		goto error2;
	} else if (keyring == new->session_keyring) {
		key_put(keyring);
		ret = 0;
		goto error2;
	}
