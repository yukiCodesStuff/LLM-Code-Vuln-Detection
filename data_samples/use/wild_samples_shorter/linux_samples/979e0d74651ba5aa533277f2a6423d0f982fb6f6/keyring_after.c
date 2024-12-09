
	kenter("{%d}", key->serial);

	/* We might get a keyring with matching index-key that is nonetheless a
	 * different keyring. */
	if (key != ctx->match_data)
		return 0;

	ctx->result = ERR_PTR(-EDEADLK);
	return 1;
}
