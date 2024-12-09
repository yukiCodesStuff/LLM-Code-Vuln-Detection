    if (ctx->session_cache_mode & SSL_SESS_CACHE_UPDATE_TIME) {
        c->time = time(NULL);
        ssl_session_calculate_timeout(c);
    }

    if (s == NULL) {
        /*
         * new cache entry -- remove old ones if cache has become too large
         * delete cache entry *before* add, so we don't remove the one we're adding!
         */

        ret = 1;

        if (SSL_CTX_sess_get_cache_size(ctx) > 0) {
            while (SSL_CTX_sess_number(ctx) >= SSL_CTX_sess_get_cache_size(ctx)) {
                if (!remove_session_lock(ctx, ctx->session_cache_tail, 0))
                    break;
                else
                    ssl_tsan_counter(ctx, &ctx->stats.sess_cache_full);
            }
        }