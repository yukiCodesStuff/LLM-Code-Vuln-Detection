 * Definitions of all built-in extensions. NOTE: Changes in the number or order
 * of these extensions should be mirrored with equivalent changes to the
 * indexes ( TLSEXT_IDX_* ) defined in ssl_local.h.
 * Each extension has an initialiser, a client and
 * server side parser and a finaliser. The initialiser is called (if the
 * extension is relevant to the given context) even if we did not see the
 * extension in the message that we received. The parser functions are only
 * NOTE: WebSphere Application Server 7+ cannot handle empty extensions at
 * the end, keep these extensions before signature_algorithm.
 */
#define INVALID_EXTENSION { 0x10000, 0, NULL, NULL, NULL, NULL, NULL, NULL }
static const EXTENSION_DEFINITION ext_defs[] = {
    {
        TLSEXT_TYPE_renegotiate,
        SSL_EXT_CLIENT_HELLO | SSL_EXT_TLS1_2_SERVER_HELLO
    }
};

/* Check whether an extension's context matches the current context */
static int validate_context(SSL *s, unsigned int extctx, unsigned int thisctx)
{
    /* Check we're allowed to use this extension in this context */