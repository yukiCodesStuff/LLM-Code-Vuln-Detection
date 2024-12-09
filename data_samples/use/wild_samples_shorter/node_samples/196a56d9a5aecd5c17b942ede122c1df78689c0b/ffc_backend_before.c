        if (prm->data_type != OSSL_PARAM_UTF8_STRING
            || prm->data == NULL
            || (group = ossl_ffc_name_to_dh_named_group(prm->data)) == NULL
            || !ossl_ffc_named_group_set_pqg(ffc, group))
#endif
            goto err;
    }
