	if (!fl->cctx->rpdev)
		return -EPIPE;

	if (handle == FASTRPC_INIT_HANDLE && !kernel) {
		dev_warn_ratelimited(fl->sctx->dev, "user app trying to send a kernel RPC message (%d)\n",  handle);
		return -EPERM;
	}

	ctx = fastrpc_context_alloc(fl, kernel, sc, args);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);
