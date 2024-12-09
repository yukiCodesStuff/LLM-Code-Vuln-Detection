{
    return strcmp(a[DB_name], b[DB_name]);
}

static IMPLEMENT_LHASH_HASH_FN(index_serial, OPENSSL_CSTRING)
static IMPLEMENT_LHASH_COMP_FN(index_serial, OPENSSL_CSTRING)
static IMPLEMENT_LHASH_HASH_FN(index_name, OPENSSL_CSTRING)
static IMPLEMENT_LHASH_COMP_FN(index_name, OPENSSL_CSTRING)
#undef BSIZE
#define BSIZE 256
BIGNUM *load_serial(const char *serialfile, int *exists, int create,
                    ASN1_INTEGER **retai)
{
    BIO *in = NULL;
    BIGNUM *ret = NULL;
    char buf[1024];
    ASN1_INTEGER *ai = NULL;

    ai = ASN1_INTEGER_new();
    if (ai == NULL)
        goto err;

    in = BIO_new_file(serialfile, "r");
    if (exists != NULL)
        *exists = in != NULL;
    if (in == NULL) {
        if (!create) {
            perror(serialfile);
            goto err;
        }
        ERR_clear_error();
        ret = BN_new();
        if (ret == NULL) {
            BIO_printf(bio_err, "Out of memory\n");
        } else if (!rand_serial(ret, ai)) {
            BIO_printf(bio_err, "Error creating random number to store in %s\n",
                       serialfile);
            BN_free(ret);
            ret = NULL;
        }
    } else {
        if (!a2i_ASN1_INTEGER(in, ai, buf, 1024)) {
            BIO_printf(bio_err, "Unable to load number from %s\n",
                       serialfile);
            goto err;
        }
        ret = ASN1_INTEGER_to_BN(ai, NULL);
        if (ret == NULL) {
            BIO_printf(bio_err, "Error converting number from bin to BIGNUM\n");
            goto err;
        }
    }

    if (ret != NULL && retai != NULL) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (ret == NULL)
        ERR_print_errors(bio_err);
    BIO_free(in);
    ASN1_INTEGER_free(ai);
    return ret;
}

int save_serial(const char *serialfile, const char *suffix, const BIGNUM *serial,
                ASN1_INTEGER **retai)
{
    char buf[1][BSIZE];
    BIO *out = NULL;
    int ret = 0;
    ASN1_INTEGER *ai = NULL;
    int j;

    if (suffix == NULL)
        j = strlen(serialfile);
    else
        j = strlen(serialfile) + strlen(suffix) + 1;
    if (j >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }

    if (suffix == NULL)
        OPENSSL_strlcpy(buf[0], serialfile, BSIZE);
    else {
#ifndef OPENSSL_SYS_VMS
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, suffix);
#else
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, suffix);
#endif
    }
    out = BIO_new_file(buf[0], "w");
    if (out == NULL) {
        goto err;
    }

    if ((ai = BN_to_ASN1_INTEGER(serial, NULL)) == NULL) {
        BIO_printf(bio_err, "error converting serial to ASN.1 format\n");
        goto err;
    }
    i2a_ASN1_INTEGER(out, ai);
    BIO_puts(out, "\n");
    ret = 1;
    if (retai) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (!ret)
        ERR_print_errors(bio_err);
    BIO_free_all(out);
    ASN1_INTEGER_free(ai);
    return ret;
}

