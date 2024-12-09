
#define LLHTTP_VERSION_MAJOR 6
#define LLHTTP_VERSION_MINOR 0
#define LLHTTP_VERSION_PATCH 7

#ifndef LLHTTP_STRICT_MODE
# define LLHTTP_STRICT_MODE 0
#endif
  HPE_OK = 0,
  HPE_INTERNAL = 1,
  HPE_STRICT = 2,
  HPE_CR_EXPECTED = 25,
  HPE_LF_EXPECTED = 3,
  HPE_UNEXPECTED_CONTENT_LENGTH = 4,
  HPE_CLOSED_CONNECTION = 5,
  HPE_INVALID_METHOD = 6,
enum llhttp_lenient_flags {
  LENIENT_HEADERS = 0x1,
  LENIENT_CHUNKED_LENGTH = 0x2,
  LENIENT_KEEP_ALIVE = 0x4,
  LENIENT_TRANSFER_ENCODING = 0x8
};
typedef enum llhttp_lenient_flags llhttp_lenient_flags_t;

enum llhttp_type {
  XX(0, OK, OK) \
  XX(1, INTERNAL, INTERNAL) \
  XX(2, STRICT, STRICT) \
  XX(25, CR_EXPECTED, CR_EXPECTED) \
  XX(3, LF_EXPECTED, LF_EXPECTED) \
  XX(4, UNEXPECTED_CONTENT_LENGTH, UNEXPECTED_CONTENT_LENGTH) \
  XX(5, CLOSED_CONNECTION, CLOSED_CONNECTION) \
  XX(6, INVALID_METHOD, INVALID_METHOD) \
void llhttp_init(llhttp_t* parser, llhttp_type_t type,
                 const llhttp_settings_t* settings);

LLHTTP_EXPORT
llhttp_t* llhttp_alloc(llhttp_type_t type);

LLHTTP_EXPORT
LLHTTP_EXPORT
uint8_t llhttp_get_upgrade(llhttp_t* parser);

/* Reset an already initialized parser back to the start state, preserving the
 * existing parser type, callback settings, user data, and lenient flags.
 */
LLHTTP_EXPORT
 */
void llhttp_set_lenient_keep_alive(llhttp_t* parser, int enabled);

/* Enables/disables lenient handling of `Transfer-Encoding` header.
 *
 * Normally `llhttp` would error when a `Transfer-Encoding` has `chunked` value
 * and another value after it (either in a single header or in multiple
 * headers whose value are internally joined using `, `).
 * This is mandated by the spec to reliably determine request body size and thus
 * avoid request smuggling.
 * With this flag the extra value will be parsed normally.
 *
 * **(USE AT YOUR OWN RISK)**
 */
void llhttp_set_lenient_transfer_encoding(llhttp_t* parser, int enabled);

#ifdef __cplusplus
}  /* extern "C" */
#endif
#endif  /* INCLUDE_LLHTTP_API_H_ */