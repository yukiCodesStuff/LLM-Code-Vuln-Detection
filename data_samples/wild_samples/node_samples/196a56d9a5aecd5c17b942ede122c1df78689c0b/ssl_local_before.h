typedef enum tlsext_index_en {
    TLSEXT_IDX_renegotiate,
    TLSEXT_IDX_server_name,
    TLSEXT_IDX_max_fragment_length,
    TLSEXT_IDX_srp,
    TLSEXT_IDX_ec_point_formats,
    TLSEXT_IDX_supported_groups,
    TLSEXT_IDX_session_ticket,
    TLSEXT_IDX_status_request,
    TLSEXT_IDX_next_proto_neg,
    TLSEXT_IDX_application_layer_protocol_negotiation,
    TLSEXT_IDX_use_srtp,
    TLSEXT_IDX_encrypt_then_mac,
    TLSEXT_IDX_signed_certificate_timestamp,
    TLSEXT_IDX_extended_master_secret,
    TLSEXT_IDX_signature_algorithms_cert,
    TLSEXT_IDX_post_handshake_auth,
    TLSEXT_IDX_signature_algorithms,
    TLSEXT_IDX_supported_versions,
    TLSEXT_IDX_psk_kex_modes,
    TLSEXT_IDX_key_share,
    TLSEXT_IDX_cookie,
    TLSEXT_IDX_cryptopro_bug,
    TLSEXT_IDX_early_data,
    TLSEXT_IDX_certificate_authorities,
    TLSEXT_IDX_quic_transport_params_draft,
    TLSEXT_IDX_quic_transport_params,
    TLSEXT_IDX_padding,
    TLSEXT_IDX_psk,
    /* Dummy index - must always be the last entry */
    TLSEXT_IDX_num_builtins
} TLSEXT_INDEX;

DEFINE_LHASH_OF(SSL_SESSION);
/* Needed in ssl_cert.c */
DEFINE_LHASH_OF(X509_NAME);

# define TLSEXT_KEYNAME_LENGTH  16
# define TLSEXT_TICK_KEY_LENGTH 32

typedef struct ssl_ctx_ext_secure_st {
    unsigned char tick_hmac_key[TLSEXT_TICK_KEY_LENGTH];
    unsigned char tick_aes_key[TLSEXT_TICK_KEY_LENGTH];
} SSL_CTX_EXT_SECURE;

/*
 * Helper function for HMAC
 * The structure should be considered opaque, it will change once the low
 * level deprecated calls are removed.  At that point it can be replaced
 * by EVP_MAC_CTX and most of the functions converted to macros or inlined
 * directly.
 */
typedef struct ssl_hmac_st {
    EVP_MAC_CTX *ctx;
# ifndef OPENSSL_NO_DEPRECATED_3_0
    HMAC_CTX *old_ctx;
# endif
} SSL_HMAC;

SSL_HMAC *ssl_hmac_new(const SSL_CTX *ctx);
void ssl_hmac_free(SSL_HMAC *ctx);
# ifndef OPENSSL_NO_DEPRECATED_3_0
HMAC_CTX *ssl_hmac_get0_HMAC_CTX(SSL_HMAC *ctx);
# endif
EVP_MAC_CTX *ssl_hmac_get0_EVP_MAC_CTX(SSL_HMAC *ctx);
int ssl_hmac_init(SSL_HMAC *ctx, void *key, size_t len, char *md);
int ssl_hmac_update(SSL_HMAC *ctx, const unsigned char *data, size_t len);
int ssl_hmac_final(SSL_HMAC *ctx, unsigned char *md, size_t *len,
                   size_t max_size);
size_t ssl_hmac_size(const SSL_HMAC *ctx);

int ssl_get_EC_curve_nid(const EVP_PKEY *pkey);
__owur int tls13_set_encoded_pub_key(EVP_PKEY *pkey,
                                     const unsigned char *enckey,
                                     size_t enckeylen);

typedef struct tls_group_info_st {
    char *tlsname;           /* Curve Name as in TLS specs */
    char *realname;          /* Curve Name according to provider */
    char *algorithm;         /* Algorithm name to fetch */
    unsigned int secbits;    /* Bits of security (from SP800-57) */
    uint16_t group_id;       /* Group ID */
    int mintls;              /* Minimum TLS version, -1 unsupported */
    int maxtls;              /* Maximum TLS version (or 0 for undefined) */
    int mindtls;             /* Minimum DTLS version, -1 unsupported */
    int maxdtls;             /* Maximum DTLS version (or 0 for undefined) */
    char is_kem;             /* Mode for this Group: 0 is KEX, 1 is KEM */
} TLS_GROUP_INFO;

/* flags values */
# define TLS_GROUP_TYPE             0x0000000FU /* Mask for group type */
# define TLS_GROUP_CURVE_PRIME      0x00000001U
# define TLS_GROUP_CURVE_CHAR2      0x00000002U
# define TLS_GROUP_CURVE_CUSTOM     0x00000004U
# define TLS_GROUP_FFDHE            0x00000008U
# define TLS_GROUP_ONLY_FOR_TLS1_3  0x00000010U

# define TLS_GROUP_FFDHE_FOR_TLS1_3 (TLS_GROUP_FFDHE|TLS_GROUP_ONLY_FOR_TLS1_3)

struct ssl_ctx_st {
    OSSL_LIB_CTX *libctx;

    const SSL_METHOD *method;
    STACK_OF(SSL_CIPHER) *cipher_list;
    /* same as above but sorted for lookup */
    STACK_OF(SSL_CIPHER) *cipher_list_by_id;
    /* TLSv1.3 specific ciphersuites */
    STACK_OF(SSL_CIPHER) *tls13_ciphersuites;
    struct x509_store_st /* X509_STORE */ *cert_store;
    LHASH_OF(SSL_SESSION) *sessions;
    /*
     * Most session-ids that will be cached, default is
     * SSL_SESSION_CACHE_MAX_SIZE_DEFAULT. 0 is unlimited.
     */
    size_t session_cache_size;
    struct ssl_session_st *session_cache_head;
    struct ssl_session_st *session_cache_tail;
    /*
     * This can have one of 2 values, ored together, SSL_SESS_CACHE_CLIENT,
     * SSL_SESS_CACHE_SERVER, Default is SSL_SESSION_CACHE_SERVER, which
     * means only SSL_accept will cache SSL_SESSIONS.
     */
    uint32_t session_cache_mode;
    /*
     * If timeout is not 0, it is the default timeout value set when
     * SSL_new() is called.  This has been put in to make life easier to set
     * things up
     */
    long session_timeout;
    /*
     * If this callback is not null, it will be called each time a session id
     * is added to the cache.  If this function returns 1, it means that the
     * callback will do a SSL_SESSION_free() when it has finished using it.
     * Otherwise, on 0, it means the callback has finished with it. If
     * remove_session_cb is not null, it will be called when a session-id is
     * removed from the cache.  After the call, OpenSSL will
     * SSL_SESSION_free() it.
     */
    int (*new_session_cb) (struct ssl_st *ssl, SSL_SESSION *sess);
    void (*remove_session_cb) (struct ssl_ctx_st *ctx, SSL_SESSION *sess);
    SSL_SESSION *(*get_session_cb) (struct ssl_st *ssl,
                                    const unsigned char *data, int len,
                                    int *copy);
    struct {
        TSAN_QUALIFIER int sess_connect;       /* SSL new conn - started */
        TSAN_QUALIFIER int sess_connect_renegotiate; /* SSL reneg - requested */
        TSAN_QUALIFIER int sess_connect_good;  /* SSL new conne/reneg - finished */
        TSAN_QUALIFIER int sess_accept;        /* SSL new accept - started */
        TSAN_QUALIFIER int sess_accept_renegotiate; /* SSL reneg - requested */
        TSAN_QUALIFIER int sess_accept_good;   /* SSL accept/reneg - finished */
        TSAN_QUALIFIER int sess_miss;          /* session lookup misses */
        TSAN_QUALIFIER int sess_timeout;       /* reuse attempt on timeouted session */
        TSAN_QUALIFIER int sess_cache_full;    /* session removed due to full cache */
        TSAN_QUALIFIER int sess_hit;           /* session reuse actually done */
        TSAN_QUALIFIER int sess_cb_hit;        /* session-id that was not in
                                                * the cache was passed back via
                                                * the callback. This indicates
                                                * that the application is
                                                * supplying session-id's from
                                                * other processes - spooky
                                                * :-) */
    } stats;
#ifdef TSAN_REQUIRES_LOCKING
    CRYPTO_RWLOCK *tsan_lock;
#endif

    CRYPTO_REF_COUNT references;

    /* if defined, these override the X509_verify_cert() calls */
    int (*app_verify_callback) (X509_STORE_CTX *, void *);
    void *app_verify_arg;
    /*
     * before OpenSSL 0.9.7, 'app_verify_arg' was ignored
     * ('app_verify_callback' was called with just one argument)
     */

    /* Default password callback. */
    pem_password_cb *default_passwd_callback;

    /* Default password callback user data. */
    void *default_passwd_callback_userdata;

    /* get client cert callback */
    int (*client_cert_cb) (SSL *ssl, X509 **x509, EVP_PKEY **pkey);

    /* cookie generate callback */
    int (*app_gen_cookie_cb) (SSL *ssl, unsigned char *cookie,
                              unsigned int *cookie_len);

    /* verify cookie callback */
    int (*app_verify_cookie_cb) (SSL *ssl, const unsigned char *cookie,
                                 unsigned int cookie_len);

    /* TLS1.3 app-controlled cookie generate callback */
    int (*gen_stateless_cookie_cb) (SSL *ssl, unsigned char *cookie,
                                    size_t *cookie_len);

    /* TLS1.3 verify app-controlled cookie callback */
    int (*verify_stateless_cookie_cb) (SSL *ssl, const unsigned char *cookie,
                                       size_t cookie_len);

    CRYPTO_EX_DATA ex_data;

    const EVP_MD *md5;          /* For SSLv3/TLSv1 'ssl3-md5' */
    const EVP_MD *sha1;         /* For SSLv3/TLSv1 'ssl3-sha1' */

    STACK_OF(X509) *extra_certs;
    STACK_OF(SSL_COMP) *comp_methods; /* stack of SSL_COMP, SSLv3/TLSv1 */

    /* Default values used when no per-SSL value is defined follow */

    /* used if SSL's info_callback is NULL */
    void (*info_callback) (const SSL *ssl, int type, int val);

    /*
     * What we put in certificate_authorities extension for TLS 1.3
     * (ClientHello and CertificateRequest) or just client cert requests for
     * earlier versions. If client_ca_names is populated then it is only used
     * for client cert requests, and in preference to ca_names.
     */
    STACK_OF(X509_NAME) *ca_names;
    STACK_OF(X509_NAME) *client_ca_names;

    /*
     * Default values to use in SSL structures follow (these are copied by
     * SSL_new)
     */

    uint64_t options;
    uint32_t mode;
    int min_proto_version;
    int max_proto_version;
    size_t max_cert_list;

    struct cert_st /* CERT */ *cert;
    int read_ahead;

    /* callback that allows applications to peek at protocol messages */
    void (*msg_callback) (int write_p, int version, int content_type,
                          const void *buf, size_t len, SSL *ssl, void *arg);
    void *msg_callback_arg;

    uint32_t verify_mode;
    size_t sid_ctx_length;
    unsigned char sid_ctx[SSL_MAX_SID_CTX_LENGTH];
    /* called 'verify_callback' in the SSL */
    int (*default_verify_callback) (int ok, X509_STORE_CTX *ctx);

    /* Default generate session ID callback. */
    GEN_SESSION_CB generate_session_id;

    X509_VERIFY_PARAM *param;

    int quiet_shutdown;

# ifndef OPENSSL_NO_CT
    CTLOG_STORE *ctlog_store;   /* CT Log Store */
    /*
     * Validates that the SCTs (Signed Certificate Timestamps) are sufficient.
     * If they are not, the connection should be aborted.
     */
    ssl_ct_validation_cb ct_validation_callback;
    void *ct_validation_callback_arg;
# endif

    /*
     * If we're using more than one pipeline how should we divide the data
     * up between the pipes?
     */
    size_t split_send_fragment;
    /*
     * Maximum amount of data to send in one fragment. actual record size can
     * be more than this due to padding and MAC overheads.
     */
    size_t max_send_fragment;

    /* Up to how many pipelines should we use? If 0 then 1 is assumed */
    size_t max_pipelines;

    /* The default read buffer length to use (0 means not set) */
    size_t default_read_buf_len;

# ifndef OPENSSL_NO_ENGINE
    /*
     * Engine to pass requests for client certs to
     */
    ENGINE *client_cert_engine;
# endif

    /* ClientHello callback.  Mostly for extensions, but not entirely. */
    SSL_client_hello_cb_fn client_hello_cb;
    void *client_hello_cb_arg;

    /* TLS extensions. */
    struct {
        /* TLS extensions servername callback */
        int (*servername_cb) (SSL *, int *, void *);
        void *servername_arg;
        /* RFC 4507 session ticket keys */
        unsigned char tick_key_name[TLSEXT_KEYNAME_LENGTH];
        SSL_CTX_EXT_SECURE *secure;
# ifndef OPENSSL_NO_DEPRECATED_3_0
        /* Callback to support customisation of ticket key setting */
        int (*ticket_key_cb) (SSL *ssl,
                              unsigned char *name, unsigned char *iv,
                              EVP_CIPHER_CTX *ectx, HMAC_CTX *hctx, int enc);
#endif
        int (*ticket_key_evp_cb) (SSL *ssl,
                                  unsigned char *name, unsigned char *iv,
                                  EVP_CIPHER_CTX *ectx, EVP_MAC_CTX *hctx,
                                  int enc);

        /* certificate status request info */
        /* Callback for status request */
        int (*status_cb) (SSL *ssl, void *arg);
        void *status_arg;
        /* ext status type used for CSR extension (OCSP Stapling) */
        int status_type;
        /* RFC 4366 Maximum Fragment Length Negotiation */
        uint8_t max_fragment_len_mode;

        /* EC extension values inherited by SSL structure */
        size_t ecpointformats_len;
        unsigned char *ecpointformats;

        size_t supportedgroups_len;
        uint16_t *supportedgroups;

        uint16_t *supported_groups_default;
        size_t supported_groups_default_len;
        /*
         * ALPN information (we are in the process of transitioning from NPN to
         * ALPN.)
         */

        /*-
         * For a server, this contains a callback function that allows the
         * server to select the protocol for the connection.
         *   out: on successful return, this must point to the raw protocol
         *        name (without the length prefix).
         *   outlen: on successful return, this contains the length of |*out|.
         *   in: points to the client's list of supported protocols in
         *       wire-format.
         *   inlen: the length of |in|.
         */
        int (*alpn_select_cb) (SSL *s,
                               const unsigned char **out,
                               unsigned char *outlen,
                               const unsigned char *in,
                               unsigned int inlen, void *arg);
        void *alpn_select_cb_arg;