int rotate_serial(const char *serialfile, const char *new_suffix,
                  const char *old_suffix)
{
    char buf[2][BSIZE];
    int i, j;

    i = strlen(serialfile) + strlen(old_suffix);
    j = strlen(serialfile) + strlen(new_suffix);
    if (i > j)
        j = i;
    if (j + 1 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.%s", serialfile, old_suffix);
#else
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-%s", serialfile, old_suffix);
#endif
    if (rename(serialfile, buf[1]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", serialfile, buf[1]);
        perror("reason");
        goto err;
    }
    if (rename(buf[0], serialfile) < 0) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", buf[0], serialfile);
        perror("reason");
        rename(buf[1], serialfile);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int rand_serial(BIGNUM *b, ASN1_INTEGER *ai)
{
    BIGNUM *btmp;
    int ret = 0;

    btmp = b == NULL ? BN_new() : b;
    if (btmp == NULL)
        return 0;

    if (!BN_rand(btmp, SERIAL_RAND_BITS, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY))
        goto error;
    if (ai && !BN_to_ASN1_INTEGER(btmp, ai))
        goto error;

    ret = 1;

 error:

    if (btmp != b)
        BN_free(btmp);

    return ret;
}

CA_DB *load_index(const char *dbfile, DB_ATTR *db_attr)
{
    CA_DB *retdb = NULL;
    TXT_DB *tmpdb = NULL;
    BIO *in;
    CONF *dbattr_conf = NULL;
    char buf[BSIZE];
#ifndef OPENSSL_NO_POSIX_IO
    FILE *dbfp;
    struct stat dbst;
#endif

    in = BIO_new_file(dbfile, "r");
    if (in == NULL)
        goto err;

#ifndef OPENSSL_NO_POSIX_IO
    BIO_get_fp(in, &dbfp);
    if (fstat(fileno(dbfp), &dbst) == -1) {
        ERR_raise_data(ERR_LIB_SYS, errno,
                       "calling fstat(%s)", dbfile);
        goto err;
    }
#endif

    if ((tmpdb = TXT_DB_read(in, DB_NUMBER)) == NULL)
        goto err;

#ifndef OPENSSL_SYS_VMS
    BIO_snprintf(buf, sizeof(buf), "%s.attr", dbfile);
#else
    BIO_snprintf(buf, sizeof(buf), "%s-attr", dbfile);
#endif
    dbattr_conf = app_load_config_quiet(buf);

    retdb = app_malloc(sizeof(*retdb), "new DB");
    retdb->db = tmpdb;
    tmpdb = NULL;
    if (db_attr)
        retdb->attributes = *db_attr;
    else {
        retdb->attributes.unique_subject = 1;
    }

    if (dbattr_conf) {
        char *p = NCONF_get_string(dbattr_conf, NULL, "unique_subject");
        if (p) {
            retdb->attributes.unique_subject = parse_yesno(p, 1);
        }
    }

    retdb->dbfname = OPENSSL_strdup(dbfile);
#ifndef OPENSSL_NO_POSIX_IO
    retdb->dbst = dbst;
#endif

 err:
    ERR_print_errors(bio_err);
    NCONF_free(dbattr_conf);
    TXT_DB_free(tmpdb);
    BIO_free_all(in);
    return retdb;
}

/*
 * Returns > 0 on success, <= 0 on error
 */
int index_index(CA_DB *db)
{
    if (!TXT_DB_create_index(db->db, DB_serial, NULL,
                             LHASH_HASH_FN(index_serial),
                             LHASH_COMP_FN(index_serial))) {
        BIO_printf(bio_err,
                   "Error creating serial number index:(%ld,%ld,%ld)\n",
                   db->db->error, db->db->arg1, db->db->arg2);
        goto err;
    }

    if (db->attributes.unique_subject
        && !TXT_DB_create_index(db->db, DB_name, index_name_qual,
                                LHASH_HASH_FN(index_name),
                                LHASH_COMP_FN(index_name))) {
        BIO_printf(bio_err, "Error creating name index:(%ld,%ld,%ld)\n",
                   db->db->error, db->db->arg1, db->db->arg2);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int save_index(const char *dbfile, const char *suffix, CA_DB *db)
{
    char buf[3][BSIZE];
    BIO *out;
    int j;

    j = strlen(dbfile) + strlen(suffix);
    if (j + 6 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s.attr", dbfile);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.attr.%s", dbfile, suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", dbfile, suffix);
#else
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s-attr", dbfile);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-attr-%s", dbfile, suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", dbfile, suffix);
#endif
    out = BIO_new_file(buf[0], "w");
    if (out == NULL) {
        perror(dbfile);
        BIO_printf(bio_err, "Unable to open '%s'\n", dbfile);
        goto err;
    }
    j = TXT_DB_write(out, db->db);
    BIO_free(out);
    if (j <= 0)
        goto err;

    out = BIO_new_file(buf[1], "w");
    if (out == NULL) {
        perror(buf[2]);
        BIO_printf(bio_err, "Unable to open '%s'\n", buf[2]);
        goto err;
    }
    BIO_printf(out, "unique_subject = %s\n",
               db->attributes.unique_subject ? "yes" : "no");
    BIO_free(out);

    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int rotate_index(const char *dbfile, const char *new_suffix,
                 const char *old_suffix)
{
    char buf[5][BSIZE];
    int i, j;

    i = strlen(dbfile) + strlen(old_suffix);
    j = strlen(dbfile) + strlen(new_suffix);
    if (i > j)
        j = i;
    if (j + 6 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[4], sizeof(buf[4]), "%s.attr", dbfile);
    j = BIO_snprintf(buf[3], sizeof(buf[3]), "%s.attr.%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s.attr.%s", dbfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", dbfile, new_suffix);
#else
    j = BIO_snprintf(buf[4], sizeof(buf[4]), "%s-attr", dbfile);
    j = BIO_snprintf(buf[3], sizeof(buf[3]), "%s-attr-%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s-attr-%s", dbfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", dbfile, new_suffix);
#endif
    if (rename(dbfile, buf[1]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", dbfile, buf[1]);
        perror("reason");
        goto err;
    }
    if (rename(buf[0], dbfile) < 0) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[0], dbfile);
        perror("reason");
        rename(buf[1], dbfile);
        goto err;
    }
    if (rename(buf[4], buf[3]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[4], buf[3]);
        perror("reason");
        rename(dbfile, buf[0]);
        rename(buf[1], dbfile);
        goto err;
    }
    if (rename(buf[2], buf[4]) < 0) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[2], buf[4]);
        perror("reason");
        rename(buf[3], buf[4]);
        rename(dbfile, buf[0]);
        rename(buf[1], dbfile);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

void free_index(CA_DB *db)
{
    if (db) {
        TXT_DB_free(db->db);
        OPENSSL_free(db->dbfname);
        OPENSSL_free(db);
    }
}

int parse_yesno(const char *str, int def)
{
    if (str) {
        switch (*str) {
        case 'f':              /* false */
        case 'F':              /* FALSE */
        case 'n':              /* no */
        case 'N':              /* NO */
        case '0':              /* 0 */
            return 0;
        case 't':              /* true */
        case 'T':              /* TRUE */
        case 'y':              /* yes */
        case 'Y':              /* YES */
        case '1':              /* 1 */
            return 1;
        }
    }
    return def;
}

/*
 * name is expected to be in the format /type0=value0/type1=value1/type2=...
 * where + can be used instead of / to form multi-valued RDNs if canmulti
 * and characters may be escaped by \
 */
X509_NAME *parse_name(const char *cp, int chtype, int canmulti,
                      const char *desc)
{
    int nextismulti = 0;
    char *work;
    X509_NAME *n;

    if (*cp++ != '/') {
        BIO_printf(bio_err,
                   "%s: %s name is expected to be in the format "
                   "/type0=value0/type1=value1/type2=... where characters may "
                   "be escaped by \\. This name is not in that format: '%s'\n",
                   opt_getprog(), desc, --cp);
        return NULL;
    }

    n = X509_NAME_new();
    if (n == NULL) {
        BIO_printf(bio_err, "%s: Out of memory\n", opt_getprog());
        return NULL;
    }
    work = OPENSSL_strdup(cp);
    if (work == NULL) {
        BIO_printf(bio_err, "%s: Error copying %s name input\n",
                   opt_getprog(), desc);
        goto err;
    }

    while (*cp != '\0') {
        char *bp = work;
        char *typestr = bp;
        unsigned char *valstr;
        int nid;
        int ismulti = nextismulti;
        nextismulti = 0;

        /* Collect the type */
        while (*cp != '\0' && *cp != '=')
            *bp++ = *cp++;
        *bp++ = '\0';
        if (*cp == '\0') {
            BIO_printf(bio_err,
                       "%s: Missing '=' after RDN type string '%s' in %s name string\n",
                       opt_getprog(), typestr, desc);
            goto err;
        }
        ++cp;

        /* Collect the value. */
        valstr = (unsigned char *)bp;
        for (; *cp != '\0' && *cp != '/'; *bp++ = *cp++) {
            /* unescaped '+' symbol string signals further member of multiRDN */
            if (canmulti && *cp == '+') {
                nextismulti = 1;
                break;
            }
            if (*cp == '\\' && *++cp == '\0') {
                BIO_printf(bio_err,
                           "%s: Escape character at end of %s name string\n",
                           opt_getprog(), desc);
                goto err;
            }
        }
        *bp++ = '\0';

        /* If not at EOS (must be + or /), move forward. */
        if (*cp != '\0')
            ++cp;

        /* Parse */
        nid = OBJ_txt2nid(typestr);
        if (nid == NID_undef) {
            BIO_printf(bio_err,
                       "%s: Skipping unknown %s name attribute \"%s\"\n",
                       opt_getprog(), desc, typestr);
            if (ismulti)
                BIO_printf(bio_err,
                           "Hint: a '+' in a value string needs be escaped using '\\' else a new member of a multi-valued RDN is expected\n");
            continue;
        }
        if (*valstr == '\0') {
            BIO_printf(bio_err,
                       "%s: No value provided for %s name attribute \"%s\", skipped\n",
                       opt_getprog(), desc, typestr);
            continue;
        }
        if (!X509_NAME_add_entry_by_NID(n, nid, chtype,
                                        valstr, strlen((char *)valstr),
                                        -1, ismulti ? -1 : 0)) {
            ERR_print_errors(bio_err);
            BIO_printf(bio_err,
                       "%s: Error adding %s name attribute \"/%s=%s\"\n",
                       opt_getprog(), desc, typestr ,valstr);
            goto err;
        }
    }

    OPENSSL_free(work);
    return n;

 err:
    X509_NAME_free(n);
    OPENSSL_free(work);
    return NULL;
}

/*
 * Read whole contents of a BIO into an allocated memory buffer and return
 * it.
 */

int bio_to_mem(unsigned char **out, int maxlen, BIO *in)
{
    BIO *mem;
    int len, ret;
    unsigned char tbuf[1024];

    mem = BIO_new(BIO_s_mem());
    if (mem == NULL)
        return -1;
    for (;;) {
        if ((maxlen != -1) && maxlen < 1024)
            len = maxlen;
        else
            len = 1024;
        len = BIO_read(in, tbuf, len);
        if (len < 0) {
            BIO_free(mem);
            return -1;
        }
        if (len == 0)
            break;
        if (BIO_write(mem, tbuf, len) != len) {
            BIO_free(mem);
            return -1;
        }
        maxlen -= len;

        if (maxlen == 0)
            break;
    }
    ret = BIO_get_mem_data(mem, (char **)out);
    BIO_set_flags(mem, BIO_FLAGS_MEM_RDONLY);
    BIO_free(mem);
    return ret;
}

int pkey_ctrl_string(EVP_PKEY_CTX *ctx, const char *value)
{
    int rv = 0;
    char *stmp, *vtmp = NULL;

    stmp = OPENSSL_strdup(value);
    if (stmp == NULL)
        return -1;
    vtmp = strchr(stmp, ':');
    if (vtmp == NULL)
        goto err;

    *vtmp = 0;
    vtmp++;
    rv = EVP_PKEY_CTX_ctrl_str(ctx, stmp, vtmp);

 err:
    OPENSSL_free(stmp);
    return rv;
}

static void nodes_print(const char *name, STACK_OF(X509_POLICY_NODE) *nodes)
{
    X509_POLICY_NODE *node;
    int i;

    BIO_printf(bio_err, "%s Policies:", name);
    if (nodes) {
        BIO_puts(bio_err, "\n");
        for (i = 0; i < sk_X509_POLICY_NODE_num(nodes); i++) {
            node = sk_X509_POLICY_NODE_value(nodes, i);
            X509_POLICY_NODE_print(bio_err, node, 2);
        }
    } else {
        BIO_puts(bio_err, " <empty>\n");
    }
}

void policies_print(X509_STORE_CTX *ctx)
{
    X509_POLICY_TREE *tree;
    int explicit_policy;
    tree = X509_STORE_CTX_get0_policy_tree(ctx);
    explicit_policy = X509_STORE_CTX_get_explicit_policy(ctx);

    BIO_printf(bio_err, "Require explicit Policy: %s\n",
               explicit_policy ? "True" : "False");

    nodes_print("Authority", X509_policy_tree_get0_policies(tree));
    nodes_print("User", X509_policy_tree_get0_user_policies(tree));
}

/*-
 * next_protos_parse parses a comma separated list of strings into a string
 * in a format suitable for passing to SSL_CTX_set_next_protos_advertised.
 *   outlen: (output) set to the length of the resulting buffer on success.
 *   err: (maybe NULL) on failure, an error message line is written to this BIO.
 *   in: a NUL terminated string like "abc,def,ghi"
 *
 *   returns: a malloc'd buffer or NULL on failure.
 */
unsigned char *next_protos_parse(size_t *outlen, const char *in)
{
    size_t len;
    unsigned char *out;
    size_t i, start = 0;
    size_t skipped = 0;

    len = strlen(in);
    if (len == 0 || len >= 65535)
        return NULL;

    out = app_malloc(len + 1, "NPN buffer");
    for (i = 0; i <= len; ++i) {
        if (i == len || in[i] == ',') {
            /*
             * Zero-length ALPN elements are invalid on the wire, we could be
             * strict and reject the entire string, but just ignoring extra
             * commas seems harmless and more friendly.
             *
             * Every comma we skip in this way puts the input buffer another
             * byte ahead of the output buffer, so all stores into the output
             * buffer need to be decremented by the number commas skipped.
             */
            if (i == start) {
                ++start;
                ++skipped;
                continue;
            }
            if (i - start > 255) {
                OPENSSL_free(out);
                return NULL;
            }
            out[start-skipped] = (unsigned char)(i - start);
            start = i + 1;
        } else {
            out[i + 1 - skipped] = in[i];
        }
    }

    if (len <= skipped) {
        OPENSSL_free(out);
        return NULL;
    }

    *outlen = len + 1 - skipped;
    return out;
}

void print_cert_checks(BIO *bio, X509 *x,
                       const char *checkhost,
                       const char *checkemail, const char *checkip)
{
    if (x == NULL)
        return;
    if (checkhost) {
        BIO_printf(bio, "Hostname %s does%s match certificate\n",
                   checkhost,
                   X509_check_host(x, checkhost, 0, 0, NULL) == 1
                       ? "" : " NOT");
    }

    if (checkemail) {
        BIO_printf(bio, "Email %s does%s match certificate\n",
                   checkemail, X509_check_email(x, checkemail, 0, 0)
                   ? "" : " NOT");
    }

    if (checkip) {
        BIO_printf(bio, "IP %s does%s match certificate\n",
                   checkip, X509_check_ip_asc(x, checkip, 0) ? "" : " NOT");
    }
}

static int do_pkey_ctx_init(EVP_PKEY_CTX *pkctx, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (pkey_ctrl_string(pkctx, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_x509_init(X509 *x, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (x509_ctrl_string(x, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_x509_req_init(X509_REQ *x, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (x509_req_ctrl_string(x, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_sign_init(EVP_MD_CTX *ctx, EVP_PKEY *pkey,
                        const char *md, STACK_OF(OPENSSL_STRING) *sigopts)
{
    EVP_PKEY_CTX *pkctx = NULL;
    char def_md[80];

    if (ctx == NULL)
        return 0;
    /*
     * EVP_PKEY_get_default_digest_name() returns 2 if the digest is mandatory
     * for this algorithm.
     */
    if (EVP_PKEY_get_default_digest_name(pkey, def_md, sizeof(def_md)) == 2
            && strcmp(def_md, "UNDEF") == 0) {
        /* The signing algorithm requires there to be no digest */
        md = NULL;
    }

    return EVP_DigestSignInit_ex(ctx, &pkctx, md, app_get0_libctx(),
                                 app_get0_propq(), pkey, NULL)
        && do_pkey_ctx_init(pkctx, sigopts);
}

static int adapt_keyid_ext(X509 *cert, X509V3_CTX *ext_ctx,
                           const char *name, const char *value, int add_default)
{
    const STACK_OF(X509_EXTENSION) *exts = X509_get0_extensions(cert);
    X509_EXTENSION *new_ext = X509V3_EXT_nconf(NULL, ext_ctx, name, value);
    int idx, rv = 0;

    if (new_ext == NULL)
        return rv;

    idx = X509v3_get_ext_by_OBJ(exts, X509_EXTENSION_get_object(new_ext), -1);
    if (idx >= 0) {
        X509_EXTENSION *found_ext = X509v3_get_ext(exts, idx);
        ASN1_OCTET_STRING *data = X509_EXTENSION_get_data(found_ext);
        int disabled = ASN1_STRING_length(data) <= 2; /* config said "none" */

        if (disabled) {
            X509_delete_ext(cert, idx);
            X509_EXTENSION_free(found_ext);
        } /* else keep existing key identifier, which might be outdated */
        rv = 1;
    } else  {
        rv = !add_default || X509_add_ext(cert, new_ext, -1);
    }
    X509_EXTENSION_free(new_ext);
    return rv;
}

/* Ensure RFC 5280 compliance, adapt keyIDs as needed, and sign the cert info */
int do_X509_sign(X509 *cert, EVP_PKEY *pkey, const char *md,
                 STACK_OF(OPENSSL_STRING) *sigopts, X509V3_CTX *ext_ctx)
{
    const STACK_OF(X509_EXTENSION) *exts = X509_get0_extensions(cert);
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();
    int self_sign;
    int rv = 0;

    if (sk_X509_EXTENSION_num(exts /* may be NULL */) > 0) {
        /* Prevent X509_V_ERR_EXTENSIONS_REQUIRE_VERSION_3 */
        if (!X509_set_version(cert, X509_VERSION_3))
            goto end;

        /*
         * Add default SKID before such that default AKID can make use of it
         * in case the certificate is self-signed
         */
        /* Prevent X509_V_ERR_MISSING_SUBJECT_KEY_IDENTIFIER */
        if (!adapt_keyid_ext(cert, ext_ctx, "subjectKeyIdentifier", "hash", 1))
            goto end;
        /* Prevent X509_V_ERR_MISSING_AUTHORITY_KEY_IDENTIFIER */
        ERR_set_mark();
        self_sign = X509_check_private_key(cert, pkey);
        ERR_pop_to_mark();
        if (!adapt_keyid_ext(cert, ext_ctx, "authorityKeyIdentifier",
                             "keyid, issuer", !self_sign))
            goto end;
    }

    if (mctx != NULL && do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_sign_ctx(cert, mctx) > 0);
 end:
    EVP_MD_CTX_free(mctx);
    return rv;
}

/* Sign the certificate request info */
int do_X509_REQ_sign(X509_REQ *x, EVP_PKEY *pkey, const char *md,
                     STACK_OF(OPENSSL_STRING) *sigopts)
{
    int rv = 0;
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();

    if (do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_REQ_sign_ctx(x, mctx) > 0);
    EVP_MD_CTX_free(mctx);
    return rv;
}

/* Sign the CRL info */
int do_X509_CRL_sign(X509_CRL *x, EVP_PKEY *pkey, const char *md,
                     STACK_OF(OPENSSL_STRING) *sigopts)
{
    int rv = 0;
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();

    if (do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_CRL_sign_ctx(x, mctx) > 0);
    EVP_MD_CTX_free(mctx);
    return rv;
}

/*
 * do_X509_verify returns 1 if the signature is valid,
 * 0 if the signature check fails, or -1 if error occurs.
 */
int do_X509_verify(X509 *x, EVP_PKEY *pkey, STACK_OF(OPENSSL_STRING) *vfyopts)
{
    int rv = 0;

    if (do_x509_init(x, vfyopts) > 0)
        rv = X509_verify(x, pkey);
    else
        rv = -1;
    return rv;
}

/*
 * do_X509_REQ_verify returns 1 if the signature is valid,
 * 0 if the signature check fails, or -1 if error occurs.
 */
int do_X509_REQ_verify(X509_REQ *x, EVP_PKEY *pkey,
                       STACK_OF(OPENSSL_STRING) *vfyopts)
{
    int rv = 0;

    if (do_x509_req_init(x, vfyopts) > 0)
        rv = X509_REQ_verify_ex(x, pkey,
                                 app_get0_libctx(), app_get0_propq());
    else
        rv = -1;
    return rv;
}

/* Get first http URL from a DIST_POINT structure */

static const char *get_dp_url(DIST_POINT *dp)
{
    GENERAL_NAMES *gens;
    GENERAL_NAME *gen;
    int i, gtype;
    ASN1_STRING *uri;
    if (!dp->distpoint || dp->distpoint->type != 0)
        return NULL;
    gens = dp->distpoint->name.fullname;
    for (i = 0; i < sk_GENERAL_NAME_num(gens); i++) {
        gen = sk_GENERAL_NAME_value(gens, i);
        uri = GENERAL_NAME_get0_value(gen, &gtype);
        if (gtype == GEN_URI && ASN1_STRING_length(uri) > 6) {
            const char *uptr = (const char *)ASN1_STRING_get0_data(uri);

            if (IS_HTTP(uptr)) /* can/should not use HTTPS here */
                return uptr;
        }
    }
    return NULL;
}

/*
 * Look through a CRLDP structure and attempt to find an http URL to
 * downloads a CRL from.
 */

static X509_CRL *load_crl_crldp(STACK_OF(DIST_POINT) *crldp)
{
    int i;
    const char *urlptr = NULL;
    for (i = 0; i < sk_DIST_POINT_num(crldp); i++) {
        DIST_POINT *dp = sk_DIST_POINT_value(crldp, i);
        urlptr = get_dp_url(dp);
        if (urlptr != NULL)
            return load_crl(urlptr, FORMAT_UNDEF, 0, "CRL via CDP");
    }
    return NULL;
}

/*
 * Example of downloading CRLs from CRLDP:
 * not usable for real world as it always downloads and doesn't cache anything.
 */

static STACK_OF(X509_CRL) *crls_http_cb(const X509_STORE_CTX *ctx,
                                        const X509_NAME *nm)
{
    X509 *x;
    STACK_OF(X509_CRL) *crls = NULL;
    X509_CRL *crl;
    STACK_OF(DIST_POINT) *crldp;

    crls = sk_X509_CRL_new_null();
    if (!crls)
        return NULL;
    x = X509_STORE_CTX_get_current_cert(ctx);
    crldp = X509_get_ext_d2i(x, NID_crl_distribution_points, NULL, NULL);
    crl = load_crl_crldp(crldp);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    if (!crl) {
        sk_X509_CRL_free(crls);
        return NULL;
    }
    sk_X509_CRL_push(crls, crl);
    /* Try to download delta CRL */
    crldp = X509_get_ext_d2i(x, NID_freshest_crl, NULL, NULL);
    crl = load_crl_crldp(crldp);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    if (crl)
        sk_X509_CRL_push(crls, crl);
    return crls;
}

void store_setup_crl_download(X509_STORE *st)
{
    X509_STORE_set_lookup_crls_cb(st, crls_http_cb);
}

#ifndef OPENSSL_NO_SOCK
static const char *tls_error_hint(void)
{
    unsigned long err = ERR_peek_error();

    if (ERR_GET_LIB(err) != ERR_LIB_SSL)
        err = ERR_peek_last_error();
    if (ERR_GET_LIB(err) != ERR_LIB_SSL)
        return NULL;

    switch (ERR_GET_REASON(err)) {
    case SSL_R_WRONG_VERSION_NUMBER:
        return "The server does not support (a suitable version of) TLS";
    case SSL_R_UNKNOWN_PROTOCOL:
        return "The server does not support HTTPS";
    case SSL_R_CERTIFICATE_VERIFY_FAILED:
        return "Cannot authenticate server via its TLS certificate, likely due to mismatch with our trusted TLS certs or missing revocation status";
    case SSL_AD_REASON_OFFSET + TLS1_AD_UNKNOWN_CA:
        return "Server did not accept our TLS certificate, likely due to mismatch with server's trust anchor or missing revocation status";
    case SSL_AD_REASON_OFFSET + SSL3_AD_HANDSHAKE_FAILURE:
        return "TLS handshake failure. Possibly the server requires our TLS certificate but did not receive it";
    default: /* no error or no hint available for error */
        return NULL;
    }
}

/* HTTP callback function that supports TLS connection also via HTTPS proxy */
BIO *app_http_tls_cb(BIO *bio, void *arg, int connect, int detail)
{
    APP_HTTP_TLS_INFO *info = (APP_HTTP_TLS_INFO *)arg;
    SSL_CTX *ssl_ctx = info->ssl_ctx;

    if (ssl_ctx == NULL) /* not using TLS */
        return bio;
    if (connect) {
        SSL *ssl;
        BIO *sbio = NULL;

        /* adapt after fixing callback design flaw, see #17088 */
        if ((info->use_proxy
             && !OSSL_HTTP_proxy_connect(bio, info->server, info->port,
                                         NULL, NULL, /* no proxy credentials */
                                         info->timeout, bio_err, opt_getprog()))
                || (sbio = BIO_new(BIO_f_ssl())) == NULL) {
            return NULL;
        }
        if (ssl_ctx == NULL || (ssl = SSL_new(ssl_ctx)) == NULL) {
            BIO_free(sbio);
            return NULL;
        }

        /* adapt after fixing callback design flaw, see #17088 */
        SSL_set_tlsext_host_name(ssl, info->server); /* not critical to do */

        SSL_set_connect_state(ssl);
        BIO_set_ssl(sbio, ssl, BIO_CLOSE);

        bio = BIO_push(sbio, bio);
    }
    if (!connect) {
        const char *hint;
        BIO *cbio;

        if (!detail) { /* disconnecting after error */
            hint = tls_error_hint();
            if (hint != NULL)
                ERR_add_error_data(2, " : ", hint);
        }
        if (ssl_ctx != NULL) {
            (void)ERR_set_mark();
            BIO_ssl_shutdown(bio);
            cbio = BIO_pop(bio); /* connect+HTTP BIO */
            BIO_free(bio); /* SSL BIO */
            (void)ERR_pop_to_mark(); /* hide SSL_R_READ_BIO_NOT_SET etc. */
            bio = cbio;
        }
    }
    return bio;
}

void APP_HTTP_TLS_INFO_free(APP_HTTP_TLS_INFO *info)
{
    if (info != NULL) {
        SSL_CTX_free(info->ssl_ctx);
        OPENSSL_free(info);
    }
}

ASN1_VALUE *app_http_get_asn1(const char *url, const char *proxy,
                              const char *no_proxy, SSL_CTX *ssl_ctx,
                              const STACK_OF(CONF_VALUE) *headers,
                              long timeout, const char *expected_content_type,
                              const ASN1_ITEM *it)
{
    APP_HTTP_TLS_INFO info;
    char *server;
    char *port;
    int use_ssl;
    BIO *mem;
    ASN1_VALUE *resp = NULL;

    if (url == NULL || it == NULL) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }

    if (!OSSL_HTTP_parse_url(url, &use_ssl, NULL /* userinfo */, &server, &port,
                             NULL /* port_num, */, NULL, NULL, NULL))
        return NULL;
    if (use_ssl && ssl_ctx == NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER,
                       "missing SSL_CTX");
        goto end;
    }
    if (!use_ssl && ssl_ctx != NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_INVALID_ARGUMENT,
                       "SSL_CTX given but use_ssl == 0");
        goto end;
    }

    info.server = server;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, server, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    mem = OSSL_HTTP_get(url, proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                        app_http_tls_cb, &info, 0 /* buf_size */, headers,
                        expected_content_type, 1 /* expect_asn1 */,
                        OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout);
    resp = ASN1_item_d2i_bio(it, mem, NULL);
    BIO_free(mem);

 end:
    OPENSSL_free(server);
    OPENSSL_free(port);
    return resp;

}

ASN1_VALUE *app_http_post_asn1(const char *host, const char *port,
                               const char *path, const char *proxy,
                               const char *no_proxy, SSL_CTX *ssl_ctx,
                               const STACK_OF(CONF_VALUE) *headers,
                               const char *content_type,
                               ASN1_VALUE *req, const ASN1_ITEM *req_it,
                               const char *expected_content_type,
                               long timeout, const ASN1_ITEM *rsp_it)
{
    int use_ssl = ssl_ctx != NULL;
    APP_HTTP_TLS_INFO info;
    BIO *rsp, *req_mem = ASN1_item_i2d_mem_bio(req_it, req);
    ASN1_VALUE *res;

    if (req_mem == NULL)
        return NULL;

    info.server = host;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, host, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    rsp = OSSL_HTTP_transfer(NULL, host, port, path, use_ssl,
                             proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                             app_http_tls_cb, &info,
                             0 /* buf_size */, headers, content_type, req_mem,
                             expected_content_type, 1 /* expect_asn1 */,
                             OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout,
                             0 /* keep_alive */);
    BIO_free(req_mem);
    res = ASN1_item_d2i_bio(rsp_it, rsp, NULL);
    BIO_free(rsp);
    return res;
}

#endif

/*
 * Platform-specific sections
 */
#if defined(_WIN32)
# ifdef fileno
#  undef fileno
#  define fileno(a) (int)_fileno(a)
# endif

# include <windows.h>
# include <tchar.h>

static int WIN32_rename(const char *from, const char *to)
{
    TCHAR *tfrom = NULL, *tto;
    DWORD err;
    int ret = 0;

    if (sizeof(TCHAR) == 1) {
        tfrom = (TCHAR *)from;
        tto = (TCHAR *)to;
    } else {                    /* UNICODE path */

        size_t i, flen = strlen(from) + 1, tlen = strlen(to) + 1;
        tfrom = malloc(sizeof(*tfrom) * (flen + tlen));
        if (tfrom == NULL)
            goto err;
        tto = tfrom + flen;
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, from, flen, (WCHAR *)tfrom, flen))
# endif
            for (i = 0; i < flen; i++)
                tfrom[i] = (TCHAR)from[i];
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, to, tlen, (WCHAR *)tto, tlen))
# endif
            for (i = 0; i < tlen; i++)
                tto[i] = (TCHAR)to[i];
    }

    if (MoveFile(tfrom, tto))
        goto ok;
    err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS || err == ERROR_FILE_EXISTS) {
        if (DeleteFile(tto) && MoveFile(tfrom, tto))
            goto ok;
        err = GetLastError();
    }
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
        errno = ENOENT;
    else if (err == ERROR_ACCESS_DENIED)
        errno = EACCES;
    else
        errno = EINVAL;         /* we could map more codes... */
 err:
    ret = -1;
 ok:
    if (tfrom != NULL && tfrom != (TCHAR *)from)
        free(tfrom);
    return ret;
}
#endif

/* app_tminterval section */
#if defined(_WIN32)
double app_tminterval(int stop, int usertime)
{
    FILETIME now;
    double ret = 0;
    static ULARGE_INTEGER tmstart;
    static int warning = 1;
# ifdef _WIN32_WINNT
    static HANDLE proc = NULL;

    if (proc == NULL) {
        if (check_winnt())
            proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                               GetCurrentProcessId());
        if (proc == NULL)
            proc = (HANDLE) - 1;
    }

    if (usertime && proc != (HANDLE) - 1) {
        FILETIME junk;
        GetProcessTimes(proc, &junk, &junk, &junk, &now);
    } else
# endif
    {
        SYSTEMTIME systime;

        if (usertime && warning) {
            BIO_printf(bio_err, "To get meaningful results, run "
                       "this program on idle system.\n");
            warning = 0;
        }
        GetSystemTime(&systime);
        SystemTimeToFileTime(&systime, &now);
    }

    if (stop == TM_START) {
        tmstart.u.LowPart = now.dwLowDateTime;
        tmstart.u.HighPart = now.dwHighDateTime;
    } else {
        ULARGE_INTEGER tmstop;

        tmstop.u.LowPart = now.dwLowDateTime;
        tmstop.u.HighPart = now.dwHighDateTime;

        ret = (__int64)(tmstop.QuadPart - tmstart.QuadPart) * 1e-7;
    }

    return ret;
}
#elif defined(OPENSSL_SYS_VXWORKS)
# include <time.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
# ifdef CLOCK_REALTIME
    static struct timespec tmstart;
    struct timespec now;
# else
    static unsigned long tmstart;
    unsigned long now;
# endif
    static int warning = 1;

    if (usertime && warning) {
        BIO_printf(bio_err, "To get meaningful results, run "
                   "this program on idle system.\n");
        warning = 0;
    }
# ifdef CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, &now);
    if (stop == TM_START)
        tmstart = now;
    else
        ret = ((now.tv_sec + now.tv_nsec * 1e-9)
               - (tmstart.tv_sec + tmstart.tv_nsec * 1e-9));
# else
    now = tickGet();
    if (stop == TM_START)
        tmstart = now;
    else
        ret = (now - tmstart) / (double)sysClkRateGet();
# endif
    return ret;
}

#elif defined(_SC_CLK_TCK)      /* by means of unistd.h */
# include <sys/times.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
    struct tms rus;
    clock_t now = times(&rus);
    static clock_t tmstart;

    if (usertime)
        now = rus.tms_utime;

    if (stop == TM_START) {
        tmstart = now;
    } else {
        long int tck = sysconf(_SC_CLK_TCK);
        ret = (now - tmstart) / (double)tck;
    }

    return ret;
}

#else
# include <sys/time.h>
# include <sys/resource.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
    struct rusage rus;
    struct timeval now;
    static struct timeval tmstart;

    if (usertime)
        getrusage(RUSAGE_SELF, &rus), now = rus.ru_utime;
    else
        gettimeofday(&now, NULL);

    if (stop == TM_START)
        tmstart = now;
    else
        ret = ((now.tv_sec + now.tv_usec * 1e-6)
               - (tmstart.tv_sec + tmstart.tv_usec * 1e-6));

    return ret;
}
#endif

int app_access(const char* name, int flag)
{
#ifdef _WIN32
    return _access(name, flag);
#else
    return access(name, flag);
#endif
}

int app_isdir(const char *name)
{
    return opt_isdir(name);
}

/* raw_read|write section */
#if defined(__VMS)
# include "vms_term_sock.h"
static int stdin_sock = -1;

static void close_stdin_sock(void)
{
    TerminalSocket (TERM_SOCK_DELETE, &stdin_sock);
}

int fileno_stdin(void)
{
    if (stdin_sock == -1) {
        TerminalSocket(TERM_SOCK_CREATE, &stdin_sock);
        atexit(close_stdin_sock);
    }

    return stdin_sock;
}
#else
int fileno_stdin(void)
{
    return fileno(stdin);
}
#endif

int fileno_stdout(void)
{
    return fileno(stdout);
}

#if defined(_WIN32) && defined(STD_INPUT_HANDLE)
int raw_read_stdin(void *buf, int siz)
{
    DWORD n;
    if (ReadFile(GetStdHandle(STD_INPUT_HANDLE), buf, siz, &n, NULL))
        return n;
    else
        return -1;
}
#elif defined(__VMS)
# include <sys/socket.h>

int raw_read_stdin(void *buf, int siz)
{
    return recv(fileno_stdin(), buf, siz, 0);
}
#else
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_read)>
#  endif
# endif
int raw_read_stdin(void *buf, int siz)
{
    return read(fileno_stdin(), buf, siz);
}
#endif

#if defined(_WIN32) && defined(STD_OUTPUT_HANDLE)
int raw_write_stdout(const void *buf, int siz)
{
    DWORD n;
    if (WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, siz, &n, NULL))
        return n;
    else
        return -1;
}
#elif defined(OPENSSL_SYS_TANDEM) && defined(OPENSSL_THREADS) && defined(_SPT_MODEL_)
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_write)>
#  endif
# endif
int raw_write_stdout(const void *buf,int siz)
{
	return write(fileno(stdout),(void*)buf,siz);
}
#else
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_write)>
#  endif
# endif
int raw_write_stdout(const void *buf, int siz)
{
    return write(fileno_stdout(), buf, siz);
}
#endif

/*
 * Centralized handling of input and output files with format specification
 * The format is meant to show what the input and output is supposed to be,
 * and is therefore a show of intent more than anything else.  However, it
 * does impact behavior on some platforms, such as differentiating between
 * text and binary input/output on non-Unix platforms
 */
BIO *dup_bio_in(int format)
{
    return BIO_new_fp(stdin,
                      BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
}

BIO *dup_bio_out(int format)
{
    BIO *b = BIO_new_fp(stdout,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
    void *prefix = NULL;

    if (b == NULL)
        return NULL;

#ifdef OPENSSL_SYS_VMS
    if (FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif

    if (FMT_istext(format)
        && (prefix = getenv("HARNESS_OSSL_PREFIX")) != NULL) {
        b = BIO_push(BIO_new(BIO_f_prefix()), b);
        BIO_set_prefix(b, prefix);
    }

    return b;
}

BIO *dup_bio_err(int format)
{
    BIO *b = BIO_new_fp(stderr,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
#ifdef OPENSSL_SYS_VMS
    if (b != NULL && FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif
    return b;
}

void unbuffer(FILE *fp)
{
/*
 * On VMS, setbuf() will only take 32-bit pointers, and a compilation
 * with /POINTER_SIZE=64 will give off a MAYLOSEDATA2 warning here.
 * However, we trust that the C RTL will never give us a FILE pointer
 * above the first 4 GB of memory, so we simply turn off the warning
 * temporarily.
 */
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment save
# pragma message disable maylosedata2
#endif
    setbuf(fp, NULL);
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment restore
#endif
}

static const char *modestr(char mode, int format)
{
    OPENSSL_assert(mode == 'a' || mode == 'r' || mode == 'w');

    switch (mode) {
    case 'a':
        return FMT_istext(format) ? "a" : "ab";
    case 'r':
        return FMT_istext(format) ? "r" : "rb";
    case 'w':
        return FMT_istext(format) ? "w" : "wb";
    }
    /* The assert above should make sure we never reach this point */
    return NULL;
}

static const char *modeverb(char mode)
{
    switch (mode) {
    case 'a':
        return "appending";
    case 'r':
        return "reading";
    case 'w':
        return "writing";
    }
    return "(doing something)";
}

/*
 * Open a file for writing, owner-read-only.
 */
BIO *bio_open_owner(const char *filename, int format, int private)
{
    FILE *fp = NULL;
    BIO *b = NULL;
    int textmode, bflags;
#ifndef OPENSSL_NO_POSIX_IO
    int fd = -1, mode;
#endif

    if (!private || filename == NULL || strcmp(filename, "-") == 0)
        return bio_open_default(filename, 'w', format);

    textmode = FMT_istext(format);
#ifndef OPENSSL_NO_POSIX_IO
    mode = O_WRONLY;
# ifdef O_CREAT
    mode |= O_CREAT;
# endif
# ifdef O_TRUNC
    mode |= O_TRUNC;
# endif
    if (!textmode) {
# ifdef O_BINARY
        mode |= O_BINARY;
# elif defined(_O_BINARY)
        mode |= _O_BINARY;
# endif
    }

# ifdef OPENSSL_SYS_VMS
    /* VMS doesn't have O_BINARY, it just doesn't make sense.  But,
     * it still needs to know that we're going binary, or fdopen()
     * will fail with "invalid argument"...  so we tell VMS what the
     * context is.
     */
    if (!textmode)
        fd = open(filename, mode, 0600, "ctx=bin");
    else
# endif
        fd = open(filename, mode, 0600);
    if (fd < 0)
        goto err;
    fp = fdopen(fd, modestr('w', format));
#else   /* OPENSSL_NO_POSIX_IO */
    /* Have stdio but not Posix IO, do the best we can */
    fp = fopen(filename, modestr('w', format));
#endif  /* OPENSSL_NO_POSIX_IO */
    if (fp == NULL)
        goto err;
    bflags = BIO_CLOSE;
    if (textmode)
        bflags |= BIO_FP_TEXT;
    b = BIO_new_fp(fp, bflags);
    if (b != NULL)
        return b;

 err:
    BIO_printf(bio_err, "%s: Can't open \"%s\" for writing, %s\n",
               opt_getprog(), filename, strerror(errno));
    ERR_print_errors(bio_err);
    /* If we have fp, then fdopen took over fd, so don't close both. */
    if (fp != NULL)
        fclose(fp);
#ifndef OPENSSL_NO_POSIX_IO
    else if (fd >= 0)
        close(fd);
#endif
    return NULL;
}

static BIO *bio_open_default_(const char *filename, char mode, int format,
                              int quiet)
{
    BIO *ret;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        ret = mode == 'r' ? dup_bio_in(format) : dup_bio_out(format);
        if (quiet) {
            ERR_clear_error();
            return ret;
        }
        if (ret != NULL)
            return ret;
        BIO_printf(bio_err,
                   "Can't open %s, %s\n",
                   mode == 'r' ? "stdin" : "stdout", strerror(errno));
    } else {
        ret = BIO_new_file(filename, modestr(mode, format));
        if (quiet) {
            ERR_clear_error();
            return ret;
        }
        if (ret != NULL)
            return ret;
        BIO_printf(bio_err,
                   "Can't open \"%s\" for %s, %s\n",
                   filename, modeverb(mode), strerror(errno));
    }
    ERR_print_errors(bio_err);
    return NULL;
}

BIO *bio_open_default(const char *filename, char mode, int format)
{
    return bio_open_default_(filename, mode, format, 0);
}

BIO *bio_open_default_quiet(const char *filename, char mode, int format)
{
    return bio_open_default_(filename, mode, format, 1);
}

void wait_for_async(SSL *s)
{
    /* On Windows select only works for sockets, so we simply don't wait  */
#ifndef OPENSSL_SYS_WINDOWS
    int width = 0;
    fd_set asyncfds;
    OSSL_ASYNC_FD *fds;
    size_t numfds;
    size_t i;

    if (!SSL_get_all_async_fds(s, NULL, &numfds))
        return;
    if (numfds == 0)
        return;
    fds = app_malloc(sizeof(OSSL_ASYNC_FD) * numfds, "allocate async fds");
    if (!SSL_get_all_async_fds(s, fds, &numfds)) {
        OPENSSL_free(fds);
        return;
    }

    FD_ZERO(&asyncfds);
    for (i = 0; i < numfds; i++) {
        if (width <= (int)fds[i])
            width = (int)fds[i] + 1;
        openssl_fdset((int)fds[i], &asyncfds);
    }
    select(width, (void *)&asyncfds, NULL, NULL, NULL);
    OPENSSL_free(fds);
#endif
}

/* if OPENSSL_SYS_WINDOWS is defined then so is OPENSSL_SYS_MSDOS */
#if defined(OPENSSL_SYS_MSDOS)
int has_stdin_waiting(void)
{
# if defined(OPENSSL_SYS_WINDOWS)
    HANDLE inhand = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events = 0;
    INPUT_RECORD inputrec;
    DWORD insize = 1;
    BOOL peeked;

    if (inhand == INVALID_HANDLE_VALUE) {
        return 0;
    }

    peeked = PeekConsoleInput(inhand, &inputrec, insize, &events);
    if (!peeked) {
        /* Probably redirected input? _kbhit() does not work in this case */
        if (!feof(stdin)) {
            return 1;
        }
        return 0;
    }
# endif
    return _kbhit();
}
#endif

/* Corrupt a signature by modifying final byte */
void corrupt_signature(const ASN1_STRING *signature)
{
        unsigned char *s = signature->data;
        s[signature->length - 1] ^= 0x1;
}

int set_cert_times(X509 *x, const char *startdate, const char *enddate,
                   int days)
{
    if (startdate == NULL || strcmp(startdate, "today") == 0) {
        if (X509_gmtime_adj(X509_getm_notBefore(x), 0) == NULL)
            return 0;
    } else {
        if (!ASN1_TIME_set_string_X509(X509_getm_notBefore(x), startdate))
            return 0;
    }
    if (enddate == NULL) {
        if (X509_time_adj_ex(X509_getm_notAfter(x), days, 0, NULL)
            == NULL)
            return 0;
    } else if (!ASN1_TIME_set_string_X509(X509_getm_notAfter(x), enddate)) {
        return 0;
    }
    return 1;
}

int set_crl_lastupdate(X509_CRL *crl, const char *lastupdate)
{
    int ret = 0;
    ASN1_TIME *tm = ASN1_TIME_new();

    if (tm == NULL)
        goto end;

    if (lastupdate == NULL) {
        if (X509_gmtime_adj(tm, 0) == NULL)
            goto end;
    } else {
        if (!ASN1_TIME_set_string_X509(tm, lastupdate))
            goto end;
    }

    if (!X509_CRL_set1_lastUpdate(crl, tm))
        goto end;

    ret = 1;
end:
    ASN1_TIME_free(tm);
    return ret;
}

int set_crl_nextupdate(X509_CRL *crl, const char *nextupdate,
                       long days, long hours, long secs)
{
    int ret = 0;
    ASN1_TIME *tm = ASN1_TIME_new();

    if (tm == NULL)
        goto end;

    if (nextupdate == NULL) {
        if (X509_time_adj_ex(tm, days, hours * 60 * 60 + secs, NULL) == NULL)
            goto end;
    } else {
        if (!ASN1_TIME_set_string_X509(tm, nextupdate))
            goto end;
    }

    if (!X509_CRL_set1_nextUpdate(crl, tm))
        goto end;

    ret = 1;
end:
    ASN1_TIME_free(tm);
    return ret;
}

void make_uppercase(char *string)
{
    int i;

    for (i = 0; string[i] != '\0'; i++)
        string[i] = toupper((unsigned char)string[i]);
}

/* This function is defined here due to visibility of bio_err */
int opt_printf_stderr(const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = BIO_vprintf(bio_err, fmt, ap);
    va_end(ap);
    return ret;
}

OSSL_PARAM *app_params_new_from_opts(STACK_OF(OPENSSL_STRING) *opts,
                                     const OSSL_PARAM *paramdefs)
{
    OSSL_PARAM *params = NULL;
    size_t sz = (size_t)sk_OPENSSL_STRING_num(opts);
    size_t params_n;
    char *opt = "", *stmp, *vtmp = NULL;
    int found = 1;

    if (opts == NULL)
        return NULL;

    params = OPENSSL_zalloc(sizeof(OSSL_PARAM) * (sz + 1));
    if (params == NULL)
        return NULL;

    for (params_n = 0; params_n < sz; params_n++) {
        opt = sk_OPENSSL_STRING_value(opts, (int)params_n);
        if ((stmp = OPENSSL_strdup(opt)) == NULL
            || (vtmp = strchr(stmp, ':')) == NULL)
            goto err;
        /* Replace ':' with 0 to terminate the string pointed to by stmp */
        *vtmp = 0;
        /* Skip over the separator so that vmtp points to the value */
        vtmp++;
        if (!OSSL_PARAM_allocate_from_text(&params[params_n], paramdefs,
                                           stmp, vtmp, strlen(vtmp), &found))
            goto err;
        OPENSSL_free(stmp);
    }
    params[params_n] = OSSL_PARAM_construct_end();
    return params;
err:
    OPENSSL_free(stmp);
    BIO_printf(bio_err, "Parameter %s '%s'\n", found ? "error" : "unknown",
               opt);
    ERR_print_errors(bio_err);
    app_params_free(params);
    return NULL;
}

void app_params_free(OSSL_PARAM *params)
{
    int i;

    if (params != NULL) {
        for (i = 0; params[i].key != NULL; ++i)
            OPENSSL_free(params[i].data);
        OPENSSL_free(params);
    }
}

EVP_PKEY *app_keygen(EVP_PKEY_CTX *ctx, const char *alg, int bits, int verbose)
{
    EVP_PKEY *res = NULL;

    if (verbose && alg != NULL) {
        BIO_printf(bio_err, "Generating %s key", alg);
        if (bits > 0)
            BIO_printf(bio_err, " with %d bits\n", bits);
        else
            BIO_printf(bio_err, "\n");
    }
    if (!RAND_status())
        BIO_printf(bio_err, "Warning: generating random key material may take a long time\n"
                   "if the system has a poor entropy source\n");
    if (EVP_PKEY_keygen(ctx, &res) <= 0)
        app_bail_out("%s: Error generating %s key\n", opt_getprog(),
                     alg != NULL ? alg : "asymmetric");
    return res;
}

EVP_PKEY *app_paramgen(EVP_PKEY_CTX *ctx, const char *alg)
{
    EVP_PKEY *res = NULL;

    if (!RAND_status())
        BIO_printf(bio_err, "Warning: generating random key parameters may take a long time\n"
                   "if the system has a poor entropy source\n");
    if (EVP_PKEY_paramgen(ctx, &res) <= 0)
        app_bail_out("%s: Generating %s key parameters failed\n",
                     opt_getprog(), alg != NULL ? alg : "asymmetric");
    return res;
}

/*
 * Return non-zero if the legacy path is still an option.
 * This decision is based on the global command line operations and the
 * behaviour thus far.
 */
int opt_legacy_okay(void)
{
    int provider_options = opt_provider_option_given();
    int libctx = app_get0_libctx() != NULL || app_get0_propq() != NULL;
#ifndef OPENSSL_NO_ENGINE
    ENGINE *e = ENGINE_get_first();

    if (e != NULL) {
        ENGINE_free(e);
        return 1;
    }
#endif
    /*
     * Having a provider option specified or a custom library context or
     * property query, is a sure sign we're not using legacy.
     */
    if (provider_options || libctx)
        return 0;
    return 1;
}
{
    BIO *in = NULL;
    BIGNUM *ret = NULL;
    char buf[1024];
    ASN1_INTEGER *ai = NULL;

    ai = ASN1_INTEGER_new();
    if (ai == NULL)
        goto err;

    in = BIO_new_file(serialfile, "r");
    if (exists != NULL)
        *exists = in != NULL;
    if (in == NULL) {
        if (!create) {
            perror(serialfile);
            goto err;
        }
        ERR_clear_error();
        ret = BN_new();
        if (ret == NULL) {
            BIO_printf(bio_err, "Out of memory\n");
        } else if (!rand_serial(ret, ai)) {
            BIO_printf(bio_err, "Error creating random number to store in %s\n",
                       serialfile);
            BN_free(ret);
            ret = NULL;
        }
    } else {
        if (!a2i_ASN1_INTEGER(in, ai, buf, 1024)) {
            BIO_printf(bio_err, "Unable to load number from %s\n",
                       serialfile);
            goto err;
        }
        ret = ASN1_INTEGER_to_BN(ai, NULL);
        if (ret == NULL) {
            BIO_printf(bio_err, "Error converting number from bin to BIGNUM\n");
            goto err;
        }
    }

    if (ret != NULL && retai != NULL) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (ret == NULL)
        ERR_print_errors(bio_err);
    BIO_free(in);
    ASN1_INTEGER_free(ai);
    return ret;
}

int save_serial(const char *serialfile, const char *suffix, const BIGNUM *serial,
                ASN1_INTEGER **retai)
{
    char buf[1][BSIZE];
    BIO *out = NULL;
    int ret = 0;
    ASN1_INTEGER *ai = NULL;
    int j;

    if (suffix == NULL)
        j = strlen(serialfile);
    else
        j = strlen(serialfile) + strlen(suffix) + 1;
    if (j >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }

    if (suffix == NULL)
        OPENSSL_strlcpy(buf[0], serialfile, BSIZE);
    else {
#ifndef OPENSSL_SYS_VMS
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, suffix);
#else
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, suffix);
#endif
    }
    out = BIO_new_file(buf[0], "w");
    if (out == NULL) {
        goto err;
    }

    if ((ai = BN_to_ASN1_INTEGER(serial, NULL)) == NULL) {
        BIO_printf(bio_err, "error converting serial to ASN.1 format\n");
        goto err;
    }
    i2a_ASN1_INTEGER(out, ai);
    BIO_puts(out, "\n");
    ret = 1;
    if (retai) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (!ret)
        ERR_print_errors(bio_err);
    BIO_free_all(out);
    ASN1_INTEGER_free(ai);
    return ret;
}

int rotate_serial(const char *serialfile, const char *new_suffix,
                  const char *old_suffix)
{
    char buf[2][BSIZE];
    int i, j;

    i = strlen(serialfile) + strlen(old_suffix);
    j = strlen(serialfile) + strlen(new_suffix);
    if (i > j)
        j = i;
    if (j + 1 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.%s", serialfile, old_suffix);
#else
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-%s", serialfile, old_suffix);
#endif
    if (rename(serialfile, buf[1]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", serialfile, buf[1]);
        perror("reason");
        goto err;
    }
    if (rename(buf[0], serialfile) < 0) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", buf[0], serialfile);
        perror("reason");
        rename(buf[1], serialfile);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int rand_serial(BIGNUM *b, ASN1_INTEGER *ai)
{
    BIGNUM *btmp;
    int ret = 0;

    btmp = b == NULL ? BN_new() : b;
    if (btmp == NULL)
        return 0;

    if (!BN_rand(btmp, SERIAL_RAND_BITS, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY))
        goto error;
    if (ai && !BN_to_ASN1_INTEGER(btmp, ai))
        goto error;

    ret = 1;

 error:

    if (btmp != b)
        BN_free(btmp);

    return ret;
}

CA_DB *load_index(const char *dbfile, DB_ATTR *db_attr)
{
    CA_DB *retdb = NULL;
    TXT_DB *tmpdb = NULL;
    BIO *in;
    CONF *dbattr_conf = NULL;
    char buf[BSIZE];
#ifndef OPENSSL_NO_POSIX_IO
    FILE *dbfp;
    struct stat dbst;
#endif

    in = BIO_new_file(dbfile, "r");
    if (in == NULL)
        goto err;

#ifndef OPENSSL_NO_POSIX_IO
    BIO_get_fp(in, &dbfp);
    if (fstat(fileno(dbfp), &dbst) == -1) {
        ERR_raise_data(ERR_LIB_SYS, errno,
                       "calling fstat(%s)", dbfile);
        goto err;
    }
#endif

    if ((tmpdb = TXT_DB_read(in, DB_NUMBER)) == NULL)
        goto err;

#ifndef OPENSSL_SYS_VMS
    BIO_snprintf(buf, sizeof(buf), "%s.attr", dbfile);
#else
    BIO_snprintf(buf, sizeof(buf), "%s-attr", dbfile);
#endif
    dbattr_conf = app_load_config_quiet(buf);

    retdb = app_malloc(sizeof(*retdb), "new DB");
    retdb->db = tmpdb;
    tmpdb = NULL;
    if (db_attr)
        retdb->attributes = *db_attr;
    else {
        retdb->attributes.unique_subject = 1;
    }

    if (dbattr_conf) {
        char *p = NCONF_get_string(dbattr_conf, NULL, "unique_subject");
        if (p) {
            retdb->attributes.unique_subject = parse_yesno(p, 1);
        }
    }

    retdb->dbfname = OPENSSL_strdup(dbfile);
#ifndef OPENSSL_NO_POSIX_IO
    retdb->dbst = dbst;
#endif

 err:
    ERR_print_errors(bio_err);
    NCONF_free(dbattr_conf);
    TXT_DB_free(tmpdb);
    BIO_free_all(in);
    return retdb;
}

/*
 * Returns > 0 on success, <= 0 on error
 */
int index_index(CA_DB *db)
{
    if (!TXT_DB_create_index(db->db, DB_serial, NULL,
                             LHASH_HASH_FN(index_serial),
                             LHASH_COMP_FN(index_serial))) {
        BIO_printf(bio_err,
                   "Error creating serial number index:(%ld,%ld,%ld)\n",
                   db->db->error, db->db->arg1, db->db->arg2);
        goto err;
    }

    if (db->attributes.unique_subject
        && !TXT_DB_create_index(db->db, DB_name, index_name_qual,
                                LHASH_HASH_FN(index_name),
                                LHASH_COMP_FN(index_name))) {
        BIO_printf(bio_err, "Error creating name index:(%ld,%ld,%ld)\n",
                   db->db->error, db->db->arg1, db->db->arg2);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int save_index(const char *dbfile, const char *suffix, CA_DB *db)
{
    char buf[3][BSIZE];
    BIO *out;
    int j;

    j = strlen(dbfile) + strlen(suffix);
    if (j + 6 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s.attr", dbfile);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.attr.%s", dbfile, suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", dbfile, suffix);
#else
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s-attr", dbfile);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-attr-%s", dbfile, suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", dbfile, suffix);
#endif
    out = BIO_new_file(buf[0], "w");
    if (out == NULL) {
        perror(dbfile);
        BIO_printf(bio_err, "Unable to open '%s'\n", dbfile);
        goto err;
    }
    j = TXT_DB_write(out, db->db);
    BIO_free(out);
    if (j <= 0)
        goto err;

    out = BIO_new_file(buf[1], "w");
    if (out == NULL) {
        perror(buf[2]);
        BIO_printf(bio_err, "Unable to open '%s'\n", buf[2]);
        goto err;
    }
    BIO_printf(out, "unique_subject = %s\n",
               db->attributes.unique_subject ? "yes" : "no");
    BIO_free(out);

    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int rotate_index(const char *dbfile, const char *new_suffix,
                 const char *old_suffix)
{
    char buf[5][BSIZE];
    int i, j;

    i = strlen(dbfile) + strlen(old_suffix);
    j = strlen(dbfile) + strlen(new_suffix);
    if (i > j)
        j = i;
    if (j + 6 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[4], sizeof(buf[4]), "%s.attr", dbfile);
    j = BIO_snprintf(buf[3], sizeof(buf[3]), "%s.attr.%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s.attr.%s", dbfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", dbfile, new_suffix);
#else
    j = BIO_snprintf(buf[4], sizeof(buf[4]), "%s-attr", dbfile);
    j = BIO_snprintf(buf[3], sizeof(buf[3]), "%s-attr-%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[2], sizeof(buf[2]), "%s-attr-%s", dbfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-%s", dbfile, old_suffix);
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", dbfile, new_suffix);
#endif
    if (rename(dbfile, buf[1]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", dbfile, buf[1]);
        perror("reason");
        goto err;
    }
    if (rename(buf[0], dbfile) < 0) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[0], dbfile);
        perror("reason");
        rename(buf[1], dbfile);
        goto err;
    }
    if (rename(buf[4], buf[3]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[4], buf[3]);
        perror("reason");
        rename(dbfile, buf[0]);
        rename(buf[1], dbfile);
        goto err;
    }
    if (rename(buf[2], buf[4]) < 0) {
        BIO_printf(bio_err, "Unable to rename %s to %s\n", buf[2], buf[4]);
        perror("reason");
        rename(buf[3], buf[4]);
        rename(dbfile, buf[0]);
        rename(buf[1], dbfile);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

void free_index(CA_DB *db)
{
    if (db) {
        TXT_DB_free(db->db);
        OPENSSL_free(db->dbfname);
        OPENSSL_free(db);
    }
}

int parse_yesno(const char *str, int def)
{
    if (str) {
        switch (*str) {
        case 'f':              /* false */
        case 'F':              /* FALSE */
        case 'n':              /* no */
        case 'N':              /* NO */
        case '0':              /* 0 */
            return 0;
        case 't':              /* true */
        case 'T':              /* TRUE */
        case 'y':              /* yes */
        case 'Y':              /* YES */
        case '1':              /* 1 */
            return 1;
        }
    }
    return def;
}

/*
 * name is expected to be in the format /type0=value0/type1=value1/type2=...
 * where + can be used instead of / to form multi-valued RDNs if canmulti
 * and characters may be escaped by \
 */
X509_NAME *parse_name(const char *cp, int chtype, int canmulti,
                      const char *desc)
{
    int nextismulti = 0;
    char *work;
    X509_NAME *n;

    if (*cp++ != '/') {
        BIO_printf(bio_err,
                   "%s: %s name is expected to be in the format "
                   "/type0=value0/type1=value1/type2=... where characters may "
                   "be escaped by \\. This name is not in that format: '%s'\n",
                   opt_getprog(), desc, --cp);
        return NULL;
    }

    n = X509_NAME_new();
    if (n == NULL) {
        BIO_printf(bio_err, "%s: Out of memory\n", opt_getprog());
        return NULL;
    }
    work = OPENSSL_strdup(cp);
    if (work == NULL) {
        BIO_printf(bio_err, "%s: Error copying %s name input\n",
                   opt_getprog(), desc);
        goto err;
    }

    while (*cp != '\0') {
        char *bp = work;
        char *typestr = bp;
        unsigned char *valstr;
        int nid;
        int ismulti = nextismulti;
        nextismulti = 0;

        /* Collect the type */
        while (*cp != '\0' && *cp != '=')
            *bp++ = *cp++;
        *bp++ = '\0';
        if (*cp == '\0') {
            BIO_printf(bio_err,
                       "%s: Missing '=' after RDN type string '%s' in %s name string\n",
                       opt_getprog(), typestr, desc);
            goto err;
        }
        ++cp;

        /* Collect the value. */
        valstr = (unsigned char *)bp;
        for (; *cp != '\0' && *cp != '/'; *bp++ = *cp++) {
            /* unescaped '+' symbol string signals further member of multiRDN */
            if (canmulti && *cp == '+') {
                nextismulti = 1;
                break;
            }
            if (*cp == '\\' && *++cp == '\0') {
                BIO_printf(bio_err,
                           "%s: Escape character at end of %s name string\n",
                           opt_getprog(), desc);
                goto err;
            }
        }
        *bp++ = '\0';

        /* If not at EOS (must be + or /), move forward. */
        if (*cp != '\0')
            ++cp;

        /* Parse */
        nid = OBJ_txt2nid(typestr);
        if (nid == NID_undef) {
            BIO_printf(bio_err,
                       "%s: Skipping unknown %s name attribute \"%s\"\n",
                       opt_getprog(), desc, typestr);
            if (ismulti)
                BIO_printf(bio_err,
                           "Hint: a '+' in a value string needs be escaped using '\\' else a new member of a multi-valued RDN is expected\n");
            continue;
        }
        if (*valstr == '\0') {
            BIO_printf(bio_err,
                       "%s: No value provided for %s name attribute \"%s\", skipped\n",
                       opt_getprog(), desc, typestr);
            continue;
        }
        if (!X509_NAME_add_entry_by_NID(n, nid, chtype,
                                        valstr, strlen((char *)valstr),
                                        -1, ismulti ? -1 : 0)) {
            ERR_print_errors(bio_err);
            BIO_printf(bio_err,
                       "%s: Error adding %s name attribute \"/%s=%s\"\n",
                       opt_getprog(), desc, typestr ,valstr);
            goto err;
        }
    }

    OPENSSL_free(work);
    return n;

 err:
    X509_NAME_free(n);
    OPENSSL_free(work);
    return NULL;
}

/*
 * Read whole contents of a BIO into an allocated memory buffer and return
 * it.
 */

int bio_to_mem(unsigned char **out, int maxlen, BIO *in)
{
    BIO *mem;
    int len, ret;
    unsigned char tbuf[1024];

    mem = BIO_new(BIO_s_mem());
    if (mem == NULL)
        return -1;
    for (;;) {
        if ((maxlen != -1) && maxlen < 1024)
            len = maxlen;
        else
            len = 1024;
        len = BIO_read(in, tbuf, len);
        if (len < 0) {
            BIO_free(mem);
            return -1;
        }
        if (len == 0)
            break;
        if (BIO_write(mem, tbuf, len) != len) {
            BIO_free(mem);
            return -1;
        }
        maxlen -= len;

        if (maxlen == 0)
            break;
    }
    ret = BIO_get_mem_data(mem, (char **)out);
    BIO_set_flags(mem, BIO_FLAGS_MEM_RDONLY);
    BIO_free(mem);
    return ret;
}

int pkey_ctrl_string(EVP_PKEY_CTX *ctx, const char *value)
{
    int rv = 0;
    char *stmp, *vtmp = NULL;

    stmp = OPENSSL_strdup(value);
    if (stmp == NULL)
        return -1;
    vtmp = strchr(stmp, ':');
    if (vtmp == NULL)
        goto err;

    *vtmp = 0;
    vtmp++;
    rv = EVP_PKEY_CTX_ctrl_str(ctx, stmp, vtmp);

 err:
    OPENSSL_free(stmp);
    return rv;
}

static void nodes_print(const char *name, STACK_OF(X509_POLICY_NODE) *nodes)
{
    X509_POLICY_NODE *node;
    int i;

    BIO_printf(bio_err, "%s Policies:", name);
    if (nodes) {
        BIO_puts(bio_err, "\n");
        for (i = 0; i < sk_X509_POLICY_NODE_num(nodes); i++) {
            node = sk_X509_POLICY_NODE_value(nodes, i);
            X509_POLICY_NODE_print(bio_err, node, 2);
        }
    } else {
        BIO_puts(bio_err, " <empty>\n");
    }
}

void policies_print(X509_STORE_CTX *ctx)
{
    X509_POLICY_TREE *tree;
    int explicit_policy;
    tree = X509_STORE_CTX_get0_policy_tree(ctx);
    explicit_policy = X509_STORE_CTX_get_explicit_policy(ctx);

    BIO_printf(bio_err, "Require explicit Policy: %s\n",
               explicit_policy ? "True" : "False");

    nodes_print("Authority", X509_policy_tree_get0_policies(tree));
    nodes_print("User", X509_policy_tree_get0_user_policies(tree));
}

/*-
 * next_protos_parse parses a comma separated list of strings into a string
 * in a format suitable for passing to SSL_CTX_set_next_protos_advertised.
 *   outlen: (output) set to the length of the resulting buffer on success.
 *   err: (maybe NULL) on failure, an error message line is written to this BIO.
 *   in: a NUL terminated string like "abc,def,ghi"
 *
 *   returns: a malloc'd buffer or NULL on failure.
 */
unsigned char *next_protos_parse(size_t *outlen, const char *in)
{
    size_t len;
    unsigned char *out;
    size_t i, start = 0;
    size_t skipped = 0;

    len = strlen(in);
    if (len == 0 || len >= 65535)
        return NULL;

    out = app_malloc(len + 1, "NPN buffer");
    for (i = 0; i <= len; ++i) {
        if (i == len || in[i] == ',') {
            /*
             * Zero-length ALPN elements are invalid on the wire, we could be
             * strict and reject the entire string, but just ignoring extra
             * commas seems harmless and more friendly.
             *
             * Every comma we skip in this way puts the input buffer another
             * byte ahead of the output buffer, so all stores into the output
             * buffer need to be decremented by the number commas skipped.
             */
            if (i == start) {
                ++start;
                ++skipped;
                continue;
            }
            if (i - start > 255) {
                OPENSSL_free(out);
                return NULL;
            }
            out[start-skipped] = (unsigned char)(i - start);
            start = i + 1;
        } else {
            out[i + 1 - skipped] = in[i];
        }
    }

    if (len <= skipped) {
        OPENSSL_free(out);
        return NULL;
    }

    *outlen = len + 1 - skipped;
    return out;
}

void print_cert_checks(BIO *bio, X509 *x,
                       const char *checkhost,
                       const char *checkemail, const char *checkip)
{
    if (x == NULL)
        return;
    if (checkhost) {
        BIO_printf(bio, "Hostname %s does%s match certificate\n",
                   checkhost,
                   X509_check_host(x, checkhost, 0, 0, NULL) == 1
                       ? "" : " NOT");
    }

    if (checkemail) {
        BIO_printf(bio, "Email %s does%s match certificate\n",
                   checkemail, X509_check_email(x, checkemail, 0, 0)
                   ? "" : " NOT");
    }

    if (checkip) {
        BIO_printf(bio, "IP %s does%s match certificate\n",
                   checkip, X509_check_ip_asc(x, checkip, 0) ? "" : " NOT");
    }
}

static int do_pkey_ctx_init(EVP_PKEY_CTX *pkctx, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (pkey_ctrl_string(pkctx, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_x509_init(X509 *x, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (x509_ctrl_string(x, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_x509_req_init(X509_REQ *x, STACK_OF(OPENSSL_STRING) *opts)
{
    int i;

    if (opts == NULL)
        return 1;

    for (i = 0; i < sk_OPENSSL_STRING_num(opts); i++) {
        char *opt = sk_OPENSSL_STRING_value(opts, i);
        if (x509_req_ctrl_string(x, opt) <= 0) {
            BIO_printf(bio_err, "parameter error \"%s\"\n", opt);
            ERR_print_errors(bio_err);
            return 0;
        }
    }

    return 1;
}

static int do_sign_init(EVP_MD_CTX *ctx, EVP_PKEY *pkey,
                        const char *md, STACK_OF(OPENSSL_STRING) *sigopts)
{
    EVP_PKEY_CTX *pkctx = NULL;
    char def_md[80];

    if (ctx == NULL)
        return 0;
    /*
     * EVP_PKEY_get_default_digest_name() returns 2 if the digest is mandatory
     * for this algorithm.
     */
    if (EVP_PKEY_get_default_digest_name(pkey, def_md, sizeof(def_md)) == 2
            && strcmp(def_md, "UNDEF") == 0) {
        /* The signing algorithm requires there to be no digest */
        md = NULL;
    }

    return EVP_DigestSignInit_ex(ctx, &pkctx, md, app_get0_libctx(),
                                 app_get0_propq(), pkey, NULL)
        && do_pkey_ctx_init(pkctx, sigopts);
}

static int adapt_keyid_ext(X509 *cert, X509V3_CTX *ext_ctx,
                           const char *name, const char *value, int add_default)
{
    const STACK_OF(X509_EXTENSION) *exts = X509_get0_extensions(cert);
    X509_EXTENSION *new_ext = X509V3_EXT_nconf(NULL, ext_ctx, name, value);
    int idx, rv = 0;

    if (new_ext == NULL)
        return rv;

    idx = X509v3_get_ext_by_OBJ(exts, X509_EXTENSION_get_object(new_ext), -1);
    if (idx >= 0) {
        X509_EXTENSION *found_ext = X509v3_get_ext(exts, idx);
        ASN1_OCTET_STRING *data = X509_EXTENSION_get_data(found_ext);
        int disabled = ASN1_STRING_length(data) <= 2; /* config said "none" */

        if (disabled) {
            X509_delete_ext(cert, idx);
            X509_EXTENSION_free(found_ext);
        } /* else keep existing key identifier, which might be outdated */
        rv = 1;
    } else  {
        rv = !add_default || X509_add_ext(cert, new_ext, -1);
    }
    X509_EXTENSION_free(new_ext);
    return rv;
}

/* Ensure RFC 5280 compliance, adapt keyIDs as needed, and sign the cert info */
int do_X509_sign(X509 *cert, EVP_PKEY *pkey, const char *md,
                 STACK_OF(OPENSSL_STRING) *sigopts, X509V3_CTX *ext_ctx)
{
    const STACK_OF(X509_EXTENSION) *exts = X509_get0_extensions(cert);
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();
    int self_sign;
    int rv = 0;

    if (sk_X509_EXTENSION_num(exts /* may be NULL */) > 0) {
        /* Prevent X509_V_ERR_EXTENSIONS_REQUIRE_VERSION_3 */
        if (!X509_set_version(cert, X509_VERSION_3))
            goto end;

        /*
         * Add default SKID before such that default AKID can make use of it
         * in case the certificate is self-signed
         */
        /* Prevent X509_V_ERR_MISSING_SUBJECT_KEY_IDENTIFIER */
        if (!adapt_keyid_ext(cert, ext_ctx, "subjectKeyIdentifier", "hash", 1))
            goto end;
        /* Prevent X509_V_ERR_MISSING_AUTHORITY_KEY_IDENTIFIER */
        ERR_set_mark();
        self_sign = X509_check_private_key(cert, pkey);
        ERR_pop_to_mark();
        if (!adapt_keyid_ext(cert, ext_ctx, "authorityKeyIdentifier",
                             "keyid, issuer", !self_sign))
            goto end;
    }

    if (mctx != NULL && do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_sign_ctx(cert, mctx) > 0);
 end:
    EVP_MD_CTX_free(mctx);
    return rv;
}

/* Sign the certificate request info */
int do_X509_REQ_sign(X509_REQ *x, EVP_PKEY *pkey, const char *md,
                     STACK_OF(OPENSSL_STRING) *sigopts)
{
    int rv = 0;
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();

    if (do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_REQ_sign_ctx(x, mctx) > 0);
    EVP_MD_CTX_free(mctx);
    return rv;
}

/* Sign the CRL info */
int do_X509_CRL_sign(X509_CRL *x, EVP_PKEY *pkey, const char *md,
                     STACK_OF(OPENSSL_STRING) *sigopts)
{
    int rv = 0;
    EVP_MD_CTX *mctx = EVP_MD_CTX_new();

    if (do_sign_init(mctx, pkey, md, sigopts) > 0)
        rv = (X509_CRL_sign_ctx(x, mctx) > 0);
    EVP_MD_CTX_free(mctx);
    return rv;
}

/*
 * do_X509_verify returns 1 if the signature is valid,
 * 0 if the signature check fails, or -1 if error occurs.
 */
int do_X509_verify(X509 *x, EVP_PKEY *pkey, STACK_OF(OPENSSL_STRING) *vfyopts)
{
    int rv = 0;

    if (do_x509_init(x, vfyopts) > 0)
        rv = X509_verify(x, pkey);
    else
        rv = -1;
    return rv;
}

/*
 * do_X509_REQ_verify returns 1 if the signature is valid,
 * 0 if the signature check fails, or -1 if error occurs.
 */
int do_X509_REQ_verify(X509_REQ *x, EVP_PKEY *pkey,
                       STACK_OF(OPENSSL_STRING) *vfyopts)
{
    int rv = 0;

    if (do_x509_req_init(x, vfyopts) > 0)
        rv = X509_REQ_verify_ex(x, pkey,
                                 app_get0_libctx(), app_get0_propq());
    else
        rv = -1;
    return rv;
}

/* Get first http URL from a DIST_POINT structure */

static const char *get_dp_url(DIST_POINT *dp)
{
    GENERAL_NAMES *gens;
    GENERAL_NAME *gen;
    int i, gtype;
    ASN1_STRING *uri;
    if (!dp->distpoint || dp->distpoint->type != 0)
        return NULL;
    gens = dp->distpoint->name.fullname;
    for (i = 0; i < sk_GENERAL_NAME_num(gens); i++) {
        gen = sk_GENERAL_NAME_value(gens, i);
        uri = GENERAL_NAME_get0_value(gen, &gtype);
        if (gtype == GEN_URI && ASN1_STRING_length(uri) > 6) {
            const char *uptr = (const char *)ASN1_STRING_get0_data(uri);

            if (IS_HTTP(uptr)) /* can/should not use HTTPS here */
                return uptr;
        }
    }
    return NULL;
}

/*
 * Look through a CRLDP structure and attempt to find an http URL to
 * downloads a CRL from.
 */

static X509_CRL *load_crl_crldp(STACK_OF(DIST_POINT) *crldp)
{
    int i;
    const char *urlptr = NULL;
    for (i = 0; i < sk_DIST_POINT_num(crldp); i++) {
        DIST_POINT *dp = sk_DIST_POINT_value(crldp, i);
        urlptr = get_dp_url(dp);
        if (urlptr != NULL)
            return load_crl(urlptr, FORMAT_UNDEF, 0, "CRL via CDP");
    }
    return NULL;
}

/*
 * Example of downloading CRLs from CRLDP:
 * not usable for real world as it always downloads and doesn't cache anything.
 */

static STACK_OF(X509_CRL) *crls_http_cb(const X509_STORE_CTX *ctx,
                                        const X509_NAME *nm)
{
    X509 *x;
    STACK_OF(X509_CRL) *crls = NULL;
    X509_CRL *crl;
    STACK_OF(DIST_POINT) *crldp;

    crls = sk_X509_CRL_new_null();
    if (!crls)
        return NULL;
    x = X509_STORE_CTX_get_current_cert(ctx);
    crldp = X509_get_ext_d2i(x, NID_crl_distribution_points, NULL, NULL);
    crl = load_crl_crldp(crldp);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    if (!crl) {
        sk_X509_CRL_free(crls);
        return NULL;
    }
    sk_X509_CRL_push(crls, crl);
    /* Try to download delta CRL */
    crldp = X509_get_ext_d2i(x, NID_freshest_crl, NULL, NULL);
    crl = load_crl_crldp(crldp);
    sk_DIST_POINT_pop_free(crldp, DIST_POINT_free);
    if (crl)
        sk_X509_CRL_push(crls, crl);
    return crls;
}

void store_setup_crl_download(X509_STORE *st)
{
    X509_STORE_set_lookup_crls_cb(st, crls_http_cb);
}

#ifndef OPENSSL_NO_SOCK
static const char *tls_error_hint(void)
{
    unsigned long err = ERR_peek_error();

    if (ERR_GET_LIB(err) != ERR_LIB_SSL)
        err = ERR_peek_last_error();
    if (ERR_GET_LIB(err) != ERR_LIB_SSL)
        return NULL;

    switch (ERR_GET_REASON(err)) {
    case SSL_R_WRONG_VERSION_NUMBER:
        return "The server does not support (a suitable version of) TLS";
    case SSL_R_UNKNOWN_PROTOCOL:
        return "The server does not support HTTPS";
    case SSL_R_CERTIFICATE_VERIFY_FAILED:
        return "Cannot authenticate server via its TLS certificate, likely due to mismatch with our trusted TLS certs or missing revocation status";
    case SSL_AD_REASON_OFFSET + TLS1_AD_UNKNOWN_CA:
        return "Server did not accept our TLS certificate, likely due to mismatch with server's trust anchor or missing revocation status";
    case SSL_AD_REASON_OFFSET + SSL3_AD_HANDSHAKE_FAILURE:
        return "TLS handshake failure. Possibly the server requires our TLS certificate but did not receive it";
    default: /* no error or no hint available for error */
        return NULL;
    }
}

/* HTTP callback function that supports TLS connection also via HTTPS proxy */
BIO *app_http_tls_cb(BIO *bio, void *arg, int connect, int detail)
{
    APP_HTTP_TLS_INFO *info = (APP_HTTP_TLS_INFO *)arg;
    SSL_CTX *ssl_ctx = info->ssl_ctx;

    if (ssl_ctx == NULL) /* not using TLS */
        return bio;
    if (connect) {
        SSL *ssl;
        BIO *sbio = NULL;

        /* adapt after fixing callback design flaw, see #17088 */
        if ((info->use_proxy
             && !OSSL_HTTP_proxy_connect(bio, info->server, info->port,
                                         NULL, NULL, /* no proxy credentials */
                                         info->timeout, bio_err, opt_getprog()))
                || (sbio = BIO_new(BIO_f_ssl())) == NULL) {
            return NULL;
        }
        if (ssl_ctx == NULL || (ssl = SSL_new(ssl_ctx)) == NULL) {
            BIO_free(sbio);
            return NULL;
        }

        /* adapt after fixing callback design flaw, see #17088 */
        SSL_set_tlsext_host_name(ssl, info->server); /* not critical to do */

        SSL_set_connect_state(ssl);
        BIO_set_ssl(sbio, ssl, BIO_CLOSE);

        bio = BIO_push(sbio, bio);
    }
    if (!connect) {
        const char *hint;
        BIO *cbio;

        if (!detail) { /* disconnecting after error */
            hint = tls_error_hint();
            if (hint != NULL)
                ERR_add_error_data(2, " : ", hint);
        }
        if (ssl_ctx != NULL) {
            (void)ERR_set_mark();
            BIO_ssl_shutdown(bio);
            cbio = BIO_pop(bio); /* connect+HTTP BIO */
            BIO_free(bio); /* SSL BIO */
            (void)ERR_pop_to_mark(); /* hide SSL_R_READ_BIO_NOT_SET etc. */
            bio = cbio;
        }
    }
    return bio;
}

void APP_HTTP_TLS_INFO_free(APP_HTTP_TLS_INFO *info)
{
    if (info != NULL) {
        SSL_CTX_free(info->ssl_ctx);
        OPENSSL_free(info);
    }
}

ASN1_VALUE *app_http_get_asn1(const char *url, const char *proxy,
                              const char *no_proxy, SSL_CTX *ssl_ctx,
                              const STACK_OF(CONF_VALUE) *headers,
                              long timeout, const char *expected_content_type,
                              const ASN1_ITEM *it)
{
    APP_HTTP_TLS_INFO info;
    char *server;
    char *port;
    int use_ssl;
    BIO *mem;
    ASN1_VALUE *resp = NULL;

    if (url == NULL || it == NULL) {
        ERR_raise(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER);
        return NULL;
    }

    if (!OSSL_HTTP_parse_url(url, &use_ssl, NULL /* userinfo */, &server, &port,
                             NULL /* port_num, */, NULL, NULL, NULL))
        return NULL;
    if (use_ssl && ssl_ctx == NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER,
                       "missing SSL_CTX");
        goto end;
    }
    if (!use_ssl && ssl_ctx != NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_INVALID_ARGUMENT,
                       "SSL_CTX given but use_ssl == 0");
        goto end;
    }

    info.server = server;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, server, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    mem = OSSL_HTTP_get(url, proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                        app_http_tls_cb, &info, 0 /* buf_size */, headers,
                        expected_content_type, 1 /* expect_asn1 */,
                        OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout);
    resp = ASN1_item_d2i_bio(it, mem, NULL);
    BIO_free(mem);

 end:
    OPENSSL_free(server);
    OPENSSL_free(port);
    return resp;

}

ASN1_VALUE *app_http_post_asn1(const char *host, const char *port,
                               const char *path, const char *proxy,
                               const char *no_proxy, SSL_CTX *ssl_ctx,
                               const STACK_OF(CONF_VALUE) *headers,
                               const char *content_type,
                               ASN1_VALUE *req, const ASN1_ITEM *req_it,
                               const char *expected_content_type,
                               long timeout, const ASN1_ITEM *rsp_it)
{
    int use_ssl = ssl_ctx != NULL;
    APP_HTTP_TLS_INFO info;
    BIO *rsp, *req_mem = ASN1_item_i2d_mem_bio(req_it, req);
    ASN1_VALUE *res;

    if (req_mem == NULL)
        return NULL;

    info.server = host;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, host, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    rsp = OSSL_HTTP_transfer(NULL, host, port, path, use_ssl,
                             proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                             app_http_tls_cb, &info,
                             0 /* buf_size */, headers, content_type, req_mem,
                             expected_content_type, 1 /* expect_asn1 */,
                             OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout,
                             0 /* keep_alive */);
    BIO_free(req_mem);
    res = ASN1_item_d2i_bio(rsp_it, rsp, NULL);
    BIO_free(rsp);
    return res;
}

#endif

/*
 * Platform-specific sections
 */
#if defined(_WIN32)
# ifdef fileno
#  undef fileno
#  define fileno(a) (int)_fileno(a)
# endif

# include <windows.h>
# include <tchar.h>

static int WIN32_rename(const char *from, const char *to)
{
    TCHAR *tfrom = NULL, *tto;
    DWORD err;
    int ret = 0;

    if (sizeof(TCHAR) == 1) {
        tfrom = (TCHAR *)from;
        tto = (TCHAR *)to;
    } else {                    /* UNICODE path */

        size_t i, flen = strlen(from) + 1, tlen = strlen(to) + 1;
        tfrom = malloc(sizeof(*tfrom) * (flen + tlen));
        if (tfrom == NULL)
            goto err;
        tto = tfrom + flen;
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, from, flen, (WCHAR *)tfrom, flen))
# endif
            for (i = 0; i < flen; i++)
                tfrom[i] = (TCHAR)from[i];
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, to, tlen, (WCHAR *)tto, tlen))
# endif
            for (i = 0; i < tlen; i++)
                tto[i] = (TCHAR)to[i];
    }

    if (MoveFile(tfrom, tto))
        goto ok;
    err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS || err == ERROR_FILE_EXISTS) {
        if (DeleteFile(tto) && MoveFile(tfrom, tto))
            goto ok;
        err = GetLastError();
    }
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
        errno = ENOENT;
    else if (err == ERROR_ACCESS_DENIED)
        errno = EACCES;
    else
        errno = EINVAL;         /* we could map more codes... */
 err:
    ret = -1;
 ok:
    if (tfrom != NULL && tfrom != (TCHAR *)from)
        free(tfrom);
    return ret;
}
#endif

/* app_tminterval section */
#if defined(_WIN32)
double app_tminterval(int stop, int usertime)
{
    FILETIME now;
    double ret = 0;
    static ULARGE_INTEGER tmstart;
    static int warning = 1;
# ifdef _WIN32_WINNT
    static HANDLE proc = NULL;

    if (proc == NULL) {
        if (check_winnt())
            proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                               GetCurrentProcessId());
        if (proc == NULL)
            proc = (HANDLE) - 1;
    }

    if (usertime && proc != (HANDLE) - 1) {
        FILETIME junk;
        GetProcessTimes(proc, &junk, &junk, &junk, &now);
    } else
# endif
    {
        SYSTEMTIME systime;

        if (usertime && warning) {
            BIO_printf(bio_err, "To get meaningful results, run "
                       "this program on idle system.\n");
            warning = 0;
        }
        GetSystemTime(&systime);
        SystemTimeToFileTime(&systime, &now);
    }

    if (stop == TM_START) {
        tmstart.u.LowPart = now.dwLowDateTime;
        tmstart.u.HighPart = now.dwHighDateTime;
    } else {
        ULARGE_INTEGER tmstop;

        tmstop.u.LowPart = now.dwLowDateTime;
        tmstop.u.HighPart = now.dwHighDateTime;

        ret = (__int64)(tmstop.QuadPart - tmstart.QuadPart) * 1e-7;
    }

    return ret;
}
#elif defined(OPENSSL_SYS_VXWORKS)
# include <time.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
# ifdef CLOCK_REALTIME
    static struct timespec tmstart;
    struct timespec now;
# else
    static unsigned long tmstart;
    unsigned long now;
# endif
    static int warning = 1;

    if (usertime && warning) {
        BIO_printf(bio_err, "To get meaningful results, run "
                   "this program on idle system.\n");
        warning = 0;
    }
# ifdef CLOCK_REALTIME
    clock_gettime(CLOCK_REALTIME, &now);
    if (stop == TM_START)
        tmstart = now;
    else
        ret = ((now.tv_sec + now.tv_nsec * 1e-9)
               - (tmstart.tv_sec + tmstart.tv_nsec * 1e-9));
# else
    now = tickGet();
    if (stop == TM_START)
        tmstart = now;
    else
        ret = (now - tmstart) / (double)sysClkRateGet();
# endif
    return ret;
}

#elif defined(_SC_CLK_TCK)      /* by means of unistd.h */
# include <sys/times.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
    struct tms rus;
    clock_t now = times(&rus);
    static clock_t tmstart;

    if (usertime)
        now = rus.tms_utime;

    if (stop == TM_START) {
        tmstart = now;
    } else {
        long int tck = sysconf(_SC_CLK_TCK);
        ret = (now - tmstart) / (double)tck;
    }

    return ret;
}

#else
# include <sys/time.h>
# include <sys/resource.h>

double app_tminterval(int stop, int usertime)
{
    double ret = 0;
    struct rusage rus;
    struct timeval now;
    static struct timeval tmstart;

    if (usertime)
        getrusage(RUSAGE_SELF, &rus), now = rus.ru_utime;
    else
        gettimeofday(&now, NULL);

    if (stop == TM_START)
        tmstart = now;
    else
        ret = ((now.tv_sec + now.tv_usec * 1e-6)
               - (tmstart.tv_sec + tmstart.tv_usec * 1e-6));

    return ret;
}
#endif

int app_access(const char* name, int flag)
{
#ifdef _WIN32
    return _access(name, flag);
#else
    return access(name, flag);
#endif
}

int app_isdir(const char *name)
{
    return opt_isdir(name);
}

/* raw_read|write section */
#if defined(__VMS)
# include "vms_term_sock.h"
static int stdin_sock = -1;

static void close_stdin_sock(void)
{
    TerminalSocket (TERM_SOCK_DELETE, &stdin_sock);
}

int fileno_stdin(void)
{
    if (stdin_sock == -1) {
        TerminalSocket(TERM_SOCK_CREATE, &stdin_sock);
        atexit(close_stdin_sock);
    }

    return stdin_sock;
}
#else
int fileno_stdin(void)
{
    return fileno(stdin);
}
#endif

int fileno_stdout(void)
{
    return fileno(stdout);
}

#if defined(_WIN32) && defined(STD_INPUT_HANDLE)
int raw_read_stdin(void *buf, int siz)
{
    DWORD n;
    if (ReadFile(GetStdHandle(STD_INPUT_HANDLE), buf, siz, &n, NULL))
        return n;
    else
        return -1;
}
#elif defined(__VMS)
# include <sys/socket.h>

int raw_read_stdin(void *buf, int siz)
{
    return recv(fileno_stdin(), buf, siz, 0);
}
#else
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_read)>
#  endif
# endif
int raw_read_stdin(void *buf, int siz)
{
    return read(fileno_stdin(), buf, siz);
}
#endif

#if defined(_WIN32) && defined(STD_OUTPUT_HANDLE)
int raw_write_stdout(const void *buf, int siz)
{
    DWORD n;
    if (WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, siz, &n, NULL))
        return n;
    else
        return -1;
}
#elif defined(OPENSSL_SYS_TANDEM) && defined(OPENSSL_THREADS) && defined(_SPT_MODEL_)
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_write)>
#  endif
# endif
int raw_write_stdout(const void *buf,int siz)
{
	return write(fileno(stdout),(void*)buf,siz);
}
#else
# if defined(__TANDEM)
#  if defined(OPENSSL_TANDEM_FLOSS)
#   include <floss.h(floss_write)>
#  endif
# endif
int raw_write_stdout(const void *buf, int siz)
{
    return write(fileno_stdout(), buf, siz);
}
#endif

/*
 * Centralized handling of input and output files with format specification
 * The format is meant to show what the input and output is supposed to be,
 * and is therefore a show of intent more than anything else.  However, it
 * does impact behavior on some platforms, such as differentiating between
 * text and binary input/output on non-Unix platforms
 */
BIO *dup_bio_in(int format)
{
    return BIO_new_fp(stdin,
                      BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
}

BIO *dup_bio_out(int format)
{
    BIO *b = BIO_new_fp(stdout,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
    void *prefix = NULL;

    if (b == NULL)
        return NULL;

#ifdef OPENSSL_SYS_VMS
    if (FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif

    if (FMT_istext(format)
        && (prefix = getenv("HARNESS_OSSL_PREFIX")) != NULL) {
        b = BIO_push(BIO_new(BIO_f_prefix()), b);
        BIO_set_prefix(b, prefix);
    }

    return b;
}

BIO *dup_bio_err(int format)
{
    BIO *b = BIO_new_fp(stderr,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
#ifdef OPENSSL_SYS_VMS
    if (b != NULL && FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif
    return b;
}

void unbuffer(FILE *fp)
{
/*
 * On VMS, setbuf() will only take 32-bit pointers, and a compilation
 * with /POINTER_SIZE=64 will give off a MAYLOSEDATA2 warning here.
 * However, we trust that the C RTL will never give us a FILE pointer
 * above the first 4 GB of memory, so we simply turn off the warning
 * temporarily.
 */
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment save
# pragma message disable maylosedata2
#endif
    setbuf(fp, NULL);
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment restore
#endif
}

static const char *modestr(char mode, int format)
{
    OPENSSL_assert(mode == 'a' || mode == 'r' || mode == 'w');

    switch (mode) {
    case 'a':
        return FMT_istext(format) ? "a" : "ab";
    case 'r':
        return FMT_istext(format) ? "r" : "rb";
    case 'w':
        return FMT_istext(format) ? "w" : "wb";
    }
    /* The assert above should make sure we never reach this point */
    return NULL;
}

static const char *modeverb(char mode)
{
    switch (mode) {
    case 'a':
        return "appending";
    case 'r':
        return "reading";
    case 'w':
        return "writing";
    }
    return "(doing something)";
}

/*
 * Open a file for writing, owner-read-only.
 */
BIO *bio_open_owner(const char *filename, int format, int private)
{
    FILE *fp = NULL;
    BIO *b = NULL;
    int textmode, bflags;
#ifndef OPENSSL_NO_POSIX_IO
    int fd = -1, mode;
#endif

    if (!private || filename == NULL || strcmp(filename, "-") == 0)
        return bio_open_default(filename, 'w', format);

    textmode = FMT_istext(format);
#ifndef OPENSSL_NO_POSIX_IO
    mode = O_WRONLY;
# ifdef O_CREAT
    mode |= O_CREAT;
# endif
# ifdef O_TRUNC
    mode |= O_TRUNC;
# endif
    if (!textmode) {
# ifdef O_BINARY
        mode |= O_BINARY;
# elif defined(_O_BINARY)
        mode |= _O_BINARY;
# endif
    }

# ifdef OPENSSL_SYS_VMS
    /* VMS doesn't have O_BINARY, it just doesn't make sense.  But,
     * it still needs to know that we're going binary, or fdopen()
     * will fail with "invalid argument"...  so we tell VMS what the
     * context is.
     */
    if (!textmode)
        fd = open(filename, mode, 0600, "ctx=bin");
    else
# endif
        fd = open(filename, mode, 0600);
    if (fd < 0)
        goto err;
    fp = fdopen(fd, modestr('w', format));
#else   /* OPENSSL_NO_POSIX_IO */
    /* Have stdio but not Posix IO, do the best we can */
    fp = fopen(filename, modestr('w', format));
#endif  /* OPENSSL_NO_POSIX_IO */
    if (fp == NULL)
        goto err;
    bflags = BIO_CLOSE;
    if (textmode)
        bflags |= BIO_FP_TEXT;
    b = BIO_new_fp(fp, bflags);
    if (b != NULL)
        return b;

 err:
    BIO_printf(bio_err, "%s: Can't open \"%s\" for writing, %s\n",
               opt_getprog(), filename, strerror(errno));
    ERR_print_errors(bio_err);
    /* If we have fp, then fdopen took over fd, so don't close both. */
    if (fp != NULL)
        fclose(fp);
#ifndef OPENSSL_NO_POSIX_IO
    else if (fd >= 0)
        close(fd);
#endif
    return NULL;
}

static BIO *bio_open_default_(const char *filename, char mode, int format,
                              int quiet)
{
    BIO *ret;

    if (filename == NULL || strcmp(filename, "-") == 0) {
        ret = mode == 'r' ? dup_bio_in(format) : dup_bio_out(format);
        if (quiet) {
            ERR_clear_error();
            return ret;
        }
        if (ret != NULL)
            return ret;
        BIO_printf(bio_err,
                   "Can't open %s, %s\n",
                   mode == 'r' ? "stdin" : "stdout", strerror(errno));
    } else {
        ret = BIO_new_file(filename, modestr(mode, format));
        if (quiet) {
            ERR_clear_error();
            return ret;
        }
        if (ret != NULL)
            return ret;
        BIO_printf(bio_err,
                   "Can't open \"%s\" for %s, %s\n",
                   filename, modeverb(mode), strerror(errno));
    }
    ERR_print_errors(bio_err);
    return NULL;
}

BIO *bio_open_default(const char *filename, char mode, int format)
{
    return bio_open_default_(filename, mode, format, 0);
}

BIO *bio_open_default_quiet(const char *filename, char mode, int format)
{
    return bio_open_default_(filename, mode, format, 1);
}

void wait_for_async(SSL *s)
{
    /* On Windows select only works for sockets, so we simply don't wait  */
#ifndef OPENSSL_SYS_WINDOWS
    int width = 0;
    fd_set asyncfds;
    OSSL_ASYNC_FD *fds;
    size_t numfds;
    size_t i;

    if (!SSL_get_all_async_fds(s, NULL, &numfds))
        return;
    if (numfds == 0)
        return;
    fds = app_malloc(sizeof(OSSL_ASYNC_FD) * numfds, "allocate async fds");
    if (!SSL_get_all_async_fds(s, fds, &numfds)) {
        OPENSSL_free(fds);
        return;
    }

    FD_ZERO(&asyncfds);
    for (i = 0; i < numfds; i++) {
        if (width <= (int)fds[i])
            width = (int)fds[i] + 1;
        openssl_fdset((int)fds[i], &asyncfds);
    }
    select(width, (void *)&asyncfds, NULL, NULL, NULL);
    OPENSSL_free(fds);
#endif
}

/* if OPENSSL_SYS_WINDOWS is defined then so is OPENSSL_SYS_MSDOS */
#if defined(OPENSSL_SYS_MSDOS)
int has_stdin_waiting(void)
{
# if defined(OPENSSL_SYS_WINDOWS)
    HANDLE inhand = GetStdHandle(STD_INPUT_HANDLE);
    DWORD events = 0;
    INPUT_RECORD inputrec;
    DWORD insize = 1;
    BOOL peeked;

    if (inhand == INVALID_HANDLE_VALUE) {
        return 0;
    }

    peeked = PeekConsoleInput(inhand, &inputrec, insize, &events);
    if (!peeked) {
        /* Probably redirected input? _kbhit() does not work in this case */
        if (!feof(stdin)) {
            return 1;
        }
        return 0;
    }
# endif
    return _kbhit();
}
#endif

/* Corrupt a signature by modifying final byte */
void corrupt_signature(const ASN1_STRING *signature)
{
        unsigned char *s = signature->data;
        s[signature->length - 1] ^= 0x1;
}

int set_cert_times(X509 *x, const char *startdate, const char *enddate,
                   int days)
{
    if (startdate == NULL || strcmp(startdate, "today") == 0) {
        if (X509_gmtime_adj(X509_getm_notBefore(x), 0) == NULL)
            return 0;
    } else {
        if (!ASN1_TIME_set_string_X509(X509_getm_notBefore(x), startdate))
            return 0;
    }
    if (enddate == NULL) {
        if (X509_time_adj_ex(X509_getm_notAfter(x), days, 0, NULL)
            == NULL)
            return 0;
    } else if (!ASN1_TIME_set_string_X509(X509_getm_notAfter(x), enddate)) {
        return 0;
    }
    return 1;
}

int set_crl_lastupdate(X509_CRL *crl, const char *lastupdate)
{
    int ret = 0;
    ASN1_TIME *tm = ASN1_TIME_new();

    if (tm == NULL)
        goto end;

    if (lastupdate == NULL) {
        if (X509_gmtime_adj(tm, 0) == NULL)
            goto end;
    } else {
        if (!ASN1_TIME_set_string_X509(tm, lastupdate))
            goto end;
    }

    if (!X509_CRL_set1_lastUpdate(crl, tm))
        goto end;

    ret = 1;
end:
    ASN1_TIME_free(tm);
    return ret;
}

int set_crl_nextupdate(X509_CRL *crl, const char *nextupdate,
                       long days, long hours, long secs)
{
    int ret = 0;
    ASN1_TIME *tm = ASN1_TIME_new();

    if (tm == NULL)
        goto end;

    if (nextupdate == NULL) {
        if (X509_time_adj_ex(tm, days, hours * 60 * 60 + secs, NULL) == NULL)
            goto end;
    } else {
        if (!ASN1_TIME_set_string_X509(tm, nextupdate))
            goto end;
    }

    if (!X509_CRL_set1_nextUpdate(crl, tm))
        goto end;

    ret = 1;
end:
    ASN1_TIME_free(tm);
    return ret;
}

void make_uppercase(char *string)
{
    int i;

    for (i = 0; string[i] != '\0'; i++)
        string[i] = toupper((unsigned char)string[i]);
}

/* This function is defined here due to visibility of bio_err */
int opt_printf_stderr(const char *fmt, ...)
{
    va_list ap;
    int ret;

    va_start(ap, fmt);
    ret = BIO_vprintf(bio_err, fmt, ap);
    va_end(ap);
    return ret;
}

OSSL_PARAM *app_params_new_from_opts(STACK_OF(OPENSSL_STRING) *opts,
                                     const OSSL_PARAM *paramdefs)
{
    OSSL_PARAM *params = NULL;
    size_t sz = (size_t)sk_OPENSSL_STRING_num(opts);
    size_t params_n;
    char *opt = "", *stmp, *vtmp = NULL;
    int found = 1;

    if (opts == NULL)
        return NULL;

    params = OPENSSL_zalloc(sizeof(OSSL_PARAM) * (sz + 1));
    if (params == NULL)
        return NULL;

    for (params_n = 0; params_n < sz; params_n++) {
        opt = sk_OPENSSL_STRING_value(opts, (int)params_n);
        if ((stmp = OPENSSL_strdup(opt)) == NULL
            || (vtmp = strchr(stmp, ':')) == NULL)
            goto err;
        /* Replace ':' with 0 to terminate the string pointed to by stmp */
        *vtmp = 0;
        /* Skip over the separator so that vmtp points to the value */
        vtmp++;
        if (!OSSL_PARAM_allocate_from_text(&params[params_n], paramdefs,
                                           stmp, vtmp, strlen(vtmp), &found))
            goto err;
        OPENSSL_free(stmp);
    }
    params[params_n] = OSSL_PARAM_construct_end();
    return params;
err:
    OPENSSL_free(stmp);
    BIO_printf(bio_err, "Parameter %s '%s'\n", found ? "error" : "unknown",
               opt);
    ERR_print_errors(bio_err);
    app_params_free(params);
    return NULL;
}

void app_params_free(OSSL_PARAM *params)
{
    int i;

    if (params != NULL) {
        for (i = 0; params[i].key != NULL; ++i)
            OPENSSL_free(params[i].data);
        OPENSSL_free(params);
    }
}

EVP_PKEY *app_keygen(EVP_PKEY_CTX *ctx, const char *alg, int bits, int verbose)
{
    EVP_PKEY *res = NULL;

    if (verbose && alg != NULL) {
        BIO_printf(bio_err, "Generating %s key", alg);
        if (bits > 0)
            BIO_printf(bio_err, " with %d bits\n", bits);
        else
            BIO_printf(bio_err, "\n");
    }
    if (!RAND_status())
        BIO_printf(bio_err, "Warning: generating random key material may take a long time\n"
                   "if the system has a poor entropy source\n");
    if (EVP_PKEY_keygen(ctx, &res) <= 0)
        app_bail_out("%s: Error generating %s key\n", opt_getprog(),
                     alg != NULL ? alg : "asymmetric");
    return res;
}

EVP_PKEY *app_paramgen(EVP_PKEY_CTX *ctx, const char *alg)
{
    EVP_PKEY *res = NULL;

    if (!RAND_status())
        BIO_printf(bio_err, "Warning: generating random key parameters may take a long time\n"
                   "if the system has a poor entropy source\n");
    if (EVP_PKEY_paramgen(ctx, &res) <= 0)
        app_bail_out("%s: Generating %s key parameters failed\n",
                     opt_getprog(), alg != NULL ? alg : "asymmetric");
    return res;
}

/*
 * Return non-zero if the legacy path is still an option.
 * This decision is based on the global command line operations and the
 * behaviour thus far.
 */
int opt_legacy_okay(void)
{
    int provider_options = opt_provider_option_given();
    int libctx = app_get0_libctx() != NULL || app_get0_propq() != NULL;
#ifndef OPENSSL_NO_ENGINE
    ENGINE *e = ENGINE_get_first();

    if (e != NULL) {
        ENGINE_free(e);
        return 1;
    }
#endif
    /*
     * Having a provider option specified or a custom library context or
     * property query, is a sure sign we're not using legacy.
     */
    if (provider_options || libctx)
        return 0;
    return 1;
}
        if (ret == NULL) {
            BIO_printf(bio_err, "Error converting number from bin to BIGNUM\n");
            goto err;
        }
    }

    if (ret != NULL && retai != NULL) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (ret == NULL)
        ERR_print_errors(bio_err);
    BIO_free(in);
    ASN1_INTEGER_free(ai);
    return ret;
}

int save_serial(const char *serialfile, const char *suffix, const BIGNUM *serial,
                ASN1_INTEGER **retai)
{
    char buf[1][BSIZE];
    BIO *out = NULL;
    int ret = 0;
    ASN1_INTEGER *ai = NULL;
    int j;

    if (suffix == NULL)
        j = strlen(serialfile);
    else
        j = strlen(serialfile) + strlen(suffix) + 1;
    if (j >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }

    if (suffix == NULL)
        OPENSSL_strlcpy(buf[0], serialfile, BSIZE);
    else {
#ifndef OPENSSL_SYS_VMS
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, suffix);
#else
        j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, suffix);
#endif
    }
    out = BIO_new_file(buf[0], "w");
    if (out == NULL) {
        goto err;
    }

    if ((ai = BN_to_ASN1_INTEGER(serial, NULL)) == NULL) {
        BIO_printf(bio_err, "error converting serial to ASN.1 format\n");
        goto err;
    }
    i2a_ASN1_INTEGER(out, ai);
    BIO_puts(out, "\n");
    ret = 1;
    if (retai) {
        *retai = ai;
        ai = NULL;
    }
 err:
    if (!ret)
        ERR_print_errors(bio_err);
    BIO_free_all(out);
    ASN1_INTEGER_free(ai);
    return ret;
}

int rotate_serial(const char *serialfile, const char *new_suffix,
                  const char *old_suffix)
{
    char buf[2][BSIZE];
    int i, j;

    i = strlen(serialfile) + strlen(old_suffix);
    j = strlen(serialfile) + strlen(new_suffix);
    if (i > j)
        j = i;
    if (j + 1 >= BSIZE) {
        BIO_printf(bio_err, "File name too long\n");
        goto err;
    }
#ifndef OPENSSL_SYS_VMS
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s.%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s.%s", serialfile, old_suffix);
#else
    j = BIO_snprintf(buf[0], sizeof(buf[0]), "%s-%s", serialfile, new_suffix);
    j = BIO_snprintf(buf[1], sizeof(buf[1]), "%s-%s", serialfile, old_suffix);
#endif
    if (rename(serialfile, buf[1]) < 0 && errno != ENOENT
#ifdef ENOTDIR
        && errno != ENOTDIR
#endif
        ) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", serialfile, buf[1]);
        perror("reason");
        goto err;
    }
    if (rename(buf[0], serialfile) < 0) {
        BIO_printf(bio_err,
                   "Unable to rename %s to %s\n", buf[0], serialfile);
        perror("reason");
        rename(buf[1], serialfile);
        goto err;
    }
    return 1;
 err:
    ERR_print_errors(bio_err);
    return 0;
}

int rand_serial(BIGNUM *b, ASN1_INTEGER *ai)
{
    BIGNUM *btmp;
    int ret = 0;

    btmp = b == NULL ? BN_new() : b;
    if (btmp == NULL)
        return 0;

    if (!BN_rand(btmp, SERIAL_RAND_BITS, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY))
        goto error;
    if (ai && !BN_to_ASN1_INTEGER(btmp, ai))
        goto error;

    ret = 1;

 error:

    if (btmp != b)
        BN_free(btmp);

    return ret;
}

CA_DB *load_index(const char *dbfile, DB_ATTR *db_attr)
{
    CA_DB *retdb = NULL;
    TXT_DB *tmpdb = NULL;
    BIO *in;
    CONF *dbattr_conf = NULL;
    char buf[BSIZE];
#ifndef OPENSSL_NO_POSIX_IO
    FILE *dbfp;
    struct stat dbst;
#endif

    in = BIO_new_file(dbfile, "r");
    if (in == NULL)
        goto err;

#ifndef OPENSSL_NO_POSIX_IO
    BIO_get_fp(in, &dbfp);
    if (fstat(fileno(dbfp), &dbst) == -1) {
        ERR_raise_data(ERR_LIB_SYS, errno,
                       "calling fstat(%s)", dbfile);
        goto err;
    }
#endif

    if ((tmpdb = TXT_DB_read(in, DB_NUMBER)) == NULL)
        goto err;

#ifndef OPENSSL_SYS_VMS
    BIO_snprintf(buf, sizeof(buf), "%s.attr", dbfile);
#else
    BIO_snprintf(buf, sizeof(buf), "%s-attr", dbfile);
#endif
    dbattr_conf = app_load_config_quiet(buf);

    retdb = app_malloc(sizeof(*retdb), "new DB");
    retdb->db = tmpdb;
    tmpdb = NULL;
    if (db_attr)
        retdb->attributes = *db_attr;
    else {
        retdb->attributes.unique_subject = 1;
    }

    if (dbattr_conf) {
        char *p = NCONF_get_string(dbattr_conf, NULL, "unique_subject");
        if (p) {
            retdb->attributes.unique_subject = parse_yesno(p, 1);
        }
{
    APP_HTTP_TLS_INFO *info = (APP_HTTP_TLS_INFO *)arg;
    SSL_CTX *ssl_ctx = info->ssl_ctx;

    if (ssl_ctx == NULL) /* not using TLS */
        return bio;
    if (connect) {
        SSL *ssl;
        BIO *sbio = NULL;

        /* adapt after fixing callback design flaw, see #17088 */
        if ((info->use_proxy
             && !OSSL_HTTP_proxy_connect(bio, info->server, info->port,
                                         NULL, NULL, /* no proxy credentials */
                                         info->timeout, bio_err, opt_getprog()))
                || (sbio = BIO_new(BIO_f_ssl())) == NULL) {
            return NULL;
        }
        if (ssl_ctx == NULL || (ssl = SSL_new(ssl_ctx)) == NULL) {
            BIO_free(sbio);
            return NULL;
        }

        /* adapt after fixing callback design flaw, see #17088 */
        SSL_set_tlsext_host_name(ssl, info->server); /* not critical to do */

        SSL_set_connect_state(ssl);
        BIO_set_ssl(sbio, ssl, BIO_CLOSE);

        bio = BIO_push(sbio, bio);
    }
    if (use_ssl && ssl_ctx == NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_NULL_PARAMETER,
                       "missing SSL_CTX");
        goto end;
    }
    if (!use_ssl && ssl_ctx != NULL) {
        ERR_raise_data(ERR_LIB_HTTP, ERR_R_PASSED_INVALID_ARGUMENT,
                       "SSL_CTX given but use_ssl == 0");
        goto end;
    }

    info.server = server;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, server, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    mem = OSSL_HTTP_get(url, proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                        app_http_tls_cb, &info, 0 /* buf_size */, headers,
                        expected_content_type, 1 /* expect_asn1 */,
                        OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout);
    resp = ASN1_item_d2i_bio(it, mem, NULL);
    BIO_free(mem);

 end:
    OPENSSL_free(server);
    OPENSSL_free(port);
    return resp;

}

ASN1_VALUE *app_http_post_asn1(const char *host, const char *port,
                               const char *path, const char *proxy,
                               const char *no_proxy, SSL_CTX *ssl_ctx,
                               const STACK_OF(CONF_VALUE) *headers,
                               const char *content_type,
                               ASN1_VALUE *req, const ASN1_ITEM *req_it,
                               const char *expected_content_type,
                               long timeout, const ASN1_ITEM *rsp_it)
{
    int use_ssl = ssl_ctx != NULL;
    APP_HTTP_TLS_INFO info;
    BIO *rsp, *req_mem = ASN1_item_i2d_mem_bio(req_it, req);
    ASN1_VALUE *res;

    if (req_mem == NULL)
        return NULL;

    info.server = host;
    info.port = port;
    info.use_proxy = /* workaround for callback design flaw, see #17088 */
        OSSL_HTTP_adapt_proxy(proxy, no_proxy, host, use_ssl) != NULL;
    info.timeout = timeout;
    info.ssl_ctx = ssl_ctx;
    rsp = OSSL_HTTP_transfer(NULL, host, port, path, use_ssl,
                             proxy, no_proxy, NULL /* bio */, NULL /* rbio */,
                             app_http_tls_cb, &info,
                             0 /* buf_size */, headers, content_type, req_mem,
                             expected_content_type, 1 /* expect_asn1 */,
                             OSSL_HTTP_DEFAULT_MAX_RESP_LEN, timeout,
                             0 /* keep_alive */);
    BIO_free(req_mem);
    res = ASN1_item_d2i_bio(rsp_it, rsp, NULL);
    BIO_free(rsp);
    return res;
}

#endif

/*
 * Platform-specific sections
 */
#if defined(_WIN32)
# ifdef fileno
#  undef fileno
#  define fileno(a) (int)_fileno(a)
# endif

# include <windows.h>
# include <tchar.h>

static int WIN32_rename(const char *from, const char *to)
{
    TCHAR *tfrom = NULL, *tto;
    DWORD err;
    int ret = 0;

    if (sizeof(TCHAR) == 1) {
        tfrom = (TCHAR *)from;
        tto = (TCHAR *)to;
    } else {                    /* UNICODE path */

        size_t i, flen = strlen(from) + 1, tlen = strlen(to) + 1;
        tfrom = malloc(sizeof(*tfrom) * (flen + tlen));
        if (tfrom == NULL)
            goto err;
        tto = tfrom + flen;
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, from, flen, (WCHAR *)tfrom, flen))
# endif
            for (i = 0; i < flen; i++)
                tfrom[i] = (TCHAR)from[i];
# if !defined(_WIN32_WCE) || _WIN32_WCE>=101
        if (!MultiByteToWideChar(CP_ACP, 0, to, tlen, (WCHAR *)tto, tlen))
# endif
            for (i = 0; i < tlen; i++)
                tto[i] = (TCHAR)to[i];
    }

    if (MoveFile(tfrom, tto))
        goto ok;
    err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS || err == ERROR_FILE_EXISTS) {
        if (DeleteFile(tto) && MoveFile(tfrom, tto))
            goto ok;
        err = GetLastError();
    }
    if (err == ERROR_FILE_NOT_FOUND || err == ERROR_PATH_NOT_FOUND)
        errno = ENOENT;
    else if (err == ERROR_ACCESS_DENIED)
        errno = EACCES;
    else
        errno = EINVAL;         /* we could map more codes... */
 err:
    ret = -1;
 ok:
    if (tfrom != NULL && tfrom != (TCHAR *)from)
        free(tfrom);
    return ret;
}
#endif

/* app_tminterval section */
#if defined(_WIN32)
double app_tminterval(int stop, int usertime)
{
    FILETIME now;
    double ret = 0;
    static ULARGE_INTEGER tmstart;
    static int warning = 1;
# ifdef _WIN32_WINNT
    static HANDLE proc = NULL;

    if (proc == NULL) {
        if (check_winnt())
            proc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE,
                               GetCurrentProcessId());
        if (proc == NULL)
            proc = (HANDLE) - 1;
    }

    if (usertime && proc != (HANDLE) - 1) {
        FILETIME junk;
        GetProcessTimes(proc, &junk, &junk, &junk, &now);
    } else
# endif
    {
        SYSTEMTIME systime;

        if (usertime && warning) {
            BIO_printf(bio_err, "To get meaningful results, run "
                       "this program on idle system.\n");
            warning = 0;
        }
        GetSystemTime(&systime);
        SystemTimeToFileTime(&systime, &now);
    }
{
    BIO *b = BIO_new_fp(stdout,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
    void *prefix = NULL;

    if (b == NULL)
        return NULL;

#ifdef OPENSSL_SYS_VMS
    if (FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif

    if (FMT_istext(format)
        && (prefix = getenv("HARNESS_OSSL_PREFIX")) != NULL) {
        b = BIO_push(BIO_new(BIO_f_prefix()), b);
        BIO_set_prefix(b, prefix);
    }
{
    BIO *b = BIO_new_fp(stderr,
                        BIO_NOCLOSE | (FMT_istext(format) ? BIO_FP_TEXT : 0));
#ifdef OPENSSL_SYS_VMS
    if (b != NULL && FMT_istext(format))
        b = BIO_push(BIO_new(BIO_f_linebuffer()), b);
#endif
    return b;
}

void unbuffer(FILE *fp)
{
/*
 * On VMS, setbuf() will only take 32-bit pointers, and a compilation
 * with /POINTER_SIZE=64 will give off a MAYLOSEDATA2 warning here.
 * However, we trust that the C RTL will never give us a FILE pointer
 * above the first 4 GB of memory, so we simply turn off the warning
 * temporarily.
 */
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment save
# pragma message disable maylosedata2
#endif
    setbuf(fp, NULL);
#if defined(OPENSSL_SYS_VMS) && defined(__DECC)
# pragma environment restore
#endif
}

static const char *modestr(char mode, int format)
{
    OPENSSL_assert(mode == 'a' || mode == 'r' || mode == 'w');

    switch (mode) {
    case 'a':
        return FMT_istext(format) ? "a" : "ab";
    case 'r':
        return FMT_istext(format) ? "r" : "rb";
    case 'w':
        return FMT_istext(format) ? "w" : "wb";
    }