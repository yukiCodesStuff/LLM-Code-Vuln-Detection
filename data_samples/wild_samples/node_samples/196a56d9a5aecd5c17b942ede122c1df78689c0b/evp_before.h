struct evp_cipher_st {
    int nid;

    int block_size;
    /* Default value for variable length ciphers */
    int key_len;
    int iv_len;

    /* Legacy structure members */
    /* Various flags */
    unsigned long flags;
    /* How the EVP_CIPHER was created. */
    int origin;
    /* init key */
    int (*init) (EVP_CIPHER_CTX *ctx, const unsigned char *key,
                 const unsigned char *iv, int enc);
    /* encrypt/decrypt data */
    int (*do_cipher) (EVP_CIPHER_CTX *ctx, unsigned char *out,
                      const unsigned char *in, size_t inl);
    /* cleanup ctx */
    int (*cleanup) (EVP_CIPHER_CTX *);
    /* how big ctx->cipher_data needs to be */
    int ctx_size;
    /* Populate a ASN1_TYPE with parameters */
    int (*set_asn1_parameters) (EVP_CIPHER_CTX *, ASN1_TYPE *);
    /* Get parameters from a ASN1_TYPE */
    int (*get_asn1_parameters) (EVP_CIPHER_CTX *, ASN1_TYPE *);
    /* Miscellaneous operations */
    int (*ctrl) (EVP_CIPHER_CTX *, int type, int arg, void *ptr);
    /* Application data */
    void *app_data;

    /* New structure members */
    /* Above comment to be removed when legacy has gone */
    int name_id;
    char *type_name;
    const char *description;
    OSSL_PROVIDER *prov;
    CRYPTO_REF_COUNT refcnt;
    CRYPTO_RWLOCK *lock;
    OSSL_FUNC_cipher_newctx_fn *newctx;
    OSSL_FUNC_cipher_encrypt_init_fn *einit;
    OSSL_FUNC_cipher_decrypt_init_fn *dinit;
    OSSL_FUNC_cipher_update_fn *cupdate;
    OSSL_FUNC_cipher_final_fn *cfinal;
    OSSL_FUNC_cipher_cipher_fn *ccipher;
    OSSL_FUNC_cipher_freectx_fn *freectx;
    OSSL_FUNC_cipher_dupctx_fn *dupctx;
    OSSL_FUNC_cipher_get_params_fn *get_params;
    OSSL_FUNC_cipher_get_ctx_params_fn *get_ctx_params;
    OSSL_FUNC_cipher_set_ctx_params_fn *set_ctx_params;
    OSSL_FUNC_cipher_gettable_params_fn *gettable_params;
    OSSL_FUNC_cipher_gettable_ctx_params_fn *gettable_ctx_params;
    OSSL_FUNC_cipher_settable_ctx_params_fn *settable_ctx_params;
} /* EVP_CIPHER */ ;

/* Macros to code block cipher wrappers */

/* Wrapper functions for each cipher mode */

#define EVP_C_DATA(kstruct, ctx) \
        ((kstruct *)EVP_CIPHER_CTX_get_cipher_data(ctx))

#define BLOCK_CIPHER_ecb_loop() \
        size_t i, bl; \
        bl = EVP_CIPHER_CTX_get0_cipher(ctx)->block_size;    \
        if (inl < bl) return 1;\
        inl -= bl; \
        for (i=0; i <= inl; i+=bl)

#define BLOCK_CIPHER_func_ecb(cname, cprefix, kstruct, ksched) \
static int cname##_ecb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
        BLOCK_CIPHER_ecb_loop() \
            cprefix##_ecb_encrypt(in + i, out + i, &EVP_C_DATA(kstruct,ctx)->ksched, EVP_CIPHER_CTX_is_encrypting(ctx)); \
        return 1;\
}

#define EVP_MAXCHUNK ((size_t)1<<(sizeof(long)*8-2))

