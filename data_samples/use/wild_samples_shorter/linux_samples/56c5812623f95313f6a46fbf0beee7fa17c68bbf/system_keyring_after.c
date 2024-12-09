			pr_devel("PKCS#7 platform keyring is not available\n");
			goto error;
		}

		ret = is_key_on_revocation_list(pkcs7);
		if (ret != -ENOKEY) {
			pr_devel("PKCS#7 platform key is on revocation list\n");
			goto error;
		}
	}
	ret = pkcs7_validate_trust(pkcs7, trusted_keys);
	if (ret < 0) {
		if (ret == -ENOKEY)