	if (!fl->cctx->rpdev)
		return -EPIPE;

	ctx = fastrpc_context_alloc(fl, kernel, sc, args);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
