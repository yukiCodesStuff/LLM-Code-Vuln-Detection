{
	if (is_hash_blacklisted(hash, hash_len, "bin") == -EKEYREJECTED)
		return -EPERM;

	return 0;
}
EXPORT_SYMBOL_GPL(is_binary_blacklisted);

#ifdef CONFIG_SYSTEM_REVOCATION_LIST
/**
 * add_key_to_revocation_list - Add a revocation certificate to the blacklist
 * @data: The data blob containing the certificate
 * @size: The size of data blob
 */
int add_key_to_revocation_list(const char *data, size_t size)
{
	key_ref_t key;

	key = key_create_or_update(make_key_ref(blacklist_keyring, true),
				   "asymmetric",
				   NULL,
				   data,
				   size,
				   ((KEY_POS_ALL & ~KEY_POS_SETATTR) | KEY_USR_VIEW),
				   KEY_ALLOC_NOT_IN_QUOTA | KEY_ALLOC_BUILT_IN);

	if (IS_ERR(key)) {
		pr_err("Problem with revocation key (%ld)\n", PTR_ERR(key));
		return PTR_ERR(key);
	}

	return 0;
}