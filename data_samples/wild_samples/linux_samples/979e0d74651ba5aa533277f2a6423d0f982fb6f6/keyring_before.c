{
	struct keyring_search_context *ctx = iterator_data;
	const struct key *key = keyring_ptr_to_key(object);

	kenter("{%d}", key->serial);

	BUG_ON(key != ctx->match_data);
	ctx->result = ERR_PTR(-EDEADLK);
	return 1;
}

/*
 * See if a cycle will will be created by inserting acyclic tree B in acyclic
 * tree A at the topmost level (ie: as a direct child of A).
 *
 * Since we are adding B to A at the top level, checking for cycles should just
 * be a matter of seeing if node A is somewhere in tree B.
 */
static int keyring_detect_cycle(struct key *A, struct key *B)
{
	struct keyring_search_context ctx = {
		.index_key	= A->index_key,
		.match_data	= A,
		.iterator	= keyring_detect_cycle_iterator,
		.flags		= (KEYRING_SEARCH_LOOKUP_DIRECT |
				   KEYRING_SEARCH_NO_STATE_CHECK |
				   KEYRING_SEARCH_NO_UPDATE_TIME |
				   KEYRING_SEARCH_NO_CHECK_PERM |
				   KEYRING_SEARCH_DETECT_TOO_DEEP),
	};

	rcu_read_lock();
	search_nested_keyrings(B, &ctx);
	rcu_read_unlock();
	return PTR_ERR(ctx.result) == -EAGAIN ? 0 : PTR_ERR(ctx.result);
}