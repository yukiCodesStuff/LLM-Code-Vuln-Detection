
  if test -z "$TIDY_DIR"; then
    AC_MSG_ERROR(Cannot find libtidy)
  else
    dnl Check for tidybuffio.h (as opposed to simply buffio.h)
    dnl which indicates that we are building against tidy-html5
    dnl and not the legacy htmltidy. The two are compatible,
    dnl except for with regard to this header file.
    if test -f "$TIDY_INCDIR/tidybuffio.h"; then
      AC_DEFINE(HAVE_TIDYBUFFIO_H,1,[defined if tidybuffio.h exists])
    fi
  fi

  TIDY_LIBDIR=$TIDY_DIR/$PHP_LIBDIR