#define BLOCK_CIPHER_func_ofb(cname, cprefix, cbits, kstruct, ksched) \
    static int cname##_ofb_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
        while(inl>=EVP_MAXCHUNK) {\
            int num = EVP_CIPHER_CTX_get_num(ctx);\
            cprefix##_ofb##cbits##_encrypt(in, out, (long)EVP_MAXCHUNK, &EVP_C_DATA(kstruct,ctx)->ksched, ctx->iv, &num); \
            EVP_CIPHER_CTX_set_num(ctx, num);\
            inl-=EVP_MAXCHUNK;\
            in +=EVP_MAXCHUNK;\
            out+=EVP_MAXCHUNK;\
        }\
        if (inl) {\
            int num = EVP_CIPHER_CTX_get_num(ctx);\
            cprefix##_ofb##cbits##_encrypt(in, out, (long)inl, &EVP_C_DATA(kstruct,ctx)->ksched, ctx->iv, &num); \
            EVP_CIPHER_CTX_set_num(ctx, num);\
        }\
        return 1;\
}

#define BLOCK_CIPHER_func_cbc(cname, cprefix, kstruct, ksched) \
static int cname##_cbc_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
        while(inl>=EVP_MAXCHUNK) \
            {\
            cprefix##_cbc_encrypt(in, out, (long)EVP_MAXCHUNK, &EVP_C_DATA(kstruct,ctx)->ksched, ctx->iv, EVP_CIPHER_CTX_is_encrypting(ctx));\
            inl-=EVP_MAXCHUNK;\
            in +=EVP_MAXCHUNK;\
            out+=EVP_MAXCHUNK;\
            }\
        if (inl)\
            cprefix##_cbc_encrypt(in, out, (long)inl, &EVP_C_DATA(kstruct,ctx)->ksched, ctx->iv, EVP_CIPHER_CTX_is_encrypting(ctx));\
        return 1;\
}

#define BLOCK_CIPHER_func_cfb(cname, cprefix, cbits, kstruct, ksched)  \
static int cname##_cfb##cbits##_cipher(EVP_CIPHER_CTX *ctx, unsigned char *out, const unsigned char *in, size_t inl) \
{\
    size_t chunk = EVP_MAXCHUNK;\
    if (cbits == 1)  chunk >>= 3;\
    if (inl < chunk) chunk = inl;\
    while (inl && inl >= chunk)\
    {\
        int num = EVP_CIPHER_CTX_get_num(ctx);\
        cprefix##_cfb##cbits##_encrypt(in, out, (long) \
            ((cbits == 1) \
                && !EVP_CIPHER_CTX_test_flags(ctx, EVP_CIPH_FLAG_LENGTH_BITS) \
                ? chunk*8 : chunk), \
            &EVP_C_DATA(kstruct, ctx)->ksched, ctx->iv,\
            &num, EVP_CIPHER_CTX_is_encrypting(ctx));\
        EVP_CIPHER_CTX_set_num(ctx, num);\
        inl -= chunk;\
        in += chunk;\
        out += chunk;\
        if (inl < chunk) chunk = inl;\
    }\
    return 1;\
}

#define BLOCK_CIPHER_all_funcs(cname, cprefix, cbits, kstruct, ksched) \
        BLOCK_CIPHER_func_cbc(cname, cprefix, kstruct, ksched) \
        BLOCK_CIPHER_func_cfb(cname, cprefix, cbits, kstruct, ksched) \
        BLOCK_CIPHER_func_ecb(cname, cprefix, kstruct, ksched) \
        BLOCK_CIPHER_func_ofb(cname, cprefix, cbits, kstruct, ksched)

#define BLOCK_CIPHER_def1(cname, nmode, mode, MODE, kstruct, nid, block_size, \
                          key_len, iv_len, flags, init_key, cleanup, \
                          set_asn1, get_asn1, ctrl) \
static const EVP_CIPHER cname##_##mode = { \
        nid##_##nmode, block_size, key_len, iv_len, \
        flags | EVP_CIPH_##MODE##_MODE, \
        EVP_ORIG_GLOBAL, \
        init_key, \
        cname##_##mode##_cipher, \
        cleanup, \
        sizeof(kstruct), \
        set_asn1, get_asn1,\
        ctrl, \
        NULL \
}; \
const EVP_CIPHER *EVP_##cname##_##mode(void) { return &cname##_##mode; }

