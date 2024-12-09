
# define ossl_bio__attr__(x)
# if defined(__GNUC__) && defined(__STDC_VERSION__) \
    && !defined(__MINGW32__) && !defined(__MINGW64__) \
    && !defined(__APPLE__)
    /*
     * Because we support the 'z' modifier, which made its appearance in C99,
     * we can't use __attribute__ with pre C99 dialects.