        /*
         * For a client, this contains the list of supported protocols in wire
         * format.
         */
        unsigned char *alpn;
        size_t alpn_len;

# ifndef OPENSSL_NO_NEXTPROTONEG
        /* Next protocol negotiation information */

        /*
         * For a server, this contains a callback function by which the set of
         * advertised protocols can be provided.
         */
        SSL_CTX_npn_advertised_cb_func npn_advertised_cb;
        void *npn_advertised_cb_arg;
        /*
         * For a client, this contains a callback function that selects the next
         * protocol from the list provided by the server.
         */
        SSL_CTX_npn_select_cb_func npn_select_cb;
        void *npn_select_cb_arg;
# endif

        unsigned char cookie_hmac_key[SHA256_DIGEST_LENGTH];
    } ext;

# ifndef OPENSSL_NO_PSK
    SSL_psk_client_cb_func psk_client_callback;
    SSL_psk_server_cb_func psk_server_callback;
# endif
    SSL_psk_find_session_cb_func psk_find_session_cb;
    SSL_psk_use_session_cb_func psk_use_session_cb;

# ifndef OPENSSL_NO_SRP
    SRP_CTX srp_ctx;            /* ctx for SRP authentication */
# endif

    /* Shared DANE context */
    struct dane_ctx_st dane;

# ifndef OPENSSL_NO_SRTP
    /* SRTP profiles we are willing to do from RFC 5764 */
    STACK_OF(SRTP_PROTECTION_PROFILE) *srtp_profiles;
# endif
    /*
     * Callback for disabling session caching and ticket support on a session
     * basis, depending on the chosen cipher.
     */
    int (*not_resumable_session_cb) (SSL *ssl, int is_forward_secure);

    CRYPTO_RWLOCK *lock;

    /*
     * Callback for logging key material for use with debugging tools like
     * Wireshark. The callback should log `line` followed by a newline.
     */
    SSL_CTX_keylog_cb_func keylog_callback;

    /*
     * The maximum number of bytes advertised in session tickets that can be
     * sent as early data.
     */
    uint32_t max_early_data;

    /*
     * The maximum number of bytes of early data that a server will tolerate
     * (which should be at least as much as max_early_data).
     */
    uint32_t recv_max_early_data;

    /* TLS1.3 padding callback */
    size_t (*record_padding_cb)(SSL *s, int type, size_t len, void *arg);
    void *record_padding_arg;
    size_t block_padding;

    /* Session ticket appdata */
    SSL_CTX_generate_session_ticket_fn generate_ticket_cb;
    SSL_CTX_decrypt_session_ticket_fn decrypt_ticket_cb;
    void *ticket_cb_data;

    /* The number of TLS1.3 tickets to automatically send */
    size_t num_tickets;

    /* Callback to determine if early_data is acceptable or not */
    SSL_allow_early_data_cb_fn allow_early_data_cb;
    void *allow_early_data_cb_data;

    /* Do we advertise Post-handshake auth support? */
    int pha_enabled;

    /* Callback for SSL async handling */
    SSL_async_callback_fn async_cb;
    void *async_cb_arg;

    char *propq;

    int ssl_mac_pkey_id[SSL_MD_NUM_IDX];
    const EVP_CIPHER *ssl_cipher_methods[SSL_ENC_NUM_IDX];
    const EVP_MD *ssl_digest_methods[SSL_MD_NUM_IDX];
    size_t ssl_mac_secret_size[SSL_MD_NUM_IDX];

    /* Cache of all sigalgs we know and whether they are available or not */
    struct sigalg_lookup_st *sigalg_lookup_cache;

    TLS_GROUP_INFO *group_list;
    size_t group_list_len;
    size_t group_list_max_len;

    /* masks of disabled algorithms */
    uint32_t disabled_enc_mask;
    uint32_t disabled_mac_mask;
    uint32_t disabled_mkey_mask;
    uint32_t disabled_auth_mask;

#ifndef OPENSSL_NO_QUIC
    const SSL_QUIC_METHOD *quic_method;
#endif
};

typedef struct cert_pkey_st CERT_PKEY;

#ifndef OPENSSL_NO_QUIC
struct quic_data_st {
    struct quic_data_st *next;
    OSSL_ENCRYPTION_LEVEL level;
    size_t start;       /* offset into quic_buf->data */
    size_t length;
};
typedef struct quic_data_st QUIC_DATA;
int quic_set_encryption_secrets(SSL *ssl, OSSL_ENCRYPTION_LEVEL level);
#endif

struct ssl_st {
    /*
     * protocol version (one of SSL2_VERSION, SSL3_VERSION, TLS1_VERSION,
     * DTLS1_VERSION)
     */
    int version;
    /* SSLv3 */
    const SSL_METHOD *method;
    /*
     * There are 2 BIO's even though they are normally both the same.  This
     * is so data can be read and written to different handlers
     */
    /* used by SSL_read */
    BIO *rbio;
    /* used by SSL_write */
    BIO *wbio;
    /* used during session-id reuse to concatenate messages */
    BIO *bbio;
    /*
     * This holds a variable that indicates what we were doing when a 0 or -1
     * is returned.  This is needed for non-blocking IO so we know what
     * request needs re-doing when in SSL_accept or SSL_connect
     */
    int rwstate;
    int (*handshake_func) (SSL *);
    /*
     * Imagine that here's a boolean member "init" that is switched as soon
     * as SSL_set_{accept/connect}_state is called for the first time, so
     * that "state" and "handshake_func" are properly initialized.  But as
     * handshake_func is == 0 until then, we use this test instead of an
     * "init" member.
     */
    /* are we the server side? */
    int server;
    /*
     * Generate a new session or reuse an old one.
     * NB: For servers, the 'new' session may actually be a previously
     * cached session or even the previous session unless
     * SSL_OP_NO_SESSION_RESUMPTION_ON_RENEGOTIATION is set
     */
    int new_session;
    /* don't send shutdown packets */
    int quiet_shutdown;
    /* we have shut things down, 0x01 sent, 0x02 for received */
    int shutdown;
    /* where we are */
    OSSL_STATEM statem;
    SSL_EARLY_DATA_STATE early_data_state;
    BUF_MEM *init_buf;          /* buffer used during init */
    void *init_msg;             /* pointer to handshake message body, set by
                                 * ssl3_get_message() */
    size_t init_num;               /* amount read/written */
    size_t init_off;               /* amount read/written */

    struct {
        long flags;
        size_t read_mac_secret_size;
        unsigned char read_mac_secret[EVP_MAX_MD_SIZE];
        size_t write_mac_secret_size;
        unsigned char write_mac_secret[EVP_MAX_MD_SIZE];
        unsigned char server_random[SSL3_RANDOM_SIZE];
        unsigned char client_random[SSL3_RANDOM_SIZE];
        /* flags for countermeasure against known-IV weakness */
        int need_empty_fragments;
        int empty_fragment_done;
        /* used during startup, digest all incoming/outgoing packets */
        BIO *handshake_buffer;
        /*
         * When handshake digest is determined, buffer is hashed and
         * freed and MD_CTX for the required digest is stored here.
         */
        EVP_MD_CTX *handshake_dgst;
        /*
         * Set whenever an expected ChangeCipherSpec message is processed.
         * Unset when the peer's Finished message is received.
         * Unexpected ChangeCipherSpec messages trigger a fatal alert.
         */
        int change_cipher_spec;
        int warn_alert;
        int fatal_alert;
        /*
         * we allow one fatal and one warning alert to be outstanding, send close
         * alert via the warning alert
         */
        int alert_dispatch;
        unsigned char send_alert[2];
        /*
         * This flag is set when we should renegotiate ASAP, basically when there
         * is no more data in the read or write buffers
         */
        int renegotiate;
        int total_renegotiations;
        int num_renegotiations;
        int in_read_app_data;
        struct {
            /* actually only need to be 16+20 for SSLv3 and 12 for TLS */
            unsigned char finish_md[EVP_MAX_MD_SIZE * 2];
            size_t finish_md_len;
            unsigned char peer_finish_md[EVP_MAX_MD_SIZE * 2];
            size_t peer_finish_md_len;
            size_t message_size;
            int message_type;
            /* used to hold the new cipher we are going to use */
            const SSL_CIPHER *new_cipher;
            EVP_PKEY *pkey;         /* holds short lived key exchange key */
            /* used for certificate requests */
            int cert_req;
            /* Certificate types in certificate request message. */
            uint8_t *ctype;
            size_t ctype_len;
            /* Certificate authorities list peer sent */
            STACK_OF(X509_NAME) *peer_ca_names;
            size_t key_block_length;
            unsigned char *key_block;
            const EVP_CIPHER *new_sym_enc;
            const EVP_MD *new_hash;
            int new_mac_pkey_type;
            size_t new_mac_secret_size;
# ifndef OPENSSL_NO_COMP
            const SSL_COMP *new_compression;
# else
            char *new_compression;
# endif
            int cert_request;
            /* Raw values of the cipher list from a client */
            unsigned char *ciphers_raw;
            size_t ciphers_rawlen;
            /* Temporary storage for premaster secret */
            unsigned char *pms;
            size_t pmslen;
# ifndef OPENSSL_NO_PSK
            /* Temporary storage for PSK key */
            unsigned char *psk;
            size_t psklen;
# endif
            /* Signature algorithm we actually use */
            const struct sigalg_lookup_st *sigalg;
            /* Pointer to certificate we use */
            CERT_PKEY *cert;
            /*
             * signature algorithms peer reports: e.g. supported signature
             * algorithms extension for server or as part of a certificate
             * request for client.
             * Keep track of the algorithms for TLS and X.509 usage separately.
             */
            uint16_t *peer_sigalgs;
            uint16_t *peer_cert_sigalgs;
            /* Size of above arrays */
            size_t peer_sigalgslen;
            size_t peer_cert_sigalgslen;
            /* Sigalg peer actually uses */
            const struct sigalg_lookup_st *peer_sigalg;
            /*
             * Set if corresponding CERT_PKEY can be used with current
             * SSL session: e.g. appropriate curve, signature algorithms etc.
             * If zero it can't be used at all.
             */
            uint32_t valid_flags[SSL_PKEY_NUM];
            /*
             * For servers the following masks are for the key and auth algorithms
             * that are supported by the certs below. For clients they are masks of
             * *disabled* algorithms based on the current session.
             */
            uint32_t mask_k;
            uint32_t mask_a;
            /*
             * The following are used by the client to see if a cipher is allowed or
             * not.  It contains the minimum and maximum version the client's using
             * based on what it knows so far.
             */
            int min_ver;
            int max_ver;
        } tmp;

        /* Connection binding to prevent renegotiation attacks */
        unsigned char previous_client_finished[EVP_MAX_MD_SIZE];
        size_t previous_client_finished_len;
        unsigned char previous_server_finished[EVP_MAX_MD_SIZE];
        size_t previous_server_finished_len;
        int send_connection_binding;

# ifndef OPENSSL_NO_NEXTPROTONEG
        /*
         * Set if we saw the Next Protocol Negotiation extension from our peer.
         */
        int npn_seen;
# endif

        /*
         * ALPN information (we are in the process of transitioning from NPN to
         * ALPN.)
         */

        /*
         * In a server these point to the selected ALPN protocol after the
         * ClientHello has been processed. In a client these contain the protocol
         * that the server selected once the ServerHello has been processed.
         */
        unsigned char *alpn_selected;
        size_t alpn_selected_len;
        /* used by the server to know what options were proposed */
        unsigned char *alpn_proposed;
        size_t alpn_proposed_len;
        /* used by the client to know if it actually sent alpn */
        int alpn_sent;

        /*
         * This is set to true if we believe that this is a version of Safari
         * running on OS X 10.6 or newer. We wish to know this because Safari on
         * 10.8 .. 10.8.3 has broken ECDHE-ECDSA support.
         */
        char is_probably_safari;

        /*
         * Track whether we did a key exchange this handshake or not, so
         * SSL_get_negotiated_group() knows whether to fall back to the
         * value in the SSL_SESSION.
         */
        char did_kex;
        /* For clients: peer temporary key */
        /* The group_id for the key exchange key */
        uint16_t group_id;
        EVP_PKEY *peer_tmp;

    } s3;

    struct dtls1_state_st *d1;  /* DTLSv1 variables */
    /* callback that allows applications to peek at protocol messages */
    void (*msg_callback) (int write_p, int version, int content_type,
                          const void *buf, size_t len, SSL *ssl, void *arg);
    void *msg_callback_arg;
    int hit;                    /* reusing a previous session */
    X509_VERIFY_PARAM *param;
    /* Per connection DANE state */
    SSL_DANE dane;
    /* crypto */
    STACK_OF(SSL_CIPHER) *peer_ciphers;
    STACK_OF(SSL_CIPHER) *cipher_list;
    STACK_OF(SSL_CIPHER) *cipher_list_by_id;
    /* TLSv1.3 specific ciphersuites */
    STACK_OF(SSL_CIPHER) *tls13_ciphersuites;
    /*
     * These are the ones being used, the ones in SSL_SESSION are the ones to
     * be 'copied' into these ones
     */
    uint32_t mac_flags;
    /*
     * The TLS1.3 secrets.
     */
    unsigned char early_secret[EVP_MAX_MD_SIZE];
    unsigned char handshake_secret[EVP_MAX_MD_SIZE];
    unsigned char master_secret[EVP_MAX_MD_SIZE];
    unsigned char resumption_master_secret[EVP_MAX_MD_SIZE];
    unsigned char client_finished_secret[EVP_MAX_MD_SIZE];
    unsigned char server_finished_secret[EVP_MAX_MD_SIZE];
    unsigned char server_finished_hash[EVP_MAX_MD_SIZE];
    unsigned char handshake_traffic_hash[EVP_MAX_MD_SIZE];
    unsigned char client_app_traffic_secret[EVP_MAX_MD_SIZE];
    unsigned char server_app_traffic_secret[EVP_MAX_MD_SIZE];
# ifndef OPENSSL_NO_QUIC
    unsigned char client_hand_traffic_secret[EVP_MAX_MD_SIZE];
    unsigned char server_hand_traffic_secret[EVP_MAX_MD_SIZE];
    unsigned char client_early_traffic_secret[EVP_MAX_MD_SIZE];
# endif
    unsigned char exporter_master_secret[EVP_MAX_MD_SIZE];
    unsigned char early_exporter_master_secret[EVP_MAX_MD_SIZE];
    EVP_CIPHER_CTX *enc_read_ctx; /* cryptographic state */
    unsigned char read_iv[EVP_MAX_IV_LENGTH]; /* TLSv1.3 static read IV */
    EVP_MD_CTX *read_hash;      /* used for mac generation */
    COMP_CTX *compress;         /* compression */
    COMP_CTX *expand;           /* uncompress */
    EVP_CIPHER_CTX *enc_write_ctx; /* cryptographic state */
    unsigned char write_iv[EVP_MAX_IV_LENGTH]; /* TLSv1.3 static write IV */
    EVP_MD_CTX *write_hash;     /* used for mac generation */
    /* session info */
    /* client cert? */
    /* This is used to hold the server certificate used */
    struct cert_st /* CERT */ *cert;