#define BLOCK_CIPHER_def_cbc(cname, kstruct, nid, block_size, key_len, \
                             iv_len, flags, init_key, cleanup, set_asn1, \
                             get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, cbc, cbc, CBC, kstruct, nid, block_size, key_len, \
                  iv_len, flags, init_key, cleanup, set_asn1, get_asn1, ctrl)

#define BLOCK_CIPHER_def_cfb(cname, kstruct, nid, key_len, \
                             iv_len, cbits, flags, init_key, cleanup, \
                             set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, cfb##cbits, cfb##cbits, CFB, kstruct, nid, 1, \
                  key_len, iv_len, flags, init_key, cleanup, set_asn1, \
                  get_asn1, ctrl)

#define BLOCK_CIPHER_def_ofb(cname, kstruct, nid, key_len, \
                             iv_len, cbits, flags, init_key, cleanup, \
                             set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, ofb##cbits, ofb, OFB, kstruct, nid, 1, \
                  key_len, iv_len, flags, init_key, cleanup, set_asn1, \
                  get_asn1, ctrl)

#define BLOCK_CIPHER_def_ecb(cname, kstruct, nid, block_size, key_len, \
                             flags, init_key, cleanup, set_asn1, \
                             get_asn1, ctrl) \
BLOCK_CIPHER_def1(cname, ecb, ecb, ECB, kstruct, nid, block_size, key_len, \
                  0, flags, init_key, cleanup, set_asn1, get_asn1, ctrl)

