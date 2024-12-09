
void ossl_sa_free(OPENSSL_SA *sa)
{
    if (sa != NULL) {
        sa_doall(sa, &sa_free_node, NULL, NULL);
        OPENSSL_free(sa);
    }
}

void ossl_sa_free_leaves(OPENSSL_SA *sa)
{