    /*
     * The hash of all messages prior to the CertificateVerify, and the length
     * of that hash.
     */
    unsigned char cert_verify_hash[EVP_MAX_MD_SIZE];
    size_t cert_verify_hash_len;

    /* Flag to indicate whether we should send a HelloRetryRequest or not */
    enum {SSL_HRR_NONE = 0, SSL_HRR_PENDING, SSL_HRR_COMPLETE}
        hello_retry_request;

    /*
     * the session_id_context is used to ensure sessions are only reused in
     * the appropriate context
     */
    size_t sid_ctx_length;
    unsigned char sid_ctx[SSL_MAX_SID_CTX_LENGTH];
    /* This can also be in the session once a session is established */
    SSL_SESSION *session;
    /* TLSv1.3 PSK session */
    SSL_SESSION *psksession;
    unsigned char *psksession_id;
    size_t psksession_id_len;
    /* Default generate session ID callback. */
    GEN_SESSION_CB generate_session_id;
    /*
     * The temporary TLSv1.3 session id. This isn't really a session id at all
     * but is a random value sent in the legacy session id field.
     */
    unsigned char tmp_session_id[SSL_MAX_SSL_SESSION_ID_LENGTH];
    size_t tmp_session_id_len;
    /* Used in SSL3 */
    /*
     * 0 don't care about verify failure.
     * 1 fail if verify fails
     */
    uint32_t verify_mode;
    /* fail if callback returns 0 */
    int (*verify_callback) (int ok, X509_STORE_CTX *ctx);
    /* optional informational callback */
    void (*info_callback) (const SSL *ssl, int type, int val);
    /* error bytes to be written */
    int error;
    /* actual code */
    int error_code;
# ifndef OPENSSL_NO_PSK
    SSL_psk_client_cb_func psk_client_callback;
    SSL_psk_server_cb_func psk_server_callback;
# endif
    SSL_psk_find_session_cb_func psk_find_session_cb;
    SSL_psk_use_session_cb_func psk_use_session_cb;

    SSL_CTX *ctx;
    /* Verified chain of peer */
    STACK_OF(X509) *verified_chain;
    long verify_result;
    /* extra application data */
    CRYPTO_EX_DATA ex_data;
    /*
     * What we put in certificate_authorities extension for TLS 1.3
     * (ClientHello and CertificateRequest) or just client cert requests for
     * earlier versions. If client_ca_names is populated then it is only used
     * for client cert requests, and in preference to ca_names.
     */
    STACK_OF(X509_NAME) *ca_names;
    STACK_OF(X509_NAME) *client_ca_names;
    CRYPTO_REF_COUNT references;
    /* protocol behaviour */
    uint64_t options;
    /* API behaviour */
    uint32_t mode;
    int min_proto_version;
    int max_proto_version;
    size_t max_cert_list;
    int first_packet;
    /*
     * What was passed in ClientHello.legacy_version. Used for RSA pre-master
     * secret and SSLv3/TLS (<=1.2) rollback check
     */
    int client_version;
    /*
     * If we're using more than one pipeline how should we divide the data
     * up between the pipes?
     */
    size_t split_send_fragment;
    /*
     * Maximum amount of data to send in one fragment. actual record size can
     * be more than this due to padding and MAC overheads.
     */
    size_t max_send_fragment;
    /* Up to how many pipelines should we use? If 0 then 1 is assumed */
    size_t max_pipelines;

    struct {
        /* Built-in extension flags */
        uint8_t extflags[TLSEXT_IDX_num_builtins];
        /* TLS extension debug callback */
        void (*debug_cb)(SSL *s, int client_server, int type,
                         const unsigned char *data, int len, void *arg);
        void *debug_arg;
        char *hostname;
        /* certificate status request info */
        /* Status type or -1 if no status type */
        int status_type;
        /* Raw extension data, if seen */
        unsigned char *scts;
        /* Length of raw extension data, if seen */
        uint16_t scts_len;
        /* Expect OCSP CertificateStatus message */
        int status_expected;

        struct {
            /* OCSP status request only */
            STACK_OF(OCSP_RESPID) *ids;
            X509_EXTENSIONS *exts;
            /* OCSP response received or to be sent */
            unsigned char *resp;
            size_t resp_len;
        } ocsp;

        /* RFC4507 session ticket expected to be received or sent */
        int ticket_expected;
        /* TLS 1.3 tickets requested by the application. */
        int extra_tickets_expected;
        size_t ecpointformats_len;
        /* our list */
        unsigned char *ecpointformats;

        size_t peer_ecpointformats_len;
        /* peer's list */
        unsigned char *peer_ecpointformats;
        size_t supportedgroups_len;
        /* our list */
        uint16_t *supportedgroups;

        size_t peer_supportedgroups_len;
         /* peer's list */
        uint16_t *peer_supportedgroups;

        /* TLS Session Ticket extension override */
        TLS_SESSION_TICKET_EXT *session_ticket;
        /* TLS Session Ticket extension callback */
        tls_session_ticket_ext_cb_fn session_ticket_cb;
        void *session_ticket_cb_arg;
        /* TLS pre-shared secret session resumption */
        tls_session_secret_cb_fn session_secret_cb;
        void *session_secret_cb_arg;
        /*
         * For a client, this contains the list of supported protocols in wire
         * format.
         */
        unsigned char *alpn;
        size_t alpn_len;
        /*
         * Next protocol negotiation. For the client, this is the protocol that
         * we sent in NextProtocol and is set when handling ServerHello
         * extensions. For a server, this is the client's selected_protocol from
         * NextProtocol and is set when handling the NextProtocol message, before
         * the Finished message.
         */
        unsigned char *npn;
        size_t npn_len;

        /* The available PSK key exchange modes */
        int psk_kex_mode;

        /* Set to one if we have negotiated ETM */
        int use_etm;

        /* Are we expecting to receive early data? */
        int early_data;
        /* Is the session suitable for early data? */
        int early_data_ok;

        /* May be sent by a server in HRR. Must be echoed back in ClientHello */
        unsigned char *tls13_cookie;
        size_t tls13_cookie_len;
        /* Have we received a cookie from the client? */
        int cookieok;

        /*
         * Maximum Fragment Length as per RFC 4366.
         * If this member contains one of the allowed values (1-4)
         * then we should include Maximum Fragment Length Negotiation
         * extension in Client Hello.
         * Please note that value of this member does not have direct
         * effect. The actual (binding) value is stored in SSL_SESSION,
         * as this extension is optional on server side.
         */
        uint8_t max_fragment_len_mode;

        /*
         * On the client side the number of ticket identities we sent in the
         * ClientHello. On the server side the identity of the ticket we
         * selected.
         */
        int tick_identity;

#ifndef OPENSSL_NO_QUIC
        uint8_t *quic_transport_params;
        size_t quic_transport_params_len;
        uint8_t *peer_quic_transport_params_draft;
        size_t peer_quic_transport_params_draft_len;
        uint8_t *peer_quic_transport_params;
        size_t peer_quic_transport_params_len;
#endif
    } ext;

#ifndef OPENSSL_NO_QUIC
    OSSL_ENCRYPTION_LEVEL quic_read_level;
    OSSL_ENCRYPTION_LEVEL quic_write_level;
    OSSL_ENCRYPTION_LEVEL quic_latest_level_received;
    BUF_MEM *quic_buf;          /* buffer incoming handshake messages */
    /*
     * defaults to 0, but can be set to:
     * - TLSEXT_TYPE_quic_transport_parameters_draft
     * - TLSEXT_TYPE_quic_transport_parameters
     * Client: if 0, send both
     * Server: if 0, use same version as client sent
     */
    int quic_transport_version;
    QUIC_DATA *quic_input_data_head;
    QUIC_DATA *quic_input_data_tail;
    size_t quic_next_record_start;
    const SSL_QUIC_METHOD *quic_method;
#endif
    /*
     * Parsed form of the ClientHello, kept around across client_hello_cb
     * calls.
     */
    CLIENTHELLO_MSG *clienthello;

    /*-
     * no further mod of servername
     * 0 : call the servername extension callback.
     * 1 : prepare 2, allow last ack just after in server callback.
     * 2 : don't call servername callback, no ack in server hello
     */
    int servername_done;
# ifndef OPENSSL_NO_CT
    /*
     * Validates that the SCTs (Signed Certificate Timestamps) are sufficient.
     * If they are not, the connection should be aborted.
     */
    ssl_ct_validation_cb ct_validation_callback;
    /* User-supplied argument that is passed to the ct_validation_callback */
    void *ct_validation_callback_arg;
    /*
     * Consolidated stack of SCTs from all sources.
     * Lazily populated by CT_get_peer_scts(SSL*)
     */
    STACK_OF(SCT) *scts;
    /* Have we attempted to find/parse SCTs yet? */
    int scts_parsed;
# endif
    SSL_CTX *session_ctx;       /* initial ctx, used to store sessions */
# ifndef OPENSSL_NO_SRTP
    /* What we'll do */
    STACK_OF(SRTP_PROTECTION_PROFILE) *srtp_profiles;
    /* What's been chosen */
    SRTP_PROTECTION_PROFILE *srtp_profile;
# endif
    /*-
     * 1 if we are renegotiating.
     * 2 if we are a server and are inside a handshake
     * (i.e. not just sending a HelloRequest)
     */
    int renegotiate;
    /* If sending a KeyUpdate is pending */
    int key_update;
    /* Post-handshake authentication state */
    SSL_PHA_STATE post_handshake_auth;
    int pha_enabled;
    uint8_t* pha_context;
    size_t pha_context_len;
    int certreqs_sent;
    EVP_MD_CTX *pha_dgst; /* this is just the digest through ClientFinished */

# ifndef OPENSSL_NO_SRP
    /* ctx for SRP authentication */
    SRP_CTX srp_ctx;
# endif
    /*
     * Callback for disabling session caching and ticket support on a session
     * basis, depending on the chosen cipher.
     */
    int (*not_resumable_session_cb) (SSL *ssl, int is_forward_secure);
    RECORD_LAYER rlayer;
    /* Default password callback. */
    pem_password_cb *default_passwd_callback;
    /* Default password callback user data. */
    void *default_passwd_callback_userdata;
    /* Async Job info */
    ASYNC_JOB *job;
    ASYNC_WAIT_CTX *waitctx;
    size_t asyncrw;

    /*
     * The maximum number of bytes advertised in session tickets that can be
     * sent as early data.
     */
    uint32_t max_early_data;
    /*
     * The maximum number of bytes of early data that a server will tolerate
     * (which should be at least as much as max_early_data).
     */
    uint32_t recv_max_early_data;

    /*
     * The number of bytes of early data received so far. If we accepted early
     * data then this is a count of the plaintext bytes. If we rejected it then
     * this is a count of the ciphertext bytes.
     */
    uint32_t early_data_count;

    /* TLS1.3 padding callback */
    size_t (*record_padding_cb)(SSL *s, int type, size_t len, void *arg);
    void *record_padding_arg;
    size_t block_padding;

    CRYPTO_RWLOCK *lock;

    /* The number of TLS1.3 tickets to automatically send */
    size_t num_tickets;
    /* The number of TLS1.3 tickets actually sent so far */
    size_t sent_tickets;
    /* The next nonce value to use when we send a ticket on this connection */
    uint64_t next_ticket_nonce;

    /* Callback to determine if early_data is acceptable or not */
    SSL_allow_early_data_cb_fn allow_early_data_cb;
    void *allow_early_data_cb_data;

    /* Callback for SSL async handling */
    SSL_async_callback_fn async_cb;
    void *async_cb_arg;

    /*
     * Signature algorithms shared by client and server: cached because these
     * are used most often.
     */
    const struct sigalg_lookup_st **shared_sigalgs;
    size_t shared_sigalgslen;
};

/*
 * Structure containing table entry of values associated with the signature
 * algorithms (signature scheme) extension
*/
typedef struct sigalg_lookup_st {
    /* TLS 1.3 signature scheme name */
    const char *name;
    /* Raw value used in extension */
    uint16_t sigalg;
    /* NID of hash algorithm or NID_undef if no hash */
    int hash;
    /* Index of hash algorithm or -1 if no hash algorithm */
    int hash_idx;
    /* NID of signature algorithm */
    int sig;
    /* Index of signature algorithm */
    int sig_idx;
    /* Combined hash and signature NID, if any */
    int sigandhash;
    /* Required public key curve (ECDSA only) */
    int curve;
    /* Whether this signature algorithm is actually available for use */
    int enabled;
} SIGALG_LOOKUP;

/*
 * Structure containing table entry of certificate info corresponding to
 * CERT_PKEY entries
 */
typedef struct {
    int nid; /* NID of public key algorithm */
    uint32_t amask; /* authmask corresponding to key type */
} SSL_CERT_LOOKUP;

/* DTLS structures */

# ifndef OPENSSL_NO_SCTP
#  define DTLS1_SCTP_AUTH_LABEL   "EXPORTER_DTLS_OVER_SCTP"
# endif

/* Max MTU overhead we know about so far is 40 for IPv6 + 8 for UDP */
# define DTLS1_MAX_MTU_OVERHEAD                   48

/*
 * Flag used in message reuse to indicate the buffer contains the record
 * header as well as the handshake message header.
 */
# define DTLS1_SKIP_RECORD_HEADER                 2

struct dtls1_retransmit_state {
    EVP_CIPHER_CTX *enc_write_ctx; /* cryptographic state */
    EVP_MD_CTX *write_hash;     /* used for mac generation */
    COMP_CTX *compress;         /* compression */
    SSL_SESSION *session;
    unsigned short epoch;
};

struct hm_header_st {
    unsigned char type;
    size_t msg_len;
    unsigned short seq;
    size_t frag_off;
    size_t frag_len;
    unsigned int is_ccs;
    struct dtls1_retransmit_state saved_retransmit_state;
};

typedef struct hm_fragment_st {
    struct hm_header_st msg_header;
    unsigned char *fragment;
    unsigned char *reassembly;
} hm_fragment;

typedef struct pqueue_st pqueue;
typedef struct pitem_st pitem;

struct pitem_st {
    unsigned char priority[8];  /* 64-bit value in big-endian encoding */
    void *data;
    pitem *next;
};

typedef struct pitem_st *piterator;

pitem *pitem_new(unsigned char *prio64be, void *data);
void pitem_free(pitem *item);
pqueue *pqueue_new(void);
void pqueue_free(pqueue *pq);
pitem *pqueue_insert(pqueue *pq, pitem *item);
pitem *pqueue_peek(pqueue *pq);
pitem *pqueue_pop(pqueue *pq);
pitem *pqueue_find(pqueue *pq, unsigned char *prio64be);
pitem *pqueue_iterator(pqueue *pq);
pitem *pqueue_next(piterator *iter);
size_t pqueue_size(pqueue *pq);