#define BLOCK_CIPHER_defs(cname, kstruct, \
                          nid, block_size, key_len, iv_len, cbits, flags, \
                          init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_cbc(cname, kstruct, nid, block_size, key_len, iv_len, flags, \
                     init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_cfb(cname, kstruct, nid, key_len, iv_len, cbits, \
                     flags, init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_ofb(cname, kstruct, nid, key_len, iv_len, cbits, \
                     flags, init_key, cleanup, set_asn1, get_asn1, ctrl) \
BLOCK_CIPHER_def_ecb(cname, kstruct, nid, block_size, key_len, flags, \
                     init_key, cleanup, set_asn1, get_asn1, ctrl)

/*-
#define BLOCK_CIPHER_defs(cname, kstruct, \
                                nid, block_size, key_len, iv_len, flags,\
                                 init_key, cleanup, set_asn1, get_asn1, ctrl)\
static const EVP_CIPHER cname##_cbc = {\
        nid##_cbc, block_size, key_len, iv_len, \
        flags | EVP_CIPH_CBC_MODE,\
        EVP_ORIG_GLOBAL,\
        init_key,\
        cname##_cbc_cipher,\
        cleanup,\
        sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
                sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
        set_asn1, get_asn1,\
        ctrl, \
        NULL \
};\
const EVP_CIPHER *EVP_##cname##_cbc(void) { return &cname##_cbc; }\
static const EVP_CIPHER cname##_cfb = {\
        nid##_cfb64, 1, key_len, iv_len, \
        flags | EVP_CIPH_CFB_MODE,\
        EVP_ORIG_GLOBAL,\
        init_key,\
        cname##_cfb_cipher,\
        cleanup,\
        sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
                sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
        set_asn1, get_asn1,\
        ctrl,\
        NULL \
};\
const EVP_CIPHER *EVP_##cname##_cfb(void) { return &cname##_cfb; }\
static const EVP_CIPHER cname##_ofb = {\
        nid##_ofb64, 1, key_len, iv_len, \
        flags | EVP_CIPH_OFB_MODE,\
        EVP_ORIG_GLOBAL,\
        init_key,\
        cname##_ofb_cipher,\
        cleanup,\
        sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
                sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
        set_asn1, get_asn1,\
        ctrl,\
        NULL \
};\
const EVP_CIPHER *EVP_##cname##_ofb(void) { return &cname##_ofb; }\
static const EVP_CIPHER cname##_ecb = {\
        nid##_ecb, block_size, key_len, iv_len, \
        flags | EVP_CIPH_ECB_MODE,\
        EVP_ORIG_GLOBAL,\
        init_key,\
        cname##_ecb_cipher,\
        cleanup,\
        sizeof(EVP_CIPHER_CTX)-sizeof((((EVP_CIPHER_CTX *)NULL)->c))+\
                sizeof((((EVP_CIPHER_CTX *)NULL)->c.kstruct)),\
        set_asn1, get_asn1,\
        ctrl,\
        NULL \
};\
const EVP_CIPHER *EVP_##cname##_ecb(void) { return &cname##_ecb; }
*/

#define IMPLEMENT_BLOCK_CIPHER(cname, ksched, cprefix, kstruct, nid, \
                               block_size, key_len, iv_len, cbits, \
                               flags, init_key, \
                               cleanup, set_asn1, get_asn1, ctrl) \
        BLOCK_CIPHER_all_funcs(cname, cprefix, cbits, kstruct, ksched) \
        BLOCK_CIPHER_defs(cname, kstruct, nid, block_size, key_len, iv_len, \
                          cbits, flags, init_key, cleanup, set_asn1, \
                          get_asn1, ctrl)

#define IMPLEMENT_CFBR(cipher,cprefix,kstruct,ksched,keysize,cbits,iv_len,fl) \
        BLOCK_CIPHER_func_cfb(cipher##_##keysize,cprefix,cbits,kstruct,ksched) \
        BLOCK_CIPHER_def_cfb(cipher##_##keysize,kstruct, \
                             NID_##cipher##_##keysize, keysize/8, iv_len, cbits, \
                             (fl)|EVP_CIPH_FLAG_DEFAULT_ASN1, \
                             cipher##_init_key, NULL, NULL, NULL, NULL)

typedef struct {
    unsigned char iv[EVP_MAX_IV_LENGTH];
    unsigned int iv_len;
    unsigned int tag_len;
} evp_cipher_aead_asn1_params;

int evp_cipher_param_to_asn1_ex(EVP_CIPHER_CTX *c, ASN1_TYPE *type,
                                evp_cipher_aead_asn1_params *params);

int evp_cipher_asn1_to_param_ex(EVP_CIPHER_CTX *c, ASN1_TYPE *type,
                                evp_cipher_aead_asn1_params *params);

/*
 * To support transparent execution of operation in backends other
 * than the "origin" key, we support transparent export/import to
 * those providers, and maintain a cache of the imported keydata,
 * so we don't need to redo the export/import every time we perform
 * the same operation in that same provider.
 * This requires that the "origin" backend (whether it's a legacy or a
 * provider "origin") implements exports, and that the target provider
 * has an EVP_KEYMGMT that implements import.
 */
typedef struct {
    EVP_KEYMGMT *keymgmt;
    void *keydata;
} OP_CACHE_ELEM;

DEFINE_STACK_OF(OP_CACHE_ELEM)

/*
 * An EVP_PKEY can have the following states:
 *
 * untyped & empty:
 *
 *     type == EVP_PKEY_NONE && keymgmt == NULL
 *
 * typed & empty:
 *
 *     (type != EVP_PKEY_NONE && pkey.ptr == NULL)      ## legacy (libcrypto only)
 *     || (keymgmt != NULL && keydata == NULL)          ## provider side
 *
 * fully assigned:
 *
 *     (type != EVP_PKEY_NONE && pkey.ptr != NULL)      ## legacy (libcrypto only)
 *     || (keymgmt != NULL && keydata != NULL)          ## provider side
 *
 * The easiest way to detect a legacy key is:
 *
 *     keymgmt == NULL && type != EVP_PKEY_NONE
 *
 * The easiest way to detect a provider side key is:
 *
 *     keymgmt != NULL
 */
#define evp_pkey_is_blank(pk)                                   \
    ((pk)->type == EVP_PKEY_NONE && (pk)->keymgmt == NULL)
#define evp_pkey_is_typed(pk)                                   \
    ((pk)->type != EVP_PKEY_NONE || (pk)->keymgmt != NULL)
#ifndef FIPS_MODULE
# define evp_pkey_is_assigned(pk)                               \
    ((pk)->pkey.ptr != NULL || (pk)->keydata != NULL)
#else
# define evp_pkey_is_assigned(pk)                               \
    ((pk)->keydata != NULL)
#endif
#define evp_pkey_is_legacy(pk)                                  \
    ((pk)->type != EVP_PKEY_NONE && (pk)->keymgmt == NULL)
#define evp_pkey_is_provided(pk)                                \
    ((pk)->keymgmt != NULL)

union legacy_pkey_st {
    void *ptr;
    struct rsa_st *rsa;     /* RSA */
#  ifndef OPENSSL_NO_DSA
    struct dsa_st *dsa;     /* DSA */
#  endif
#  ifndef OPENSSL_NO_DH
    struct dh_st *dh;       /* DH */
#  endif
#  ifndef OPENSSL_NO_EC
    struct ec_key_st *ec;   /* ECC */
    ECX_KEY *ecx;           /* X25519, X448, Ed25519, Ed448 */
#  endif
};

struct evp_pkey_st {
    /* == Legacy attributes == */
    int type;
    int save_type;

# ifndef FIPS_MODULE
    /*
     * Legacy key "origin" is composed of a pointer to an EVP_PKEY_ASN1_METHOD,
     * a pointer to a low level key and possibly a pointer to an engine.
     */
    const EVP_PKEY_ASN1_METHOD *ameth;
    ENGINE *engine;
    ENGINE *pmeth_engine; /* If not NULL public key ENGINE to use */

    /* Union to store the reference to an origin legacy key */
    union legacy_pkey_st pkey;

    /* Union to store the reference to a non-origin legacy key */
    union legacy_pkey_st legacy_cache_pkey;
# endif

    /* == Common attributes == */
    CRYPTO_REF_COUNT references;
    CRYPTO_RWLOCK *lock;
#ifndef FIPS_MODULE
    STACK_OF(X509_ATTRIBUTE) *attributes; /* [ 0 ] */
    int save_parameters;
    unsigned int foreign:1; /* the low-level key is using an engine or an app-method */
    CRYPTO_EX_DATA ex_data;
#endif

    /* == Provider attributes == */

    /*
     * Provider keydata "origin" is composed of a pointer to an EVP_KEYMGMT
     * and a pointer to the provider side key data.  This is never used at
     * the same time as the legacy key data above.
     */
    EVP_KEYMGMT *keymgmt;
    void *keydata;
    /*
     * If any libcrypto code does anything that may modify the keydata
     * contents, this dirty counter must be incremented.
     */
    size_t dirty_cnt;

    /*
     * To support transparent execution of operation in backends other
     * than the "origin" key, we support transparent export/import to
     * those providers, and maintain a cache of the imported keydata,
     * so we don't need to redo the export/import every time we perform
     * the same operation in that same provider.
     */
    STACK_OF(OP_CACHE_ELEM) *operation_cache;

    /*
     * We keep a copy of that "origin"'s dirty count, so we know if the
     * operation cache needs flushing.
     */
    size_t dirty_cnt_copy;

    /* Cache of key object information */
    struct {
        int bits;
        int security_bits;
        int size;
    } cache;
} /* EVP_PKEY */ ;

#define EVP_PKEY_CTX_IS_SIGNATURE_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_SIGN \
     || (ctx)->operation == EVP_PKEY_OP_SIGNCTX \
     || (ctx)->operation == EVP_PKEY_OP_VERIFY \
     || (ctx)->operation == EVP_PKEY_OP_VERIFYCTX \
     || (ctx)->operation == EVP_PKEY_OP_VERIFYRECOVER)

#define EVP_PKEY_CTX_IS_DERIVE_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_DERIVE)

#define EVP_PKEY_CTX_IS_ASYM_CIPHER_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_ENCRYPT \
     || (ctx)->operation == EVP_PKEY_OP_DECRYPT)

#define EVP_PKEY_CTX_IS_GEN_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_PARAMGEN \
     || (ctx)->operation == EVP_PKEY_OP_KEYGEN)

