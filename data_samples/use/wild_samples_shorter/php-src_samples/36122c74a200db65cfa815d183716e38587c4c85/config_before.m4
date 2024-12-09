    ODBC_LIBS=$CUSTOM_ODBC_LIBS
    ODBC_TYPE=custom-odbc
    AC_DEFINE(HAVE_CODBC,1,[ ])
    AC_MSG_RESULT([$ext_ouput])
  fi
fi

if test -z "$ODBC_TYPE"; then

  if test "$PHP_IODBC" != "no"; then
    AC_MSG_CHECKING(for iODBC support)
    if test "$PHP_IODBC" = "yes"; then
      PHP_IODBC=/usr/local
    fi
    PHP_ADD_LIBRARY_WITH_PATH(iodbc, $PHP_IODBC/$PHP_LIBDIR)
    PHP_ADD_INCLUDE($PHP_IODBC/include, 1)
    ODBC_TYPE=iodbc
    ODBC_INCLUDE=-I$PHP_IODBC/include
    ODBC_LFLAGS=-L$PHP_IODBC/$PHP_LIBDIR
    ODBC_LIBS=-liodbc
    AC_DEFINE(HAVE_IODBC,1,[ ])
    AC_DEFINE(HAVE_ODBC2,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then