typedef struct dtls1_state_st {
    unsigned char cookie[DTLS1_COOKIE_LENGTH];
    size_t cookie_len;
    unsigned int cookie_verified;
    /* handshake message numbers */
    unsigned short handshake_write_seq;
    unsigned short next_handshake_write_seq;
    unsigned short handshake_read_seq;
    /* Buffered handshake messages */
    pqueue *buffered_messages;
    /* Buffered (sent) handshake records */
    pqueue *sent_messages;
    size_t link_mtu;      /* max on-the-wire DTLS packet size */
    size_t mtu;           /* max DTLS packet size */
    struct hm_header_st w_msg_hdr;
    struct hm_header_st r_msg_hdr;
    /* Number of alerts received so far */
    unsigned int timeout_num_alerts;
    /*
     * Indicates when the last handshake msg sent will timeout
     */
    struct timeval next_timeout;
    /* Timeout duration */
    unsigned int timeout_duration_us;

    unsigned int retransmitting;
# ifndef OPENSSL_NO_SCTP
    int shutdown_received;
# endif

    DTLS_timer_cb timer_cb;

} DTLS1_STATE;

/*
 * From ECC-TLS draft, used in encoding the curve type in ECParameters
 */
#  define EXPLICIT_PRIME_CURVE_TYPE  1
#  define EXPLICIT_CHAR2_CURVE_TYPE  2
#  define NAMED_CURVE_TYPE           3

struct cert_pkey_st {
    X509 *x509;
    EVP_PKEY *privatekey;
    /* Chain for this certificate */
    STACK_OF(X509) *chain;
    /*-
     * serverinfo data for this certificate.  The data is in TLS Extension
     * wire format, specifically it's a series of records like:
     *   uint16_t extension_type; // (RFC 5246, 7.4.1.4, Extension)
     *   uint16_t length;
     *   uint8_t data[length];
     */
    unsigned char *serverinfo;
    size_t serverinfo_length;
};
/* Retrieve Suite B flags */
# define tls1_suiteb(s)  (s->cert->cert_flags & SSL_CERT_FLAG_SUITEB_128_LOS)
/* Uses to check strict mode: suite B modes are always strict */
# define SSL_CERT_FLAGS_CHECK_TLS_STRICT \
        (SSL_CERT_FLAG_SUITEB_128_LOS|SSL_CERT_FLAG_TLS_STRICT)

typedef enum {
    ENDPOINT_CLIENT = 0,
    ENDPOINT_SERVER,
    ENDPOINT_BOTH
} ENDPOINT;


typedef struct {
    unsigned short ext_type;
    ENDPOINT role;
    /* The context which this extension applies to */
    unsigned int context;
    /*
     * Per-connection flags relating to this extension type: not used if
     * part of an SSL_CTX structure.
     */
    uint32_t ext_flags;
    SSL_custom_ext_add_cb_ex add_cb;
    SSL_custom_ext_free_cb_ex free_cb;
    void *add_arg;
    SSL_custom_ext_parse_cb_ex parse_cb;
    void *parse_arg;
} custom_ext_method;

/* ext_flags values */

/*
 * Indicates an extension has been received. Used to check for unsolicited or
 * duplicate extensions.
 */
# define SSL_EXT_FLAG_RECEIVED   0x1
/*
 * Indicates an extension has been sent: used to enable sending of
 * corresponding ServerHello extension.
 */
# define SSL_EXT_FLAG_SENT       0x2

typedef struct {
    custom_ext_method *meths;
    size_t meths_count;
} custom_ext_methods;

typedef struct cert_st {
    /* Current active set */
    /*
     * ALWAYS points to an element of the pkeys array
     * Probably it would make more sense to store
     * an index, not a pointer.
     */
    CERT_PKEY *key;

    EVP_PKEY *dh_tmp;
    DH *(*dh_tmp_cb) (SSL *ssl, int is_export, int keysize);
    int dh_tmp_auto;
    /* Flags related to certificates */
    uint32_t cert_flags;
    CERT_PKEY pkeys[SSL_PKEY_NUM];
    /* Custom certificate types sent in certificate request message. */
    uint8_t *ctype;
    size_t ctype_len;
    /*
     * supported signature algorithms. When set on a client this is sent in
     * the client hello as the supported signature algorithms extension. For
     * servers it represents the signature algorithms we are willing to use.
     */
    uint16_t *conf_sigalgs;
    /* Size of above array */
    size_t conf_sigalgslen;
    /*
     * Client authentication signature algorithms, if not set then uses
     * conf_sigalgs. On servers these will be the signature algorithms sent
     * to the client in a certificate request for TLS 1.2. On a client this
     * represents the signature algorithms we are willing to use for client
     * authentication.
     */
    uint16_t *client_sigalgs;
    /* Size of above array */
    size_t client_sigalgslen;
    /*
     * Certificate setup callback: if set is called whenever a certificate
     * may be required (client or server). the callback can then examine any
     * appropriate parameters and setup any certificates required. This
     * allows advanced applications to select certificates on the fly: for
     * example based on supported signature algorithms or curves.
     */
    int (*cert_cb) (SSL *ssl, void *arg);
    void *cert_cb_arg;
    /*
     * Optional X509_STORE for chain building or certificate validation If
     * NULL the parent SSL_CTX store is used instead.
     */
    X509_STORE *chain_store;
    X509_STORE *verify_store;
    /* Custom extensions */
    custom_ext_methods custext;
    /* Security callback */
    int (*sec_cb) (const SSL *s, const SSL_CTX *ctx, int op, int bits, int nid,
                   void *other, void *ex);
    /* Security level */
    int sec_level;
    void *sec_ex;
# ifndef OPENSSL_NO_PSK
    /* If not NULL psk identity hint to use for servers */
    char *psk_identity_hint;
# endif
    CRYPTO_REF_COUNT references;             /* >1 only if SSL_copy_session_id is used */
    CRYPTO_RWLOCK *lock;
} CERT;

# define FP_ICC  (int (*)(const void *,const void *))

/*
 * This is for the SSLv3/TLSv1.0 differences in crypto/hash stuff It is a bit
 * of a mess of functions, but hell, think of it as an opaque structure :-)
 */
typedef struct ssl3_enc_method {
    int (*enc) (SSL *, SSL3_RECORD *, size_t, int, SSL_MAC_BUF *, size_t);
    int (*mac) (SSL *, SSL3_RECORD *, unsigned char *, int);
    int (*setup_key_block) (SSL *);
    int (*generate_master_secret) (SSL *, unsigned char *, unsigned char *,
                                   size_t, size_t *);
    int (*change_cipher_state) (SSL *, int);
    size_t (*final_finish_mac) (SSL *, const char *, size_t, unsigned char *);
    const char *client_finished_label;
    size_t client_finished_label_len;
    const char *server_finished_label;
    size_t server_finished_label_len;
    int (*alert_value) (int);
    int (*export_keying_material) (SSL *, unsigned char *, size_t,
                                   const char *, size_t,
                                   const unsigned char *, size_t,
                                   int use_context);
    /* Various flags indicating protocol version requirements */
    uint32_t enc_flags;
    /* Set the handshake header */
    int (*set_handshake_header) (SSL *s, WPACKET *pkt, int type);
    /* Close construction of the handshake message */
    int (*close_construct_packet) (SSL *s, WPACKET *pkt, int htype);
    /* Write out handshake message */
    int (*do_write) (SSL *s);
} SSL3_ENC_METHOD;

# define ssl_set_handshake_header(s, pkt, htype) \
        s->method->ssl3_enc->set_handshake_header((s), (pkt), (htype))
# define ssl_close_construct_packet(s, pkt, htype) \
        s->method->ssl3_enc->close_construct_packet((s), (pkt), (htype))
# define ssl_do_write(s)  s->method->ssl3_enc->do_write(s)

/* Values for enc_flags */

/* Uses explicit IV for CBC mode */
# define SSL_ENC_FLAG_EXPLICIT_IV        0x1
/* Uses signature algorithms extension */
# define SSL_ENC_FLAG_SIGALGS            0x2
/* Uses SHA256 default PRF */
# define SSL_ENC_FLAG_SHA256_PRF         0x4
/* Is DTLS */
# define SSL_ENC_FLAG_DTLS               0x8
/*
 * Allow TLS 1.2 ciphersuites: applies to DTLS 1.2 as well as TLS 1.2: may
 * apply to others in future.
 */
# define SSL_ENC_FLAG_TLS1_2_CIPHERS     0x10

# ifndef OPENSSL_NO_COMP
/* Used for holding the relevant compression methods loaded into SSL_CTX */
typedef struct ssl3_comp_st {
    int comp_id;                /* The identifier byte for this compression
                                 * type */
    char *name;                 /* Text name used for the compression type */
    COMP_METHOD *method;        /* The method :-) */
} SSL3_COMP;
# endif

typedef enum downgrade_en {
    DOWNGRADE_NONE,
    DOWNGRADE_TO_1_2,
    DOWNGRADE_TO_1_1
} DOWNGRADE;

/*
 * Dummy status type for the status_type extension. Indicates no status type
 * set
 */
#define TLSEXT_STATUSTYPE_nothing  -1

/* Sigalgs values */
#define TLSEXT_SIGALG_ecdsa_secp256r1_sha256                    0x0403
#define TLSEXT_SIGALG_ecdsa_secp384r1_sha384                    0x0503
#define TLSEXT_SIGALG_ecdsa_secp521r1_sha512                    0x0603
#define TLSEXT_SIGALG_ecdsa_sha224                              0x0303
#define TLSEXT_SIGALG_ecdsa_sha1                                0x0203
#define TLSEXT_SIGALG_rsa_pss_rsae_sha256                       0x0804
#define TLSEXT_SIGALG_rsa_pss_rsae_sha384                       0x0805
#define TLSEXT_SIGALG_rsa_pss_rsae_sha512                       0x0806
#define TLSEXT_SIGALG_rsa_pss_pss_sha256                        0x0809
#define TLSEXT_SIGALG_rsa_pss_pss_sha384                        0x080a
#define TLSEXT_SIGALG_rsa_pss_pss_sha512                        0x080b
#define TLSEXT_SIGALG_rsa_pkcs1_sha256                          0x0401
#define TLSEXT_SIGALG_rsa_pkcs1_sha384                          0x0501
#define TLSEXT_SIGALG_rsa_pkcs1_sha512                          0x0601
#define TLSEXT_SIGALG_rsa_pkcs1_sha224                          0x0301
#define TLSEXT_SIGALG_rsa_pkcs1_sha1                            0x0201
#define TLSEXT_SIGALG_dsa_sha256                                0x0402
#define TLSEXT_SIGALG_dsa_sha384                                0x0502
#define TLSEXT_SIGALG_dsa_sha512                                0x0602
#define TLSEXT_SIGALG_dsa_sha224                                0x0302
#define TLSEXT_SIGALG_dsa_sha1                                  0x0202
#define TLSEXT_SIGALG_gostr34102012_256_intrinsic               0x0840
#define TLSEXT_SIGALG_gostr34102012_512_intrinsic               0x0841
#define TLSEXT_SIGALG_gostr34102012_256_gostr34112012_256       0xeeee
#define TLSEXT_SIGALG_gostr34102012_512_gostr34112012_512       0xefef
#define TLSEXT_SIGALG_gostr34102001_gostr3411                   0xeded

#define TLSEXT_SIGALG_ed25519                                   0x0807
#define TLSEXT_SIGALG_ed448                                     0x0808

/* Known PSK key exchange modes */
#define TLSEXT_KEX_MODE_KE                                      0x00
#define TLSEXT_KEX_MODE_KE_DHE                                  0x01

/*
 * Internal representations of key exchange modes
 */
#define TLSEXT_KEX_MODE_FLAG_NONE                               0
#define TLSEXT_KEX_MODE_FLAG_KE                                 1
#define TLSEXT_KEX_MODE_FLAG_KE_DHE                             2

#define SSL_USE_PSS(s) (s->s3.tmp.peer_sigalg != NULL && \
                        s->s3.tmp.peer_sigalg->sig == EVP_PKEY_RSA_PSS)

/* A dummy signature value not valid for TLSv1.2 signature algs */
#define TLSEXT_signature_rsa_pss                                0x0101

/* TLSv1.3 downgrade protection sentinel values */
extern const unsigned char tls11downgrade[8];
extern const unsigned char tls12downgrade[8];

extern SSL3_ENC_METHOD ssl3_undef_enc_method;

__owur const SSL_METHOD *ssl_bad_method(int ver);
__owur const SSL_METHOD *sslv3_method(void);
__owur const SSL_METHOD *sslv3_server_method(void);
__owur const SSL_METHOD *sslv3_client_method(void);
__owur const SSL_METHOD *tlsv1_method(void);
__owur const SSL_METHOD *tlsv1_server_method(void);
__owur const SSL_METHOD *tlsv1_client_method(void);
__owur const SSL_METHOD *tlsv1_1_method(void);
__owur const SSL_METHOD *tlsv1_1_server_method(void);
__owur const SSL_METHOD *tlsv1_1_client_method(void);
__owur const SSL_METHOD *tlsv1_2_method(void);
__owur const SSL_METHOD *tlsv1_2_server_method(void);
__owur const SSL_METHOD *tlsv1_2_client_method(void);
__owur const SSL_METHOD *tlsv1_3_method(void);
__owur const SSL_METHOD *tlsv1_3_server_method(void);
__owur const SSL_METHOD *tlsv1_3_client_method(void);
__owur const SSL_METHOD *dtlsv1_method(void);
__owur const SSL_METHOD *dtlsv1_server_method(void);
__owur const SSL_METHOD *dtlsv1_client_method(void);
__owur const SSL_METHOD *dtls_bad_ver_client_method(void);
__owur const SSL_METHOD *dtlsv1_2_method(void);
__owur const SSL_METHOD *dtlsv1_2_server_method(void);
__owur const SSL_METHOD *dtlsv1_2_client_method(void);

extern const SSL3_ENC_METHOD TLSv1_enc_data;
extern const SSL3_ENC_METHOD TLSv1_1_enc_data;
extern const SSL3_ENC_METHOD TLSv1_2_enc_data;
extern const SSL3_ENC_METHOD TLSv1_3_enc_data;
extern const SSL3_ENC_METHOD SSLv3_enc_data;
extern const SSL3_ENC_METHOD DTLSv1_enc_data;
extern const SSL3_ENC_METHOD DTLSv1_2_enc_data;

/*
 * Flags for SSL methods
 */
# define SSL_METHOD_NO_FIPS      (1U<<0)
# define SSL_METHOD_NO_SUITEB    (1U<<1)

# define IMPLEMENT_tls_meth_func(version, flags, mask, func_name, s_accept, \
                                 s_connect, enc_data) \
