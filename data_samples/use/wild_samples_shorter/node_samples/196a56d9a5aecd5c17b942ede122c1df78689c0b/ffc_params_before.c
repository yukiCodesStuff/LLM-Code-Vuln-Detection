    dst->h = src->h;
    dst->gindex = src->gindex;
    dst->flags = src->flags;
    return 1;
}

int ossl_ffc_params_cmp(const FFC_PARAMS *a, const FFC_PARAMS *b, int ignore_q)