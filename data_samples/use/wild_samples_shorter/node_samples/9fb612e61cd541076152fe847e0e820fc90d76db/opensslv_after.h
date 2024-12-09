 */
# define OPENSSL_VERSION_MAJOR  3
# define OPENSSL_VERSION_MINOR  0
# define OPENSSL_VERSION_PATCH  7

/*
 * Additional version information
 *
 * longer variant with OPENSSL_VERSION_PRE_RELEASE_STR and
 * OPENSSL_VERSION_BUILD_METADATA_STR appended.
 */
# define OPENSSL_VERSION_STR "3.0.7"
# define OPENSSL_FULL_VERSION_STR "3.0.7+quic"

/*
 * SECTION 3: ADDITIONAL METADATA
 *
 * These strings are defined separately to allow them to be parsable.
 */
# define OPENSSL_RELEASE_DATE "1 Nov 2022"

/*
 * SECTION 4: BACKWARD COMPATIBILITY
 */

# define OPENSSL_VERSION_TEXT "OpenSSL 3.0.7+quic 1 Nov 2022"

/* Synthesize OPENSSL_VERSION_NUMBER with the layout 0xMNN00PPSL */
# ifdef OPENSSL_VERSION_PRE_RELEASE
#  define _OPENSSL_VERSION_PRE_RELEASE 0x0L