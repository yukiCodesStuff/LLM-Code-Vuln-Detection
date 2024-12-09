 *     On Windows we store the error in the thread errno, not
 *     in the winsock error code. This is to avoid loosing the
 *     actual last winsock error. So use macro ERRNO to fetch the
 *     errno this funtion sets when returning NULL, not SOCKERRNO.
 * author:
 *     Paul Vixie, 1996.
 */
const char *