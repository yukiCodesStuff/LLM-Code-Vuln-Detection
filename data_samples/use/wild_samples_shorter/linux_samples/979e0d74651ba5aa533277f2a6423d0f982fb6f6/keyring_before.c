
	kenter("{%d}", key->serial);

	BUG_ON(key != ctx->match_data);
	ctx->result = ERR_PTR(-EDEADLK);
	return 1;
}