const SSL_METHOD *func_name(void)  \
        { \
        static const SSL_METHOD func_name##_data= { \
                version, \
                flags, \
                mask, \
                tls1_new, \
                tls1_clear, \
                tls1_free, \
                s_accept, \
                s_connect, \
                ssl3_read, \
                ssl3_peek, \
                ssl3_write, \
                ssl3_shutdown, \
                ssl3_renegotiate, \
                ssl3_renegotiate_check, \
                ssl3_read_bytes, \
                ssl3_write_bytes, \
                ssl3_dispatch_alert, \
                ssl3_ctrl, \
                ssl3_ctx_ctrl, \
                ssl3_get_cipher_by_char, \
                ssl3_put_cipher_by_char, \
                ssl3_pending, \
                ssl3_num_ciphers, \
                ssl3_get_cipher, \
                tls1_default_timeout, \
                &enc_data, \
                ssl_undefined_void_function, \
                ssl3_callback_ctrl, \
                ssl3_ctx_callback_ctrl, \
        }; \
        return &func_name##_data; \
        }

# define IMPLEMENT_ssl3_meth_func(func_name, s_accept, s_connect) \
const SSL_METHOD *func_name(void)  \
        { \
        static const SSL_METHOD func_name##_data= { \
                SSL3_VERSION, \
                SSL_METHOD_NO_FIPS | SSL_METHOD_NO_SUITEB, \
                SSL_OP_NO_SSLv3, \
                ssl3_new, \
                ssl3_clear, \
                ssl3_free, \
                s_accept, \
                s_connect, \
                ssl3_read, \
                ssl3_peek, \
                ssl3_write, \
                ssl3_shutdown, \
                ssl3_renegotiate, \
                ssl3_renegotiate_check, \
                ssl3_read_bytes, \
                ssl3_write_bytes, \
                ssl3_dispatch_alert, \
                ssl3_ctrl, \
                ssl3_ctx_ctrl, \
                ssl3_get_cipher_by_char, \
                ssl3_put_cipher_by_char, \
                ssl3_pending, \
                ssl3_num_ciphers, \
                ssl3_get_cipher, \
                ssl3_default_timeout, \
                &SSLv3_enc_data, \
                ssl_undefined_void_function, \
                ssl3_callback_ctrl, \
                ssl3_ctx_callback_ctrl, \
        }; \
        return &func_name##_data; \
        }

# define IMPLEMENT_dtls1_meth_func(version, flags, mask, func_name, s_accept, \
                                        s_connect, enc_data) \
const SSL_METHOD *func_name(void)  \
        { \
        static const SSL_METHOD func_name##_data= { \
                version, \
                flags, \
                mask, \
                dtls1_new, \
                dtls1_clear, \
                dtls1_free, \
                s_accept, \
                s_connect, \
                ssl3_read, \
                ssl3_peek, \
                ssl3_write, \
                dtls1_shutdown, \
                ssl3_renegotiate, \
                ssl3_renegotiate_check, \
                dtls1_read_bytes, \
                dtls1_write_app_data_bytes, \
                dtls1_dispatch_alert, \
                dtls1_ctrl, \
                ssl3_ctx_ctrl, \
                ssl3_get_cipher_by_char, \
                ssl3_put_cipher_by_char, \
                ssl3_pending, \
                ssl3_num_ciphers, \
                ssl3_get_cipher, \
                dtls1_default_timeout, \
                &enc_data, \
                ssl_undefined_void_function, \
                ssl3_callback_ctrl, \
                ssl3_ctx_callback_ctrl, \
        }; \
        return &func_name##_data; \
        }

struct openssl_ssl_test_functions {
    int (*p_ssl_init_wbio_buffer) (SSL *s);
    int (*p_ssl3_setup_buffers) (SSL *s);
};

const char *ssl_protocol_to_string(int version);

/* Returns true if certificate and private key for 'idx' are present */
static ossl_inline int ssl_has_cert(const SSL *s, int idx)
{
    if (idx < 0 || idx >= SSL_PKEY_NUM)
        return 0;
    return s->cert->pkeys[idx].x509 != NULL
        && s->cert->pkeys[idx].privatekey != NULL;
}

static ossl_inline void tls1_get_peer_groups(SSL *s, const uint16_t **pgroups,
                                             size_t *pgroupslen)
{
    *pgroups = s->ext.peer_supportedgroups;
    *pgroupslen = s->ext.peer_supportedgroups_len;
}

# ifndef OPENSSL_UNIT_TEST

__owur int ssl_read_internal(SSL *s, void *buf, size_t num, size_t *readbytes);
__owur int ssl_write_internal(SSL *s, const void *buf, size_t num, size_t *written);
void ssl_clear_cipher_ctx(SSL *s);
int ssl_clear_bad_session(SSL *s);
__owur CERT *ssl_cert_new(void);
__owur CERT *ssl_cert_dup(CERT *cert);
void ssl_cert_clear_certs(CERT *c);
void ssl_cert_free(CERT *c);
__owur int ssl_generate_session_id(SSL *s, SSL_SESSION *ss);
__owur int ssl_get_new_session(SSL *s, int session);
__owur SSL_SESSION *lookup_sess_in_cache(SSL *s, const unsigned char *sess_id,
                                         size_t sess_id_len);
__owur int ssl_get_prev_session(SSL *s, CLIENTHELLO_MSG *hello);
__owur SSL_SESSION *ssl_session_dup(const SSL_SESSION *src, int ticket);
__owur int ssl_cipher_id_cmp(const SSL_CIPHER *a, const SSL_CIPHER *b);
DECLARE_OBJ_BSEARCH_GLOBAL_CMP_FN(SSL_CIPHER, SSL_CIPHER, ssl_cipher_id);
__owur int ssl_cipher_ptr_id_cmp(const SSL_CIPHER *const *ap,
                                 const SSL_CIPHER *const *bp);
__owur STACK_OF(SSL_CIPHER) *ssl_create_cipher_list(SSL_CTX *ctx,
                                                    STACK_OF(SSL_CIPHER) *tls13_ciphersuites,
                                                    STACK_OF(SSL_CIPHER) **cipher_list,
                                                    STACK_OF(SSL_CIPHER) **cipher_list_by_id,
                                                    const char *rule_str,
                                                    CERT *c);
__owur int ssl_cache_cipherlist(SSL *s, PACKET *cipher_suites, int sslv2format);
__owur int bytes_to_cipher_list(SSL *s, PACKET *cipher_suites,
                                STACK_OF(SSL_CIPHER) **skp,
                                STACK_OF(SSL_CIPHER) **scsvs, int sslv2format,
                                int fatal);
void ssl_update_cache(SSL *s, int mode);
__owur int ssl_cipher_get_evp_cipher(SSL_CTX *ctx, const SSL_CIPHER *sslc,
                                     const EVP_CIPHER **enc);
__owur int ssl_cipher_get_evp(SSL_CTX *ctxc, const SSL_SESSION *s,
                              const EVP_CIPHER **enc, const EVP_MD **md,
                              int *mac_pkey_type, size_t *mac_secret_size,
                              SSL_COMP **comp, int use_etm);
__owur int ssl_cipher_get_overhead(const SSL_CIPHER *c, size_t *mac_overhead,
                                   size_t *int_overhead, size_t *blocksize,
                                   size_t *ext_overhead);
__owur int ssl_cert_is_disabled(SSL_CTX *ctx, size_t idx);
__owur const SSL_CIPHER *ssl_get_cipher_by_char(SSL *ssl,
                                                const unsigned char *ptr,
                                                int all);
__owur int ssl_cert_set0_chain(SSL *s, SSL_CTX *ctx, STACK_OF(X509) *chain);
__owur int ssl_cert_set1_chain(SSL *s, SSL_CTX *ctx, STACK_OF(X509) *chain);
__owur int ssl_cert_add0_chain_cert(SSL *s, SSL_CTX *ctx, X509 *x);
__owur int ssl_cert_add1_chain_cert(SSL *s, SSL_CTX *ctx, X509 *x);
__owur int ssl_cert_select_current(CERT *c, X509 *x);
__owur int ssl_cert_set_current(CERT *c, long arg);
void ssl_cert_set_cert_cb(CERT *c, int (*cb) (SSL *ssl, void *arg), void *arg);

__owur int ssl_verify_cert_chain(SSL *s, STACK_OF(X509) *sk);
__owur int ssl_build_cert_chain(SSL *s, SSL_CTX *ctx, int flags);
__owur int ssl_cert_set_cert_store(CERT *c, X509_STORE *store, int chain,
                                   int ref);
__owur int ssl_cert_get_cert_store(CERT *c, X509_STORE **pstore, int chain);

__owur int ssl_security(const SSL *s, int op, int bits, int nid, void *other);
__owur int ssl_ctx_security(const SSL_CTX *ctx, int op, int bits, int nid,
                            void *other);
int ssl_get_security_level_bits(const SSL *s, const SSL_CTX *ctx, int *levelp);

__owur int ssl_cert_lookup_by_nid(int nid, size_t *pidx);
__owur const SSL_CERT_LOOKUP *ssl_cert_lookup_by_pkey(const EVP_PKEY *pk,
                                                      size_t *pidx);
__owur const SSL_CERT_LOOKUP *ssl_cert_lookup_by_idx(size_t idx);

int ssl_undefined_function(SSL *s);
__owur int ssl_undefined_void_function(void);
__owur int ssl_undefined_const_function(const SSL *s);
__owur int ssl_get_server_cert_serverinfo(SSL *s,
                                          const unsigned char **serverinfo,
                                          size_t *serverinfo_length);
void ssl_set_masks(SSL *s);
__owur STACK_OF(SSL_CIPHER) *ssl_get_ciphers_by_id(SSL *s);
__owur int ssl_x509err2alert(int type);
void ssl_sort_cipher_list(void);
int ssl_load_ciphers(SSL_CTX *ctx);
__owur int ssl_setup_sig_algs(SSL_CTX *ctx);
int ssl_load_groups(SSL_CTX *ctx);
__owur int ssl_fill_hello_random(SSL *s, int server, unsigned char *field,
                                 size_t len, DOWNGRADE dgrd);
__owur int ssl_generate_master_secret(SSL *s, unsigned char *pms, size_t pmslen,
                                      int free_pms);
__owur EVP_PKEY *ssl_generate_pkey(SSL *s, EVP_PKEY *pm);
__owur int ssl_gensecret(SSL *s, unsigned char *pms, size_t pmslen);
__owur int ssl_derive(SSL *s, EVP_PKEY *privkey, EVP_PKEY *pubkey,
                      int genmaster);
__owur int ssl_decapsulate(SSL *s, EVP_PKEY *privkey,
                           const unsigned char *ct, size_t ctlen,
                           int gensecret);
__owur int ssl_encapsulate(SSL *s, EVP_PKEY *pubkey,
                           unsigned char **ctp, size_t *ctlenp,
                           int gensecret);
__owur EVP_PKEY *ssl_dh_to_pkey(DH *dh);
__owur int ssl_set_tmp_ecdh_groups(uint16_t **pext, size_t *pextlen,
                                   void *key);
__owur unsigned int ssl_get_max_send_fragment(const SSL *ssl);
__owur unsigned int ssl_get_split_send_fragment(const SSL *ssl);

__owur const SSL_CIPHER *ssl3_get_cipher_by_id(uint32_t id);
__owur const SSL_CIPHER *ssl3_get_cipher_by_std_name(const char *stdname);
__owur const SSL_CIPHER *ssl3_get_cipher_by_char(const unsigned char *p);
__owur int ssl3_put_cipher_by_char(const SSL_CIPHER *c, WPACKET *pkt,
                                   size_t *len);
int ssl3_init_finished_mac(SSL *s);
__owur int ssl3_setup_key_block(SSL *s);
__owur int ssl3_change_cipher_state(SSL *s, int which);
void ssl3_cleanup_key_block(SSL *s);
__owur int ssl3_do_write(SSL *s, int type);
int ssl3_send_alert(SSL *s, int level, int desc);
__owur int ssl3_generate_master_secret(SSL *s, unsigned char *out,
                                       unsigned char *p, size_t len,
                                       size_t *secret_size);
__owur int ssl3_get_req_cert_type(SSL *s, WPACKET *pkt);
__owur int ssl3_num_ciphers(void);
__owur const SSL_CIPHER *ssl3_get_cipher(unsigned int u);
int ssl3_renegotiate(SSL *ssl);
int ssl3_renegotiate_check(SSL *ssl, int initok);
void ssl3_digest_master_key_set_params(const SSL_SESSION *session,
                                       OSSL_PARAM params[]);
__owur int ssl3_dispatch_alert(SSL *s);
__owur size_t ssl3_final_finish_mac(SSL *s, const char *sender, size_t slen,
                                    unsigned char *p);
__owur int ssl3_finish_mac(SSL *s, const unsigned char *buf, size_t len);
void ssl3_free_digest_list(SSL *s);
__owur unsigned long ssl3_output_cert_chain(SSL *s, WPACKET *pkt,
                                            CERT_PKEY *cpk);
__owur const SSL_CIPHER *ssl3_choose_cipher(SSL *ssl,
                                            STACK_OF(SSL_CIPHER) *clnt,
                                            STACK_OF(SSL_CIPHER) *srvr);
__owur int ssl3_digest_cached_records(SSL *s, int keep);
__owur int ssl3_new(SSL *s);
void ssl3_free(SSL *s);
__owur int ssl3_read(SSL *s, void *buf, size_t len, size_t *readbytes);
__owur int ssl3_peek(SSL *s, void *buf, size_t len, size_t *readbytes);
__owur int ssl3_write(SSL *s, const void *buf, size_t len, size_t *written);
__owur int ssl3_shutdown(SSL *s);
int ssl3_clear(SSL *s);
__owur long ssl3_ctrl(SSL *s, int cmd, long larg, void *parg);
__owur long ssl3_ctx_ctrl(SSL_CTX *s, int cmd, long larg, void *parg);
__owur long ssl3_callback_ctrl(SSL *s, int cmd, void (*fp) (void));
__owur long ssl3_ctx_callback_ctrl(SSL_CTX *s, int cmd, void (*fp) (void));

__owur int ssl3_do_change_cipher_spec(SSL *ssl);
__owur long ssl3_default_timeout(void);

__owur int ssl3_set_handshake_header(SSL *s, WPACKET *pkt, int htype);
__owur int tls_close_construct_packet(SSL *s, WPACKET *pkt, int htype);
__owur int tls_setup_handshake(SSL *s);
__owur int dtls1_set_handshake_header(SSL *s, WPACKET *pkt, int htype);
__owur int dtls1_close_construct_packet(SSL *s, WPACKET *pkt, int htype);
__owur int ssl3_handshake_write(SSL *s);

__owur int ssl_allow_compression(SSL *s);

__owur int ssl_version_supported(const SSL *s, int version,
                                 const SSL_METHOD **meth);

__owur int ssl_set_client_hello_version(SSL *s);
__owur int ssl_check_version_downgrade(SSL *s);
__owur int ssl_set_version_bound(int method_version, int version, int *bound);
__owur int ssl_choose_server_version(SSL *s, CLIENTHELLO_MSG *hello,
                                     DOWNGRADE *dgrd);