#define EVP_PKEY_CTX_IS_FROMDATA_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_FROMDATA)

#define EVP_PKEY_CTX_IS_KEM_OP(ctx) \
    ((ctx)->operation == EVP_PKEY_OP_ENCAPSULATE \
     || (ctx)->operation == EVP_PKEY_OP_DECAPSULATE)

void openssl_add_all_ciphers_int(void);
void openssl_add_all_digests_int(void);
void evp_cleanup_int(void);
void evp_app_cleanup_int(void);
void *evp_pkey_export_to_provider(EVP_PKEY *pk, OSSL_LIB_CTX *libctx,
                                  EVP_KEYMGMT **keymgmt,
                                  const char *propquery);
#ifndef FIPS_MODULE
int evp_pkey_copy_downgraded(EVP_PKEY **dest, const EVP_PKEY *src);
void *evp_pkey_get_legacy(EVP_PKEY *pk);
void evp_pkey_free_legacy(EVP_PKEY *x);
EVP_PKEY *evp_pkcs82pkey_legacy(const PKCS8_PRIV_KEY_INFO *p8inf,
                                OSSL_LIB_CTX *libctx, const char *propq);
#endif

/*
 * KEYMGMT utility functions
 */

/*
 * Key import structure and helper function, to be used as an export callback
 */
