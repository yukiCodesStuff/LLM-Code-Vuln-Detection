    t->tv_sec = (long)(now_ul / 10000000);
    t->tv_usec = ((int)(now_ul % 10000000)) / 10;
# else
    gettimeofday(t, NULL);
# endif
}

#endif