__owur int ssl_choose_client_version(SSL *s, int version,
                                     RAW_EXTENSION *extensions);
__owur int ssl_get_min_max_version(const SSL *s, int *min_version,
                                   int *max_version, int *real_max);

__owur long tls1_default_timeout(void);
__owur int dtls1_do_write(SSL *s, int type);
void dtls1_set_message_header(SSL *s,
                              unsigned char mt,
                              size_t len,
                              size_t frag_off, size_t frag_len);

int dtls1_write_app_data_bytes(SSL *s, int type, const void *buf_, size_t len,
                               size_t *written);

__owur int dtls1_read_failed(SSL *s, int code);
__owur int dtls1_buffer_message(SSL *s, int ccs);
__owur int dtls1_retransmit_message(SSL *s, unsigned short seq, int *found);
__owur int dtls1_get_queue_priority(unsigned short seq, int is_ccs);
int dtls1_retransmit_buffered_messages(SSL *s);
void dtls1_clear_received_buffer(SSL *s);
void dtls1_clear_sent_buffer(SSL *s);
void dtls1_get_message_header(unsigned char *data,
                              struct hm_header_st *msg_hdr);
__owur long dtls1_default_timeout(void);
__owur struct timeval *dtls1_get_timeout(SSL *s, struct timeval *timeleft);
__owur int dtls1_check_timeout_num(SSL *s);
__owur int dtls1_handle_timeout(SSL *s);
void dtls1_start_timer(SSL *s);
void dtls1_stop_timer(SSL *s);
__owur int dtls1_is_timer_expired(SSL *s);
__owur int dtls_raw_hello_verify_request(WPACKET *pkt, unsigned char *cookie,
                                         size_t cookie_len);
__owur size_t dtls1_min_mtu(SSL *s);
void dtls1_hm_fragment_free(hm_fragment *frag);
__owur int dtls1_query_mtu(SSL *s);

__owur int tls1_new(SSL *s);
void tls1_free(SSL *s);
int tls1_clear(SSL *s);

__owur int dtls1_new(SSL *s);
void dtls1_free(SSL *s);
int dtls1_clear(SSL *s);
long dtls1_ctrl(SSL *s, int cmd, long larg, void *parg);
__owur int dtls1_shutdown(SSL *s);

__owur int dtls1_dispatch_alert(SSL *s);

__owur int ssl_init_wbio_buffer(SSL *s);
int ssl_free_wbio_buffer(SSL *s);

__owur int tls1_change_cipher_state(SSL *s, int which);
__owur int tls1_setup_key_block(SSL *s);
__owur size_t tls1_final_finish_mac(SSL *s, const char *str, size_t slen,
                                    unsigned char *p);
__owur int tls1_generate_master_secret(SSL *s, unsigned char *out,
                                       unsigned char *p, size_t len,
                                       size_t *secret_size);
__owur int tls13_setup_key_block(SSL *s);
__owur size_t tls13_final_finish_mac(SSL *s, const char *str, size_t slen,
                                     unsigned char *p);
__owur int tls13_change_cipher_state(SSL *s, int which);
__owur int tls13_update_key(SSL *s, int send);
__owur int tls13_hkdf_expand(SSL *s, const EVP_MD *md,
                             const unsigned char *secret,
                             const unsigned char *label, size_t labellen,
                             const unsigned char *data, size_t datalen,
                             unsigned char *out, size_t outlen, int fatal);
__owur int tls13_derive_key(SSL *s, const EVP_MD *md,
                            const unsigned char *secret, unsigned char *key,
                            size_t keylen);
__owur int tls13_derive_iv(SSL *s, const EVP_MD *md,
                           const unsigned char *secret, unsigned char *iv,
                           size_t ivlen);
__owur int tls13_derive_finishedkey(SSL *s, const EVP_MD *md,
                                    const unsigned char *secret,
                                    unsigned char *fin, size_t finlen);
int tls13_generate_secret(SSL *s, const EVP_MD *md,
                          const unsigned char *prevsecret,
                          const unsigned char *insecret,
                          size_t insecretlen,
                          unsigned char *outsecret);
__owur int tls13_generate_handshake_secret(SSL *s,
                                           const unsigned char *insecret,
                                           size_t insecretlen);
__owur int tls13_generate_master_secret(SSL *s, unsigned char *out,
                                        unsigned char *prev, size_t prevlen,
                                        size_t *secret_size);
__owur int tls1_export_keying_material(SSL *s, unsigned char *out, size_t olen,
                                       const char *label, size_t llen,
                                       const unsigned char *p, size_t plen,
                                       int use_context);
__owur int tls13_export_keying_material(SSL *s, unsigned char *out, size_t olen,
                                        const char *label, size_t llen,
                                        const unsigned char *context,
                                        size_t contextlen, int use_context);
__owur int tls13_export_keying_material_early(SSL *s, unsigned char *out,
                                              size_t olen, const char *label,
                                              size_t llen,
                                              const unsigned char *context,
                                              size_t contextlen);
__owur int tls1_alert_code(int code);
__owur int tls13_alert_code(int code);
__owur int ssl3_alert_code(int code);

__owur int ssl_check_srvr_ecc_cert_and_alg(X509 *x, SSL *s);

SSL_COMP *ssl3_comp_find(STACK_OF(SSL_COMP) *sk, int n);

__owur const TLS_GROUP_INFO *tls1_group_id_lookup(SSL_CTX *ctx, uint16_t curve_id);
__owur int tls1_group_id2nid(uint16_t group_id, int include_unknown);
__owur uint16_t tls1_nid2group_id(int nid);
__owur int tls1_check_group_id(SSL *s, uint16_t group_id, int check_own_curves);
__owur uint16_t tls1_shared_group(SSL *s, int nmatch);
__owur int tls1_set_groups(uint16_t **pext, size_t *pextlen,
                           int *curves, size_t ncurves);
__owur int tls1_set_groups_list(SSL_CTX *ctx, uint16_t **pext, size_t *pextlen,
                                const char *str);
__owur EVP_PKEY *ssl_generate_pkey_group(SSL *s, uint16_t id);
__owur int tls_valid_group(SSL *s, uint16_t group_id, int minversion,
                           int maxversion, int isec, int *okfortls13);
__owur EVP_PKEY *ssl_generate_param_group(SSL *s, uint16_t id);
void tls1_get_formatlist(SSL *s, const unsigned char **pformats,
                         size_t *num_formats);
__owur int tls1_check_ec_tmp_key(SSL *s, unsigned long id);

__owur int tls_group_allowed(SSL *s, uint16_t curve, int op);
void tls1_get_supported_groups(SSL *s, const uint16_t **pgroups,
                               size_t *pgroupslen);

__owur int tls1_set_server_sigalgs(SSL *s);

__owur SSL_TICKET_STATUS tls_get_ticket_from_client(SSL *s, CLIENTHELLO_MSG *hello,
                                                    SSL_SESSION **ret);
__owur SSL_TICKET_STATUS tls_decrypt_ticket(SSL *s, const unsigned char *etick,
                                            size_t eticklen,
                                            const unsigned char *sess_id,
                                            size_t sesslen, SSL_SESSION **psess);

__owur int tls_use_ticket(SSL *s);

void ssl_set_sig_mask(uint32_t *pmask_a, SSL *s, int op);

__owur int tls1_set_sigalgs_list(CERT *c, const char *str, int client);
__owur int tls1_set_raw_sigalgs(CERT *c, const uint16_t *psigs, size_t salglen,
                                int client);
__owur int tls1_set_sigalgs(CERT *c, const int *salg, size_t salglen,
                            int client);
int tls1_check_chain(SSL *s, X509 *x, EVP_PKEY *pk, STACK_OF(X509) *chain,
                     int idx);
void tls1_set_cert_validity(SSL *s);

#  ifndef OPENSSL_NO_CT
__owur int ssl_validate_ct(SSL *s);
#  endif

__owur EVP_PKEY *ssl_get_auto_dh(SSL *s);

__owur int ssl_security_cert(SSL *s, SSL_CTX *ctx, X509 *x, int vfy, int is_ee);
__owur int ssl_security_cert_chain(SSL *s, STACK_OF(X509) *sk, X509 *ex,
                                   int vfy);

int tls_choose_sigalg(SSL *s, int fatalerrs);

__owur EVP_MD_CTX *ssl_replace_hash(EVP_MD_CTX **hash, const EVP_MD *md);
void ssl_clear_hash_ctx(EVP_MD_CTX **hash);
__owur long ssl_get_algorithm2(SSL *s);
__owur int tls12_copy_sigalgs(SSL *s, WPACKET *pkt,
                              const uint16_t *psig, size_t psiglen);
__owur int tls1_save_u16(PACKET *pkt, uint16_t **pdest, size_t *pdestlen);
__owur int tls1_save_sigalgs(SSL *s, PACKET *pkt, int cert);
__owur int tls1_process_sigalgs(SSL *s);
__owur int tls1_set_peer_legacy_sigalg(SSL *s, const EVP_PKEY *pkey);
__owur int tls1_lookup_md(SSL_CTX *ctx, const SIGALG_LOOKUP *lu,
                          const EVP_MD **pmd);
__owur size_t tls12_get_psigalgs(SSL *s, int sent, const uint16_t **psigs);
__owur int tls_check_sigalg_curve(const SSL *s, int curve);
__owur int tls12_check_peer_sigalg(SSL *s, uint16_t, EVP_PKEY *pkey);
__owur int ssl_set_client_disabled(SSL *s);
__owur int ssl_cipher_disabled(const SSL *s, const SSL_CIPHER *c, int op, int echde);

__owur int ssl_handshake_hash(SSL *s, unsigned char *out, size_t outlen,
                                 size_t *hashlen);
__owur const EVP_MD *ssl_md(SSL_CTX *ctx, int idx);
__owur const EVP_MD *ssl_handshake_md(SSL *s);
__owur const EVP_MD *ssl_prf_md(SSL *s);

/*
 * ssl_log_rsa_client_key_exchange logs |premaster| to the SSL_CTX associated
 * with |ssl|, if logging is enabled. It returns one on success and zero on
 * failure. The entry is identified by the first 8 bytes of
 * |encrypted_premaster|.
 */
__owur int ssl_log_rsa_client_key_exchange(SSL *ssl,
                                           const uint8_t *encrypted_premaster,
                                           size_t encrypted_premaster_len,
                                           const uint8_t *premaster,
                                           size_t premaster_len);

/*
 * ssl_log_secret logs |secret| to the SSL_CTX associated with |ssl|, if
 * logging is available. It returns one on success and zero on failure. It tags
 * the entry with |label|.
 */
__owur int ssl_log_secret(SSL *ssl, const char *label,
                          const uint8_t *secret, size_t secret_len);

#define MASTER_SECRET_LABEL "CLIENT_RANDOM"
#define CLIENT_EARLY_LABEL "CLIENT_EARLY_TRAFFIC_SECRET"
#define CLIENT_HANDSHAKE_LABEL "CLIENT_HANDSHAKE_TRAFFIC_SECRET"
#define SERVER_HANDSHAKE_LABEL "SERVER_HANDSHAKE_TRAFFIC_SECRET"
#define CLIENT_APPLICATION_LABEL "CLIENT_TRAFFIC_SECRET_0"
#define SERVER_APPLICATION_LABEL "SERVER_TRAFFIC_SECRET_0"
#define EARLY_EXPORTER_SECRET_LABEL "EARLY_EXPORTER_SECRET"
#define EXPORTER_SECRET_LABEL "EXPORTER_SECRET"

#  ifndef OPENSSL_NO_KTLS
/* ktls.c */
int ktls_check_supported_cipher(const SSL *s, const EVP_CIPHER *c,
                                const EVP_CIPHER_CTX *dd);
int ktls_configure_crypto(const SSL *s, const EVP_CIPHER *c, EVP_CIPHER_CTX *dd,
                          void *rl_sequence, ktls_crypto_info_t *crypto_info,
                          unsigned char **rec_seq, unsigned char *iv,
                          unsigned char *key, unsigned char *mac_key,
                          size_t mac_secret_size);
#  endif

/* s3_cbc.c */
__owur char ssl3_cbc_record_digest_supported(const EVP_MD_CTX *ctx);
__owur int ssl3_cbc_digest_record(const EVP_MD *md,
                                  unsigned char *md_out,
                                  size_t *md_out_size,
                                  const unsigned char *header,
                                  const unsigned char *data,
                                  size_t data_size,
                                  size_t data_plus_mac_plus_padding_size,
                                  const unsigned char *mac_secret,
                                  size_t mac_secret_length, char is_sslv3);

__owur int srp_generate_server_master_secret(SSL *s);
__owur int srp_generate_client_master_secret(SSL *s);
__owur int srp_verify_server_param(SSL *s);

/* statem/statem_srvr.c */

__owur int send_certificate_request(SSL *s);

/* statem/extensions_cust.c */

custom_ext_method *custom_ext_find(const custom_ext_methods *exts,
                                   ENDPOINT role, unsigned int ext_type,
                                   size_t *idx);

void custom_ext_init(custom_ext_methods *meths);

__owur int custom_ext_parse(SSL *s, unsigned int context, unsigned int ext_type,
                            const unsigned char *ext_data, size_t ext_size,
                            X509 *x, size_t chainidx);
__owur int custom_ext_add(SSL *s, int context, WPACKET *pkt, X509 *x,
                          size_t chainidx, int maxversion);

__owur int custom_exts_copy(custom_ext_methods *dst,
                            const custom_ext_methods *src);
__owur int custom_exts_copy_flags(custom_ext_methods *dst,
                                  const custom_ext_methods *src);
void custom_exts_free(custom_ext_methods *exts);

void ssl_comp_free_compression_methods_int(void);

#ifndef OPENSSL_NO_QUIC
__owur int SSL_clear_not_quic(SSL *s);
__owur int SSL_clear_quic(SSL *s);
#endif

/* ssl_mcnf.c */
void ssl_ctx_system_config(SSL_CTX *ctx);

const EVP_CIPHER *ssl_evp_cipher_fetch(OSSL_LIB_CTX *libctx,
                                       int nid,
                                       const char *properties);
int ssl_evp_cipher_up_ref(const EVP_CIPHER *cipher);
void ssl_evp_cipher_free(const EVP_CIPHER *cipher);
const EVP_MD *ssl_evp_md_fetch(OSSL_LIB_CTX *libctx,
                               int nid,
                               const char *properties);
int ssl_evp_md_up_ref(const EVP_MD *md);
void ssl_evp_md_free(const EVP_MD *md);

int tls_provider_set_tls_params(SSL *s, EVP_CIPHER_CTX *ctx,
                                const EVP_CIPHER *ciph,
                                const EVP_MD *md);

