extern int sqlite_decode_binary(const unsigned char *in, unsigned char *out);

#define php_sqlite_encode_binary(in, n, out) sqlite_encode_binary((const unsigned char *)in, n, (unsigned char *)out)
#define php_sqlite_decode_binary(in, out) in && *in ? sqlite_decode_binary((const unsigned char *)in, (unsigned char *)out) : 0

static int sqlite_count_elements(zval *object, long *count TSRMLS_DC);

static int le_sqlite_db, le_sqlite_result, le_sqlite_pdb;