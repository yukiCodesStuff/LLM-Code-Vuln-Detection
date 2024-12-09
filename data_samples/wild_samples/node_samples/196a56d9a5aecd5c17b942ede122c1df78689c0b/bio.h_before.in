enum BIO_sock_info_type {
    BIO_SOCK_INFO_ADDRESS
};
int BIO_sock_info(int sock,
                  enum BIO_sock_info_type type, union BIO_sock_info_u *info);

#  define BIO_SOCK_REUSEADDR    0x01
#  define BIO_SOCK_V6_ONLY      0x02
#  define BIO_SOCK_KEEPALIVE    0x04
#  define BIO_SOCK_NONBLOCK     0x08
#  define BIO_SOCK_NODELAY      0x10

int BIO_socket(int domain, int socktype, int protocol, int options);
int BIO_connect(int sock, const BIO_ADDR *addr, int options);
int BIO_bind(int sock, const BIO_ADDR *addr, int options);
int BIO_listen(int sock, const BIO_ADDR *addr, int options);
int BIO_accept_ex(int accept_sock, BIO_ADDR *addr, int options);
int BIO_closesocket(int sock);

BIO *BIO_new_socket(int sock, int close_flag);
BIO *BIO_new_connect(const char *host_port);
BIO *BIO_new_accept(const char *host_port);
# endif /* OPENSSL_NO_SOCK*/

BIO *BIO_new_fd(int fd, int close_flag);

int BIO_new_bio_pair(BIO **bio1, size_t writebuf1,
                     BIO **bio2, size_t writebuf2);
/*
 * If successful, returns 1 and in *bio1, *bio2 two BIO pair endpoints.
 * Otherwise returns 0 and sets *bio1 and *bio2 to NULL. Size 0 uses default
 * value.
 */

void BIO_copy_next_retry(BIO *b);

/*
 * long BIO_ghbn_ctrl(int cmd,int iarg,char *parg);
 */

# define ossl_bio__attr__(x)
# if defined(__GNUC__) && defined(__STDC_VERSION__) \
    && !defined(__APPLE__)
    /*
     * Because we support the 'z' modifier, which made its appearance in C99,
     * we can't use __attribute__ with pre C99 dialects.
     */
#  if __STDC_VERSION__ >= 199901L
#   undef ossl_bio__attr__
#   define ossl_bio__attr__ __attribute__
#   if __GNUC__*10 + __GNUC_MINOR__ >= 44
#    define ossl_bio__printf__ __gnu_printf__
#   else
#    define ossl_bio__printf__ __printf__
#   endif
#  endif
# endif
int BIO_printf(BIO *bio, const char *format, ...)
ossl_bio__attr__((__format__(ossl_bio__printf__, 2, 3)));
int BIO_vprintf(BIO *bio, const char *format, va_list args)
ossl_bio__attr__((__format__(ossl_bio__printf__, 2, 0)));
int BIO_snprintf(char *buf, size_t n, const char *format, ...)
ossl_bio__attr__((__format__(ossl_bio__printf__, 3, 4)));
int BIO_vsnprintf(char *buf, size_t n, const char *format, va_list args)
ossl_bio__attr__((__format__(ossl_bio__printf__, 3, 0)));
# undef ossl_bio__attr__
# undef ossl_bio__printf__


BIO_METHOD *BIO_meth_new(int type, const char *name);
void BIO_meth_free(BIO_METHOD *biom);
int (*BIO_meth_get_write(const BIO_METHOD *biom)) (BIO *, const char *, int);
int (*BIO_meth_get_write_ex(const BIO_METHOD *biom)) (BIO *, const char *, size_t,
                                                size_t *);
int BIO_meth_set_write(BIO_METHOD *biom,
                       int (*write) (BIO *, const char *, int));
int BIO_meth_set_write_ex(BIO_METHOD *biom,
                       int (*bwrite) (BIO *, const char *, size_t, size_t *));
int (*BIO_meth_get_read(const BIO_METHOD *biom)) (BIO *, char *, int);
int (*BIO_meth_get_read_ex(const BIO_METHOD *biom)) (BIO *, char *, size_t, size_t *);
int BIO_meth_set_read(BIO_METHOD *biom,
                      int (*read) (BIO *, char *, int));
int BIO_meth_set_read_ex(BIO_METHOD *biom,
                         int (*bread) (BIO *, char *, size_t, size_t *));
int (*BIO_meth_get_puts(const BIO_METHOD *biom)) (BIO *, const char *);
int BIO_meth_set_puts(BIO_METHOD *biom,
                      int (*puts) (BIO *, const char *));
int (*BIO_meth_get_gets(const BIO_METHOD *biom)) (BIO *, char *, int);
int BIO_meth_set_gets(BIO_METHOD *biom,
                      int (*gets) (BIO *, char *, int));
long (*BIO_meth_get_ctrl(const BIO_METHOD *biom)) (BIO *, int, long, void *);
int BIO_meth_set_ctrl(BIO_METHOD *biom,
                      long (*ctrl) (BIO *, int, long, void *));
int (*BIO_meth_get_create(const BIO_METHOD *bion)) (BIO *);
int BIO_meth_set_create(BIO_METHOD *biom, int (*create) (BIO *));
int (*BIO_meth_get_destroy(const BIO_METHOD *biom)) (BIO *);
int BIO_meth_set_destroy(BIO_METHOD *biom, int (*destroy) (BIO *));
long (*BIO_meth_get_callback_ctrl(const BIO_METHOD *biom))
                                 (BIO *, int, BIO_info_cb *);
int BIO_meth_set_callback_ctrl(BIO_METHOD *biom,
                               long (*callback_ctrl) (BIO *, int,
                                                      BIO_info_cb *));

# ifdef  __cplusplus
}
# endif
#endif