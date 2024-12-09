                 libgd/gdxpm.c libgd/gdfontt.c libgd/gdfonts.c libgd/gdfontmb.c libgd/gdfontl.c \
                 libgd/gdfontg.c libgd/gdtables.c libgd/gdft.c libgd/gdcache.c libgd/gdkanji.c \
                 libgd/wbmp.c libgd/gd_wbmp.c libgd/gdhelpers.c libgd/gd_topal.c libgd/gd_gif_in.c \
                 libgd/xbm.c libgd/gd_gif_out.c "

dnl check for fabsf and floorf which are available since C99
  AC_CHECK_FUNCS([fabsf floorf])
