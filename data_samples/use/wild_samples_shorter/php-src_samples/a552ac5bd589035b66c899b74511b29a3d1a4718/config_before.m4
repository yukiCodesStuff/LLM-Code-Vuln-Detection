
  if test -z "$TIDY_DIR"; then
    AC_MSG_ERROR(Cannot find libtidy)
  fi

  TIDY_LIBDIR=$TIDY_DIR/$PHP_LIBDIR