struct evp_keymgmt_util_try_import_data_st {
    EVP_KEYMGMT *keymgmt;
    void *keydata;

    int selection;
};
int evp_keymgmt_util_try_import(const OSSL_PARAM params[], void *arg);
int evp_keymgmt_util_assign_pkey(EVP_PKEY *pkey, EVP_KEYMGMT *keymgmt,
                                 void *keydata);
EVP_PKEY *evp_keymgmt_util_make_pkey(EVP_KEYMGMT *keymgmt, void *keydata);

int evp_keymgmt_util_export(const EVP_PKEY *pk, int selection,
                            OSSL_CALLBACK *export_cb, void *export_cbarg);
void *evp_keymgmt_util_export_to_provider(EVP_PKEY *pk, EVP_KEYMGMT *keymgmt);
OP_CACHE_ELEM *evp_keymgmt_util_find_operation_cache(EVP_PKEY *pk,
                                                     EVP_KEYMGMT *keymgmt);
int evp_keymgmt_util_clear_operation_cache(EVP_PKEY *pk, int locking);
int evp_keymgmt_util_cache_keydata(EVP_PKEY *pk,
                                   EVP_KEYMGMT *keymgmt, void *keydata);
void evp_keymgmt_util_cache_keyinfo(EVP_PKEY *pk);
void *evp_keymgmt_util_fromdata(EVP_PKEY *target, EVP_KEYMGMT *keymgmt,
                                int selection, const OSSL_PARAM params[]);
int evp_keymgmt_util_has(EVP_PKEY *pk, int selection);
int evp_keymgmt_util_match(EVP_PKEY *pk1, EVP_PKEY *pk2, int selection);
int evp_keymgmt_util_copy(EVP_PKEY *to, EVP_PKEY *from, int selection);
void *evp_keymgmt_util_gen(EVP_PKEY *target, EVP_KEYMGMT *keymgmt,
                           void *genctx, OSSL_CALLBACK *cb, void *cbarg);
int evp_keymgmt_util_get_deflt_digest_name(EVP_KEYMGMT *keymgmt,
                                           void *keydata,
                                           char *mdname, size_t mdname_sz);
const char *evp_keymgmt_util_query_operation_name(EVP_KEYMGMT *keymgmt,
                                                  int op_id);

/*
 * KEYMGMT provider interface functions
 */
void *evp_keymgmt_newdata(const EVP_KEYMGMT *keymgmt);
void evp_keymgmt_freedata(const EVP_KEYMGMT *keymgmt, void *keyddata);
int evp_keymgmt_get_params(const EVP_KEYMGMT *keymgmt,
                           void *keydata, OSSL_PARAM params[]);
