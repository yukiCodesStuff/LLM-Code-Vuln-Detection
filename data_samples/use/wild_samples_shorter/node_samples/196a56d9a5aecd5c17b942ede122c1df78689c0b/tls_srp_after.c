    BN_clear_free(s->srp_ctx.s);
    s->srp_ctx.s = NULL;
    if (!SRP_create_verifier_BN_ex(user, pass, &s->srp_ctx.s, &s->srp_ctx.v,
                                   s->srp_ctx.N, s->srp_ctx.g, s->ctx->libctx,
                                   s->ctx->propq))
        return -1;

    return 1;