void tls_engine_finish(ENGINE *e);
const EVP_CIPHER *tls_get_cipher_from_engine(int nid);
const EVP_MD *tls_get_digest_from_engine(int nid);
int tls_engine_load_ssl_client_cert(SSL *s, X509 **px509, EVP_PKEY **ppkey);
int ssl_hmac_old_new(SSL_HMAC *ret);
void ssl_hmac_old_free(SSL_HMAC *ctx);
int ssl_hmac_old_init(SSL_HMAC *ctx, void *key, size_t len, char *md);
int ssl_hmac_old_update(SSL_HMAC *ctx, const unsigned char *data, size_t len);
int ssl_hmac_old_final(SSL_HMAC *ctx, unsigned char *md, size_t *len);
size_t ssl_hmac_old_size(const SSL_HMAC *ctx);

int ssl_ctx_srp_ctx_free_intern(SSL_CTX *ctx);
int ssl_ctx_srp_ctx_init_intern(SSL_CTX *ctx);
int ssl_srp_ctx_free_intern(SSL *s);
int ssl_srp_ctx_init_intern(SSL *s);

int ssl_srp_calc_a_param_intern(SSL *s);
int ssl_srp_server_param_with_username_intern(SSL *s, int *ad);

void ssl_session_calculate_timeout(SSL_SESSION* ss);

# else /* OPENSSL_UNIT_TEST */

#  define ssl_init_wbio_buffer SSL_test_functions()->p_ssl_init_wbio_buffer
#  define ssl3_setup_buffers SSL_test_functions()->p_ssl3_setup_buffers

# endif

/* Some helper routines to support TSAN operations safely */
static ossl_unused ossl_inline int ssl_tsan_lock(const SSL_CTX *ctx)
{
#ifdef TSAN_REQUIRES_LOCKING
    if (!CRYPTO_THREAD_write_lock(ctx->tsan_lock))
        return 0;
#endif
    return 1;
}

static ossl_unused ossl_inline void ssl_tsan_unlock(const SSL_CTX *ctx)
{
#ifdef TSAN_REQUIRES_LOCKING
    CRYPTO_THREAD_unlock(ctx->tsan_lock);
#endif
}

static ossl_unused ossl_inline void ssl_tsan_counter(const SSL_CTX *ctx,
                                                     TSAN_QUALIFIER int *stat)
{
    if (ssl_tsan_lock(ctx)) {
        tsan_counter(stat);
        ssl_tsan_unlock(ctx);
    }
}

#endif
{
    *pgroups = s->ext.peer_supportedgroups;
    *pgroupslen = s->ext.peer_supportedgroups_len;
}

# ifndef OPENSSL_UNIT_TEST

__owur int ssl_read_internal(SSL *s, void *buf, size_t num, size_t *readbytes);
__owur int ssl_write_internal(SSL *s, const void *buf, size_t num, size_t *written);
void ssl_clear_cipher_ctx(SSL *s);
int ssl_clear_bad_session(SSL *s);
__owur CERT *ssl_cert_new(void);
__owur CERT *ssl_cert_dup(CERT *cert);
void ssl_cert_clear_certs(CERT *c);
void ssl_cert_free(CERT *c);
__owur int ssl_generate_session_id(SSL *s, SSL_SESSION *ss);
__owur int ssl_get_new_session(SSL *s, int session);
__owur SSL_SESSION *lookup_sess_in_cache(SSL *s, const unsigned char *sess_id,
                                         size_t sess_id_len);
__owur int ssl_get_prev_session(SSL *s, CLIENTHELLO_MSG *hello);
__owur SSL_SESSION *ssl_session_dup(const SSL_SESSION *src, int ticket);
__owur int ssl_cipher_id_cmp(const SSL_CIPHER *a, const SSL_CIPHER *b);
DECLARE_OBJ_BSEARCH_GLOBAL_CMP_FN(SSL_CIPHER, SSL_CIPHER, ssl_cipher_id);
__owur int ssl_cipher_ptr_id_cmp(const SSL_CIPHER *const *ap,
                                 const SSL_CIPHER *const *bp);
__owur STACK_OF(SSL_CIPHER) *ssl_create_cipher_list(SSL_CTX *ctx,
                                                    STACK_OF(SSL_CIPHER) *tls13_ciphersuites,
                                                    STACK_OF(SSL_CIPHER) **cipher_list,
                                                    STACK_OF(SSL_CIPHER) **cipher_list_by_id,
                                                    const char *rule_str,
                                                    CERT *c);
__owur int ssl_cache_cipherlist(SSL *s, PACKET *cipher_suites, int sslv2format);
__owur int bytes_to_cipher_list(SSL *s, PACKET *cipher_suites,
                                STACK_OF(SSL_CIPHER) **skp,
                                STACK_OF(SSL_CIPHER) **scsvs, int sslv2format,
                                int fatal);
void ssl_update_cache(SSL *s, int mode);
__owur int ssl_cipher_get_evp_cipher(SSL_CTX *ctx, const SSL_CIPHER *sslc,
                                     const EVP_CIPHER **enc);
__owur int ssl_cipher_get_evp(SSL_CTX *ctxc, const SSL_SESSION *s,
                              const EVP_CIPHER **enc, const EVP_MD **md,
                              int *mac_pkey_type, size_t *mac_secret_size,
                              SSL_COMP **comp, int use_etm);
__owur int ssl_cipher_get_overhead(const SSL_CIPHER *c, size_t *mac_overhead,
                                   size_t *int_overhead, size_t *blocksize,
                                   size_t *ext_overhead);
__owur int ssl_cert_is_disabled(SSL_CTX *ctx, size_t idx);
__owur const SSL_CIPHER *ssl_get_cipher_by_char(SSL *ssl,
                                                const unsigned char *ptr,
                                                int all);
__owur int ssl_cert_set0_chain(SSL *s, SSL_CTX *ctx, STACK_OF(X509) *chain);
__owur int ssl_cert_set1_chain(SSL *s, SSL_CTX *ctx, STACK_OF(X509) *chain);
__owur int ssl_cert_add0_chain_cert(SSL *s, SSL_CTX *ctx, X509 *x);
__owur int ssl_cert_add1_chain_cert(SSL *s, SSL_CTX *ctx, X509 *x);
__owur int ssl_cert_select_current(CERT *c, X509 *x);
__owur int ssl_cert_set_current(CERT *c, long arg);
void ssl_cert_set_cert_cb(CERT *c, int (*cb) (SSL *ssl, void *arg), void *arg);

__owur int ssl_verify_cert_chain(SSL *s, STACK_OF(X509) *sk);
__owur int ssl_build_cert_chain(SSL *s, SSL_CTX *ctx, int flags);
__owur int ssl_cert_set_cert_store(CERT *c, X509_STORE *store, int chain,
                                   int ref);
__owur int ssl_cert_get_cert_store(CERT *c, X509_STORE **pstore, int chain);

__owur int ssl_security(const SSL *s, int op, int bits, int nid, void *other);
__owur int ssl_ctx_security(const SSL_CTX *ctx, int op, int bits, int nid,
                            void *other);
int ssl_get_security_level_bits(const SSL *s, const SSL_CTX *ctx, int *levelp);

__owur int ssl_cert_lookup_by_nid(int nid, size_t *pidx);
__owur const SSL_CERT_LOOKUP *ssl_cert_lookup_by_pkey(const EVP_PKEY *pk,
                                                      size_t *pidx);
__owur const SSL_CERT_LOOKUP *ssl_cert_lookup_by_idx(size_t idx);

int ssl_undefined_function(SSL *s);
__owur int ssl_undefined_void_function(void);
__owur int ssl_undefined_const_function(const SSL *s);
__owur int ssl_get_server_cert_serverinfo(SSL *s,
                                          const unsigned char **serverinfo,
                                          size_t *serverinfo_length);
void ssl_set_masks(SSL *s);
__owur STACK_OF(SSL_CIPHER) *ssl_get_ciphers_by_id(SSL *s);
__owur int ssl_x509err2alert(int type);
void ssl_sort_cipher_list(void);
int ssl_load_ciphers(SSL_CTX *ctx);
__owur int ssl_setup_sig_algs(SSL_CTX *ctx);
int ssl_load_groups(SSL_CTX *ctx);
__owur int ssl_fill_hello_random(SSL *s, int server, unsigned char *field,
                                 size_t len, DOWNGRADE dgrd);
__owur int ssl_generate_master_secret(SSL *s, unsigned char *pms, size_t pmslen,
                                      int free_pms);
__owur EVP_PKEY *ssl_generate_pkey(SSL *s, EVP_PKEY *pm);
__owur int ssl_gensecret(SSL *s, unsigned char *pms, size_t pmslen);
__owur int ssl_derive(SSL *s, EVP_PKEY *privkey, EVP_PKEY *pubkey,
                      int genmaster);
__owur int ssl_decapsulate(SSL *s, EVP_PKEY *privkey,
                           const unsigned char *ct, size_t ctlen,
                           int gensecret);
__owur int ssl_encapsulate(SSL *s, EVP_PKEY *pubkey,
                           unsigned char **ctp, size_t *ctlenp,
                           int gensecret);
__owur EVP_PKEY *ssl_dh_to_pkey(DH *dh);
__owur int ssl_set_tmp_ecdh_groups(uint16_t **pext, size_t *pextlen,
                                   void *key);
__owur unsigned int ssl_get_max_send_fragment(const SSL *ssl);
__owur unsigned int ssl_get_split_send_fragment(const SSL *ssl);

__owur const SSL_CIPHER *ssl3_get_cipher_by_id(uint32_t id);
__owur const SSL_CIPHER *ssl3_get_cipher_by_std_name(const char *stdname);
__owur const SSL_CIPHER *ssl3_get_cipher_by_char(const unsigned char *p);
__owur int ssl3_put_cipher_by_char(const SSL_CIPHER *c, WPACKET *pkt,
                                   size_t *len);
int ssl3_init_finished_mac(SSL *s);
__owur int ssl3_setup_key_block(SSL *s);
__owur int ssl3_change_cipher_state(SSL *s, int which);
void ssl3_cleanup_key_block(SSL *s);
__owur int ssl3_do_write(SSL *s, int type);
int ssl3_send_alert(SSL *s, int level, int desc);
__owur int ssl3_generate_master_secret(SSL *s, unsigned char *out,
                                       unsigned char *p, size_t len,
                                       size_t *secret_size);
__owur int ssl3_get_req_cert_type(SSL *s, WPACKET *pkt);
__owur int ssl3_num_ciphers(void);
__owur const SSL_CIPHER *ssl3_get_cipher(unsigned int u);
int ssl3_renegotiate(SSL *ssl);
int ssl3_renegotiate_check(SSL *ssl, int initok);
void ssl3_digest_master_key_set_params(const SSL_SESSION *session,
                                       OSSL_PARAM params[]);
__owur int ssl3_dispatch_alert(SSL *s);
__owur size_t ssl3_final_finish_mac(SSL *s, const char *sender, size_t slen,
                                    unsigned char *p);
__owur int ssl3_finish_mac(SSL *s, const unsigned char *buf, size_t len);
void ssl3_free_digest_list(SSL *s);
__owur unsigned long ssl3_output_cert_chain(SSL *s, WPACKET *pkt,
                                            CERT_PKEY *cpk);
__owur const SSL_CIPHER *ssl3_choose_cipher(SSL *ssl,
                                            STACK_OF(SSL_CIPHER) *clnt,
                                            STACK_OF(SSL_CIPHER) *srvr);
__owur int ssl3_digest_cached_records(SSL *s, int keep);
__owur int ssl3_new(SSL *s);
void ssl3_free(SSL *s);
__owur int ssl3_read(SSL *s, void *buf, size_t len, size_t *readbytes);
__owur int ssl3_peek(SSL *s, void *buf, size_t len, size_t *readbytes);
__owur int ssl3_write(SSL *s, const void *buf, size_t len, size_t *written);
__owur int ssl3_shutdown(SSL *s);
int ssl3_clear(SSL *s);
__owur long ssl3_ctrl(SSL *s, int cmd, long larg, void *parg);
__owur long ssl3_ctx_ctrl(SSL_CTX *s, int cmd, long larg, void *parg);
__owur long ssl3_callback_ctrl(SSL *s, int cmd, void (*fp) (void));
__owur long ssl3_ctx_callback_ctrl(SSL_CTX *s, int cmd, void (*fp) (void));

__owur int ssl3_do_change_cipher_spec(SSL *ssl);
__owur long ssl3_default_timeout(void);

__owur int ssl3_set_handshake_header(SSL *s, WPACKET *pkt, int htype);
__owur int tls_close_construct_packet(SSL *s, WPACKET *pkt, int htype);
__owur int tls_setup_handshake(SSL *s);
__owur int dtls1_set_handshake_header(SSL *s, WPACKET *pkt, int htype);
__owur int dtls1_close_construct_packet(SSL *s, WPACKET *pkt, int htype);
__owur int ssl3_handshake_write(SSL *s);

__owur int ssl_allow_compression(SSL *s);

__owur int ssl_version_supported(const SSL *s, int version,
                                 const SSL_METHOD **meth);

__owur int ssl_set_client_hello_version(SSL *s);
__owur int ssl_check_version_downgrade(SSL *s);
__owur int ssl_set_version_bound(int method_version, int version, int *bound);
__owur int ssl_choose_server_version(SSL *s, CLIENTHELLO_MSG *hello,
                                     DOWNGRADE *dgrd);
__owur int ssl_choose_client_version(SSL *s, int version,
                                     RAW_EXTENSION *extensions);
__owur int ssl_get_min_max_version(const SSL *s, int *min_version,
                                   int *max_version, int *real_max);

__owur long tls1_default_timeout(void);
__owur int dtls1_do_write(SSL *s, int type);
void dtls1_set_message_header(SSL *s,
                              unsigned char mt,
                              size_t len,
                              size_t frag_off, size_t frag_len);

int dtls1_write_app_data_bytes(SSL *s, int type, const void *buf_, size_t len,
                               size_t *written);

__owur int dtls1_read_failed(SSL *s, int code);
__owur int dtls1_buffer_message(SSL *s, int ccs);
__owur int dtls1_retransmit_message(SSL *s, unsigned short seq, int *found);
__owur int dtls1_get_queue_priority(unsigned short seq, int is_ccs);
int dtls1_retransmit_buffered_messages(SSL *s);
void dtls1_clear_received_buffer(SSL *s);
void dtls1_clear_sent_buffer(SSL *s);
void dtls1_get_message_header(unsigned char *data,
                              struct hm_header_st *msg_hdr);
__owur long dtls1_default_timeout(void);
__owur struct timeval *dtls1_get_timeout(SSL *s, struct timeval *timeleft);
__owur int dtls1_check_timeout_num(SSL *s);
__owur int dtls1_handle_timeout(SSL *s);
void dtls1_start_timer(SSL *s);
void dtls1_stop_timer(SSL *s);
__owur int dtls1_is_timer_expired(SSL *s);
__owur int dtls_raw_hello_verify_request(WPACKET *pkt, unsigned char *cookie,
                                         size_t cookie_len);