int evp_keymgmt_set_params(const EVP_KEYMGMT *keymgmt,
                           void *keydata, const OSSL_PARAM params[]);
void *evp_keymgmt_gen_init(const EVP_KEYMGMT *keymgmt, int selection,
                           const OSSL_PARAM params[]);
int evp_keymgmt_gen_set_template(const EVP_KEYMGMT *keymgmt, void *genctx,
                                 void *template);
int evp_keymgmt_gen_set_params(const EVP_KEYMGMT *keymgmt, void *genctx,
                               const OSSL_PARAM params[]);
void *evp_keymgmt_gen(const EVP_KEYMGMT *keymgmt, void *genctx,
                      OSSL_CALLBACK *cb, void *cbarg);
void evp_keymgmt_gen_cleanup(const EVP_KEYMGMT *keymgmt, void *genctx);

int evp_keymgmt_has_load(const EVP_KEYMGMT *keymgmt);
void *evp_keymgmt_load(const EVP_KEYMGMT *keymgmt,
                       const void *objref, size_t objref_sz);

int evp_keymgmt_has(const EVP_KEYMGMT *keymgmt, void *keyddata, int selection);
int evp_keymgmt_validate(const EVP_KEYMGMT *keymgmt, void *keydata,
                         int selection, int checktype);
int evp_keymgmt_match(const EVP_KEYMGMT *keymgmt,
                      const void *keydata1, const void *keydata2,
                      int selection);

int evp_keymgmt_import(const EVP_KEYMGMT *keymgmt, void *keydata,
                       int selection, const OSSL_PARAM params[]);
const OSSL_PARAM *evp_keymgmt_import_types(const EVP_KEYMGMT *keymgmt,
                                           int selection);
int evp_keymgmt_export(const EVP_KEYMGMT *keymgmt, void *keydata,
                       int selection, OSSL_CALLBACK *param_cb, void *cbarg);
const OSSL_PARAM *evp_keymgmt_export_types(const EVP_KEYMGMT *keymgmt,
                                           int selection);
void *evp_keymgmt_dup(const EVP_KEYMGMT *keymgmt,
                      const void *keydata_from, int selection);
EVP_KEYMGMT *evp_keymgmt_fetch_from_prov(OSSL_PROVIDER *prov,
                                         const char *name,
                                         const char *properties);

/* Pulling defines out of C source files */

# define EVP_RC4_KEY_SIZE 16
# ifndef TLS1_1_VERSION
#  define TLS1_1_VERSION   0x0302
# endif

void evp_encode_ctx_set_flags(EVP_ENCODE_CTX *ctx, unsigned int flags);

/* EVP_ENCODE_CTX flags */
/* Don't generate new lines when encoding */
#define EVP_ENCODE_CTX_NO_NEWLINES          1
/* Use the SRP base64 alphabet instead of the standard one */
#define EVP_ENCODE_CTX_USE_SRP_ALPHABET     2

const EVP_CIPHER *evp_get_cipherbyname_ex(OSSL_LIB_CTX *libctx,
                                          const char *name);
const EVP_MD *evp_get_digestbyname_ex(OSSL_LIB_CTX *libctx,
                                      const char *name);

int ossl_pkcs5_pbkdf2_hmac_ex(const char *pass, int passlen,
                              const unsigned char *salt, int saltlen, int iter,
                              const EVP_MD *digest, int keylen,
                              unsigned char *out,
                              OSSL_LIB_CTX *libctx, const char *propq);

# ifndef FIPS_MODULE
/*
 * Internal helpers for stricter EVP_PKEY_CTX_{set,get}_params().
 *
 * Return 1 on success, 0 or negative for errors.
 *
 * In particular they return -2 if any of the params is not supported.
 *
 * They are not available in FIPS_MODULE as they depend on
 *      - EVP_PKEY_CTX_{get,set}_params()
 *      - EVP_PKEY_CTX_{gettable,settable}_params()
 *
 */
