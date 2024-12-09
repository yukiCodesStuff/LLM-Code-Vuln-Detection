   USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#if defined(__GNUC__) && defined(__i386__) && ! defined(__MINGW32__)
/* We'll use this version by default only where we know it helps.

   regparm() generates warnings on Solaris boxes.   See SF bug #692878.

   #define FASTCALL __attribute__((stdcall, regparm(3)))
   and let's try this:
*/
#  define FASTCALL __attribute__((regparm(3)))
#  define PTRFASTCALL __attribute__((regparm(3)))
#endif

/* Using __fastcall seems to have an unexpected negative effect under
   MS VC++, especially for function pointers, so we won't use it for
/* Make sure all of these are defined if they aren't already. */

#ifndef FASTCALL
#  define FASTCALL
#endif

#ifndef PTRCALL
#  define PTRCALL
#endif

#ifndef PTRFASTCALL
#  define PTRFASTCALL
#endif

#ifndef XML_MIN_SIZE
#  if ! defined(__cplusplus) && ! defined(inline)
#    ifdef __GNUC__
#      define inline __inline
#    endif /* __GNUC__ */
#  endif
#endif /* XML_MIN_SIZE */

#ifdef __cplusplus
#  define inline inline
#else
#  ifndef inline
#    define inline
#  endif
#endif

#ifndef UNUSED_P
#  define UNUSED_P(p) (void)p
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef XML_ENABLE_VISIBILITY
#  if XML_ENABLE_VISIBILITY
__attribute__((visibility("default")))
#  endif
#endif
void
_INTERNAL_trim_to_complete_utf8_characters(const char *from,
                                           const char **fromLimRef);

#ifdef __cplusplus
}
#endif