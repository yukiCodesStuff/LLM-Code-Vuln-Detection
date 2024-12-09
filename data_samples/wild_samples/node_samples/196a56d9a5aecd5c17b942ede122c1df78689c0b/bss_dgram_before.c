{
# if defined(_WIN32)
    SYSTEMTIME st;
    unsigned __int64 now_ul;
    FILETIME now_ft;

    GetSystemTime(&st);
    SystemTimeToFileTime(&st, &now_ft);
    now_ul = ((unsigned __int64)now_ft.dwHighDateTime << 32) | now_ft.dwLowDateTime;
#  ifdef  __MINGW32__
    now_ul -= 116444736000000000ULL;
#  else
    now_ul -= 116444736000000000UI64; /* re-bias to 1/1/1970 */
#  endif
    t->tv_sec = (long)(now_ul / 10000000);
    t->tv_usec = ((int)(now_ul % 10000000)) / 10;
# else
    gettimeofday(t, NULL);
# endif
}

#endif