int evp_pkey_ctx_set_params_strict(EVP_PKEY_CTX *ctx, OSSL_PARAM *params);
int evp_pkey_ctx_get_params_strict(EVP_PKEY_CTX *ctx, OSSL_PARAM *params);

EVP_MD_CTX *evp_md_ctx_new_ex(EVP_PKEY *pkey, const ASN1_OCTET_STRING *id,
                              OSSL_LIB_CTX *libctx, const char *propq);
int evp_pkey_name2type(const char *name);
const char *evp_pkey_type2name(int type);

int evp_pkey_ctx_set1_id_prov(EVP_PKEY_CTX *ctx, const void *id, int len);
int evp_pkey_ctx_get1_id_prov(EVP_PKEY_CTX *ctx, void *id);
int evp_pkey_ctx_get1_id_len_prov(EVP_PKEY_CTX *ctx, size_t *id_len);

int evp_pkey_ctx_use_cached_data(EVP_PKEY_CTX *ctx);
# endif /* !defined(FIPS_MODULE) */

int evp_method_store_cache_flush(OSSL_LIB_CTX *libctx);
int evp_method_store_remove_all_provided(const OSSL_PROVIDER *prov);

int evp_default_properties_enable_fips_int(OSSL_LIB_CTX *libctx, int enable,
                                           int loadconfig);
int evp_set_default_properties_int(OSSL_LIB_CTX *libctx, const char *propq,
                                   int loadconfig, int mirrored);
char *evp_get_global_properties_str(OSSL_LIB_CTX *libctx, int loadconfig);

void evp_md_ctx_clear_digest(EVP_MD_CTX *ctx, int force, int keep_digest);

/* Three possible states: */
# define EVP_PKEY_STATE_UNKNOWN         0
# define EVP_PKEY_STATE_LEGACY          1
# define EVP_PKEY_STATE_PROVIDER        2
int evp_pkey_ctx_state(const EVP_PKEY_CTX *ctx);

/* These two must ONLY be called for provider side operations */
int evp_pkey_ctx_ctrl_to_param(EVP_PKEY_CTX *ctx,
                               int keytype, int optype,
                               int cmd, int p1, void *p2);
int evp_pkey_ctx_ctrl_str_to_param(EVP_PKEY_CTX *ctx,
                                   const char *name, const char *value);

/* These two must ONLY be called for legacy operations */
int evp_pkey_ctx_set_params_to_ctrl(EVP_PKEY_CTX *ctx, const OSSL_PARAM *params);
int evp_pkey_ctx_get_params_to_ctrl(EVP_PKEY_CTX *ctx, OSSL_PARAM *params);

/* This must ONLY be called for legacy EVP_PKEYs */
int evp_pkey_get_params_to_ctrl(const EVP_PKEY *pkey, OSSL_PARAM *params);

/* Same as the public get0 functions but are not const */
# ifndef OPENSSL_NO_DEPRECATED_3_0
DH *evp_pkey_get0_DH_int(const EVP_PKEY *pkey);
EC_KEY *evp_pkey_get0_EC_KEY_int(const EVP_PKEY *pkey);
RSA *evp_pkey_get0_RSA_int(const EVP_PKEY *pkey);
# endif

/* Get internal identification number routines */
int evp_asym_cipher_get_number(const EVP_ASYM_CIPHER *cipher);
int evp_cipher_get_number(const EVP_CIPHER *cipher);
int evp_kdf_get_number(const EVP_KDF *kdf);
int evp_kem_get_number(const EVP_KEM *wrap);
int evp_keyexch_get_number(const EVP_KEYEXCH *keyexch);
int evp_keymgmt_get_number(const EVP_KEYMGMT *keymgmt);
int evp_mac_get_number(const EVP_MAC *mac);
int evp_md_get_number(const EVP_MD *md);
int evp_rand_get_number(const EVP_RAND *rand);
int evp_signature_get_number(const EVP_SIGNATURE *signature);

#endif /* OSSL_CRYPTO_EVP_H */