__owur size_t dtls1_min_mtu(SSL *s);
void dtls1_hm_fragment_free(hm_fragment *frag);
__owur int dtls1_query_mtu(SSL *s);

__owur int tls1_new(SSL *s);
void tls1_free(SSL *s);
int tls1_clear(SSL *s);

__owur int dtls1_new(SSL *s);
void dtls1_free(SSL *s);
int dtls1_clear(SSL *s);
long dtls1_ctrl(SSL *s, int cmd, long larg, void *parg);
__owur int dtls1_shutdown(SSL *s);

__owur int dtls1_dispatch_alert(SSL *s);

__owur int ssl_init_wbio_buffer(SSL *s);
int ssl_free_wbio_buffer(SSL *s);

__owur int tls1_change_cipher_state(SSL *s, int which);
__owur int tls1_setup_key_block(SSL *s);
__owur size_t tls1_final_finish_mac(SSL *s, const char *str, size_t slen,
                                    unsigned char *p);
__owur int tls1_generate_master_secret(SSL *s, unsigned char *out,
                                       unsigned char *p, size_t len,
                                       size_t *secret_size);
__owur int tls13_setup_key_block(SSL *s);
__owur size_t tls13_final_finish_mac(SSL *s, const char *str, size_t slen,
                                     unsigned char *p);
__owur int tls13_change_cipher_state(SSL *s, int which);
__owur int tls13_update_key(SSL *s, int send);
__owur int tls13_hkdf_expand(SSL *s, const EVP_MD *md,
                             const unsigned char *secret,
                             const unsigned char *label, size_t labellen,
                             const unsigned char *data, size_t datalen,
                             unsigned char *out, size_t outlen, int fatal);
__owur int tls13_derive_key(SSL *s, const EVP_MD *md,
                            const unsigned char *secret, unsigned char *key,
                            size_t keylen);
__owur int tls13_derive_iv(SSL *s, const EVP_MD *md,
                           const unsigned char *secret, unsigned char *iv,
                           size_t ivlen);
__owur int tls13_derive_finishedkey(SSL *s, const EVP_MD *md,
                                    const unsigned char *secret,
                                    unsigned char *fin, size_t finlen);
int tls13_generate_secret(SSL *s, const EVP_MD *md,
                          const unsigned char *prevsecret,
                          const unsigned char *insecret,
                          size_t insecretlen,
                          unsigned char *outsecret);
__owur int tls13_generate_handshake_secret(SSL *s,
                                           const unsigned char *insecret,
                                           size_t insecretlen);
__owur int tls13_generate_master_secret(SSL *s, unsigned char *out,
                                        unsigned char *prev, size_t prevlen,
                                        size_t *secret_size);
__owur int tls1_export_keying_material(SSL *s, unsigned char *out, size_t olen,
                                       const char *label, size_t llen,
                                       const unsigned char *p, size_t plen,
                                       int use_context);
__owur int tls13_export_keying_material(SSL *s, unsigned char *out, size_t olen,
                                        const char *label, size_t llen,
                                        const unsigned char *context,
                                        size_t contextlen, int use_context);
__owur int tls13_export_keying_material_early(SSL *s, unsigned char *out,
                                              size_t olen, const char *label,
                                              size_t llen,
                                              const unsigned char *context,
                                              size_t contextlen);
__owur int tls1_alert_code(int code);
__owur int tls13_alert_code(int code);
__owur int ssl3_alert_code(int code);

__owur int ssl_check_srvr_ecc_cert_and_alg(X509 *x, SSL *s);

SSL_COMP *ssl3_comp_find(STACK_OF(SSL_COMP) *sk, int n);

__owur const TLS_GROUP_INFO *tls1_group_id_lookup(SSL_CTX *ctx, uint16_t curve_id);
__owur int tls1_group_id2nid(uint16_t group_id, int include_unknown);
__owur uint16_t tls1_nid2group_id(int nid);
__owur int tls1_check_group_id(SSL *s, uint16_t group_id, int check_own_curves);
__owur uint16_t tls1_shared_group(SSL *s, int nmatch);
__owur int tls1_set_groups(uint16_t **pext, size_t *pextlen,
                           int *curves, size_t ncurves);
__owur int tls1_set_groups_list(SSL_CTX *ctx, uint16_t **pext, size_t *pextlen,
                                const char *str);
__owur EVP_PKEY *ssl_generate_pkey_group(SSL *s, uint16_t id);
__owur int tls_valid_group(SSL *s, uint16_t group_id, int minversion,
                           int maxversion, int isec, int *okfortls13);
__owur EVP_PKEY *ssl_generate_param_group(SSL *s, uint16_t id);
void tls1_get_formatlist(SSL *s, const unsigned char **pformats,
                         size_t *num_formats);
__owur int tls1_check_ec_tmp_key(SSL *s, unsigned long id);

__owur int tls_group_allowed(SSL *s, uint16_t curve, int op);
void tls1_get_supported_groups(SSL *s, const uint16_t **pgroups,
                               size_t *pgroupslen);

__owur int tls1_set_server_sigalgs(SSL *s);

__owur SSL_TICKET_STATUS tls_get_ticket_from_client(SSL *s, CLIENTHELLO_MSG *hello,
                                                    SSL_SESSION **ret);
__owur SSL_TICKET_STATUS tls_decrypt_ticket(SSL *s, const unsigned char *etick,
                                            size_t eticklen,
                                            const unsigned char *sess_id,
                                            size_t sesslen, SSL_SESSION **psess);

__owur int tls_use_ticket(SSL *s);

void ssl_set_sig_mask(uint32_t *pmask_a, SSL *s, int op);

__owur int tls1_set_sigalgs_list(CERT *c, const char *str, int client);
__owur int tls1_set_raw_sigalgs(CERT *c, const uint16_t *psigs, size_t salglen,
                                int client);
__owur int tls1_set_sigalgs(CERT *c, const int *salg, size_t salglen,
                            int client);
int tls1_check_chain(SSL *s, X509 *x, EVP_PKEY *pk, STACK_OF(X509) *chain,
                     int idx);
void tls1_set_cert_validity(SSL *s);

#  ifndef OPENSSL_NO_CT
__owur int ssl_validate_ct(SSL *s);
#  endif

__owur EVP_PKEY *ssl_get_auto_dh(SSL *s);

__owur int ssl_security_cert(SSL *s, SSL_CTX *ctx, X509 *x, int vfy, int is_ee);
__owur int ssl_security_cert_chain(SSL *s, STACK_OF(X509) *sk, X509 *ex,
                                   int vfy);

int tls_choose_sigalg(SSL *s, int fatalerrs);

__owur EVP_MD_CTX *ssl_replace_hash(EVP_MD_CTX **hash, const EVP_MD *md);
void ssl_clear_hash_ctx(EVP_MD_CTX **hash);
__owur long ssl_get_algorithm2(SSL *s);
__owur int tls12_copy_sigalgs(SSL *s, WPACKET *pkt,
                              const uint16_t *psig, size_t psiglen);
__owur int tls1_save_u16(PACKET *pkt, uint16_t **pdest, size_t *pdestlen);
__owur int tls1_save_sigalgs(SSL *s, PACKET *pkt, int cert);
__owur int tls1_process_sigalgs(SSL *s);
__owur int tls1_set_peer_legacy_sigalg(SSL *s, const EVP_PKEY *pkey);
__owur int tls1_lookup_md(SSL_CTX *ctx, const SIGALG_LOOKUP *lu,
                          const EVP_MD **pmd);
__owur size_t tls12_get_psigalgs(SSL *s, int sent, const uint16_t **psigs);
__owur int tls_check_sigalg_curve(const SSL *s, int curve);
__owur int tls12_check_peer_sigalg(SSL *s, uint16_t, EVP_PKEY *pkey);
__owur int ssl_set_client_disabled(SSL *s);
__owur int ssl_cipher_disabled(const SSL *s, const SSL_CIPHER *c, int op, int echde);

__owur int ssl_handshake_hash(SSL *s, unsigned char *out, size_t outlen,
                                 size_t *hashlen);
__owur const EVP_MD *ssl_md(SSL_CTX *ctx, int idx);
__owur const EVP_MD *ssl_handshake_md(SSL *s);
__owur const EVP_MD *ssl_prf_md(SSL *s);

/*
 * ssl_log_rsa_client_key_exchange logs |premaster| to the SSL_CTX associated
 * with |ssl|, if logging is enabled. It returns one on success and zero on
 * failure. The entry is identified by the first 8 bytes of
 * |encrypted_premaster|.
 */
__owur int ssl_log_rsa_client_key_exchange(SSL *ssl,
                                           const uint8_t *encrypted_premaster,
                                           size_t encrypted_premaster_len,
                                           const uint8_t *premaster,
                                           size_t premaster_len);

/*
 * ssl_log_secret logs |secret| to the SSL_CTX associated with |ssl|, if
 * logging is available. It returns one on success and zero on failure. It tags
 * the entry with |label|.
 */
__owur int ssl_log_secret(SSL *ssl, const char *label,
                          const uint8_t *secret, size_t secret_len);

#define MASTER_SECRET_LABEL "CLIENT_RANDOM"
#define CLIENT_EARLY_LABEL "CLIENT_EARLY_TRAFFIC_SECRET"
#define CLIENT_HANDSHAKE_LABEL "CLIENT_HANDSHAKE_TRAFFIC_SECRET"
#define SERVER_HANDSHAKE_LABEL "SERVER_HANDSHAKE_TRAFFIC_SECRET"
#define CLIENT_APPLICATION_LABEL "CLIENT_TRAFFIC_SECRET_0"
#define SERVER_APPLICATION_LABEL "SERVER_TRAFFIC_SECRET_0"
#define EARLY_EXPORTER_SECRET_LABEL "EARLY_EXPORTER_SECRET"
#define EXPORTER_SECRET_LABEL "EXPORTER_SECRET"

#  ifndef OPENSSL_NO_KTLS
/* ktls.c */
int ktls_check_supported_cipher(const SSL *s, const EVP_CIPHER *c,
                                const EVP_CIPHER_CTX *dd);
int ktls_configure_crypto(const SSL *s, const EVP_CIPHER *c, EVP_CIPHER_CTX *dd,
                          void *rl_sequence, ktls_crypto_info_t *crypto_info,
                          unsigned char **rec_seq, unsigned char *iv,
                          unsigned char *key, unsigned char *mac_key,
                          size_t mac_secret_size);
#  endif

/* s3_cbc.c */
__owur char ssl3_cbc_record_digest_supported(const EVP_MD_CTX *ctx);
__owur int ssl3_cbc_digest_record(const EVP_MD *md,
                                  unsigned char *md_out,
                                  size_t *md_out_size,
                                  const unsigned char *header,
                                  const unsigned char *data,
                                  size_t data_size,
                                  size_t data_plus_mac_plus_padding_size,
                                  const unsigned char *mac_secret,
                                  size_t mac_secret_length, char is_sslv3);

__owur int srp_generate_server_master_secret(SSL *s);
__owur int srp_generate_client_master_secret(SSL *s);
__owur int srp_verify_server_param(SSL *s);

/* statem/statem_srvr.c */

__owur int send_certificate_request(SSL *s);

/* statem/extensions_cust.c */

custom_ext_method *custom_ext_find(const custom_ext_methods *exts,
                                   ENDPOINT role, unsigned int ext_type,
                                   size_t *idx);

void custom_ext_init(custom_ext_methods *meths);

__owur int custom_ext_parse(SSL *s, unsigned int context, unsigned int ext_type,
                            const unsigned char *ext_data, size_t ext_size,
                            X509 *x, size_t chainidx);
__owur int custom_ext_add(SSL *s, int context, WPACKET *pkt, X509 *x,
                          size_t chainidx, int maxversion);

__owur int custom_exts_copy(custom_ext_methods *dst,
                            const custom_ext_methods *src);
__owur int custom_exts_copy_flags(custom_ext_methods *dst,
                                  const custom_ext_methods *src);
void custom_exts_free(custom_ext_methods *exts);

void ssl_comp_free_compression_methods_int(void);

#ifndef OPENSSL_NO_QUIC
__owur int SSL_clear_not_quic(SSL *s);
__owur int SSL_clear_quic(SSL *s);
#endif

/* ssl_mcnf.c */
void ssl_ctx_system_config(SSL_CTX *ctx);

const EVP_CIPHER *ssl_evp_cipher_fetch(OSSL_LIB_CTX *libctx,
                                       int nid,
                                       const char *properties);
int ssl_evp_cipher_up_ref(const EVP_CIPHER *cipher);
void ssl_evp_cipher_free(const EVP_CIPHER *cipher);
const EVP_MD *ssl_evp_md_fetch(OSSL_LIB_CTX *libctx,
                               int nid,
                               const char *properties);
int ssl_evp_md_up_ref(const EVP_MD *md);
void ssl_evp_md_free(const EVP_MD *md);

int tls_provider_set_tls_params(SSL *s, EVP_CIPHER_CTX *ctx,
                                const EVP_CIPHER *ciph,
                                const EVP_MD *md);

void tls_engine_finish(ENGINE *e);
const EVP_CIPHER *tls_get_cipher_from_engine(int nid);
const EVP_MD *tls_get_digest_from_engine(int nid);
int tls_engine_load_ssl_client_cert(SSL *s, X509 **px509, EVP_PKEY **ppkey);
int ssl_hmac_old_new(SSL_HMAC *ret);
void ssl_hmac_old_free(SSL_HMAC *ctx);
int ssl_hmac_old_init(SSL_HMAC *ctx, void *key, size_t len, char *md);
int ssl_hmac_old_update(SSL_HMAC *ctx, const unsigned char *data, size_t len);
int ssl_hmac_old_final(SSL_HMAC *ctx, unsigned char *md, size_t *len);
size_t ssl_hmac_old_size(const SSL_HMAC *ctx);

int ssl_ctx_srp_ctx_free_intern(SSL_CTX *ctx);
int ssl_ctx_srp_ctx_init_intern(SSL_CTX *ctx);
int ssl_srp_ctx_free_intern(SSL *s);
int ssl_srp_ctx_init_intern(SSL *s);

int ssl_srp_calc_a_param_intern(SSL *s);
int ssl_srp_server_param_with_username_intern(SSL *s, int *ad);

void ssl_session_calculate_timeout(SSL_SESSION* ss);

# else /* OPENSSL_UNIT_TEST */

#  define ssl_init_wbio_buffer SSL_test_functions()->p_ssl_init_wbio_buffer
#  define ssl3_setup_buffers SSL_test_functions()->p_ssl3_setup_buffers

# endif

/* Some helper routines to support TSAN operations safely */
static ossl_unused ossl_inline int ssl_tsan_lock(const SSL_CTX *ctx)
{
#ifdef TSAN_REQUIRES_LOCKING
    if (!CRYPTO_THREAD_write_lock(ctx->tsan_lock))
        return 0;
#endif
    return 1;
}