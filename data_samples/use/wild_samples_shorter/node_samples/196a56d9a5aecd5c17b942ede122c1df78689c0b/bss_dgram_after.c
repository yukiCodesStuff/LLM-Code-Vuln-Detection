    t->tv_sec = (long)(now_ul / 10000000);
    t->tv_usec = ((int)(now_ul % 10000000)) / 10;
# else
    if (gettimeofday(t, NULL) < 0)
        perror("gettimeofday");
# endif
}

#endif