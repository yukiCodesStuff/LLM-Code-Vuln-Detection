{
    OPENSSL_free(p);
}

void ossl_sa_free(OPENSSL_SA *sa)
{
    if (sa != NULL) {
        sa_doall(sa, &sa_free_node, NULL, NULL);
        OPENSSL_free(sa);
    }
}