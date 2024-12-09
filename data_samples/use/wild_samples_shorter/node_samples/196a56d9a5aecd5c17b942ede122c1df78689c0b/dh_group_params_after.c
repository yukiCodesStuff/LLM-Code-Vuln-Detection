    if (dh == NULL)
        return NULL;

    ossl_ffc_named_group_set(&dh->params, group);
    dh->params.nid = ossl_ffc_named_group_get_uid(group);
    dh->dirty_cnt++;
    return dh;
}
                                                    dh->params.g)) != NULL) {
        if (dh->params.q == NULL)
            dh->params.q = (BIGNUM *)ossl_ffc_named_group_get_q(group);
        /* cache the nid and default key length */
        dh->params.nid = ossl_ffc_named_group_get_uid(group);
        dh->params.keylength = ossl_ffc_named_group_get_keylength(group);
        dh->dirty_cnt++;
    }
}
