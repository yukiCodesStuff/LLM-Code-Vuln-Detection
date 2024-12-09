{
	unsigned long max;

	if (mmu_has_feature(MMU_FTR_68_BIT_VA))
		max = MAX_USER_CONTEXT;
	else
		max = MAX_USER_CONTEXT_65BIT_VA;

	return alloc_context_id(MIN_USER_CONTEXT, max);
}
EXPORT_SYMBOL_GPL(hash__alloc_context_id);

void slb_setup_new_exec(void);

static int realloc_context_ids(mm_context_t *ctx)
{
	int i, id;

	/*
	 * id 0 (aka. ctx->id) is special, we always allocate a new one, even if
	 * there wasn't one allocated previously (which happens in the exec
	 * case where ctx is newly allocated).
	 *
	 * We have to be a bit careful here. We must keep the existing ids in
	 * the array, so that we can test if they're non-zero to decide if we
	 * need to allocate a new one. However in case of error we must free the
	 * ids we've allocated but *not* any of the existing ones (or risk a
	 * UAF). That's why we decrement i at the start of the error handling
	 * loop, to skip the id that we just tested but couldn't reallocate.
	 */
	for (i = 0; i < ARRAY_SIZE(ctx->extended_id); i++) {
		if (i == 0 || ctx->extended_id[i]) {
			id = hash__alloc_context_id();
			if (id < 0)
				goto error;

			ctx->extended_id[i] = id;
		}
	}

	/* The caller expects us to return id */
	return ctx->id;

error:
	for (i--; i >= 0; i--) {
		if (ctx->extended_id[i])
			ida_free(&mmu_context_ida, ctx->extended_id[i]);
	}

	return id;
}
		if (current->mm->context.hash_context->spt) {
			mm->context.hash_context->spt = kmalloc(sizeof(struct subpage_prot_table),
								GFP_KERNEL);
			if (!mm->context.hash_context->spt) {
				kfree(mm->context.hash_context);
				return -ENOMEM;
			}