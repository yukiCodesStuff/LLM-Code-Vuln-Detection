dnl
dnl $Id$
dnl

AC_DEFUN([PHP_ODBC_CHECK_HEADER],[
if ! test -f "$ODBC_INCDIR/$1"; then
  AC_MSG_ERROR([ODBC header file '$ODBC_INCDIR/$1' not found!])
fi
])

dnl
dnl Figure out which library file to link with for the Solid support.
dnl
AC_DEFUN([PHP_ODBC_FIND_SOLID_LIBS],[
  AC_MSG_CHECKING([Solid library file])  
  ac_solid_uname_r=`uname -r 2>/dev/null`
  ac_solid_uname_s=`uname -s 2>/dev/null`
  case $ac_solid_uname_s in
    AIX) ac_solid_os=a3x;;   # a4x for AIX4/ Solid 2.3/3.0 only
    HP-UX) ac_solid_os=h9x;; # h1x for hpux11, h0x for hpux10
    IRIX) ac_solid_os=irx;;  # Solid 2.3(?)/ 3.0 only
    Linux) 
      if ldd -v /bin/sh | grep GLIBC > /dev/null; then
        AC_DEFINE(SS_LINUX,1,[Needed in sqlunix.h ])
        ac_solid_os=l2x
      else
        AC_DEFINE(SS_LINUX,1,[Needed in sqlunix.h ])
        ac_solid_os=lux
      fi;; 
    SunOS) 
      ac_solid_os=ssx;; # should we deal with SunOS 4?
    FreeBSD) 
      if test `expr $ac_solid_uname_r : '\(.\)'` -gt "2"; then
        AC_DEFINE(SS_FBX,1,[Needed in sqlunix.h for wchar defs ])
        ac_solid_os=fex
      else 
        AC_DEFINE(SS_FBX,1,[Needed in sqlunix.h for wchar defs ])
        ac_solid_os=fbx
      fi;;
  esac

  if test -f $1/soc${ac_solid_os}35.a; then
    ac_solid_version=35
    ac_solid_prefix=soc
  elif test -f $1/scl${ac_solid_os}30.a; then
    ac_solid_version=30
    ac_solid_prefix=scl
  elif test -f $1/scl${ac_solid_os}23.a; then
    ac_solid_version=23
    ac_solid_prefix=scl
  fi

#
# Check for the library files, and setup the ODBC_LIBS path...
#
if test ! -f $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.so -a \
  ! -f $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.a; then
  #
  # we have an error and should bail out, as we can't find the libs!
  #
  echo ""
  echo "*********************************************************************"
  echo "* Unable to locate $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.so or $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.a"
  echo "* Please correct this by creating the following links and reconfiguring:"
  echo "* $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.a -> $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.a"
  echo "* $1/${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.so -> $1/lib${ac_solid_prefix}${ac_solid_os}${ac_solid_version}.so"
  echo "*********************************************************************"
else
  ODBC_LFLAGS=-L$1
  ODBC_LIBS=-l${ac_solid_prefix}${ac_solid_os}${ac_solid_version}
fi

  AC_MSG_RESULT(`echo $ODBC_LIBS | sed -e 's!.*/!!'`)
])


dnl
dnl Figure out which library file to link with for the Empress support.
dnl

AC_DEFUN([PHP_ODBC_FIND_EMPRESS_LIBS],[
  AC_MSG_CHECKING([Empress library file])
  ODBC_LIBS=`echo $1/libempodbccl.so | cut -d' ' -f1`
  if test ! -f $ODBC_LIBS; then
    ODBC_LIBS=`echo $1/libempodbccl.so | cut -d' ' -f1`
  fi
  AC_MSG_RESULT(`echo $ODBC_LIBS | sed -e 's!.*/!!'`)
])

AC_DEFUN([PHP_ODBC_FIND_EMPRESS_BCS_LIBS],[
  AC_MSG_CHECKING([Empress local access library file])
  ODBCBCS_LIBS=`echo $1/libempodbcbcs.a | cut -d' ' -f1`
  if test ! -f $ODBCBCS_LIBS; then
    ODBCBCS_LIBS=`echo $1/libempodbcbcs.a | cut -d' ' -f1`
  fi
  AC_MSG_RESULT(`echo $ODBCBCS_LIBS | sed -e 's!.*/!!'`)
])

dnl
dnl configure options
dnl
if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(adabas,,
[  --with-adabas[=DIR]       Include Adabas D support [/usr/local]])

  if test "$PHP_ADABAS" != "no"; then
    AC_MSG_CHECKING([for Adabas support])
    if test "$PHP_ADABAS" = "yes"; then
      PHP_ADABAS=/usr/local
    fi
    PHP_ADD_INCLUDE($PHP_ADABAS/incl)
    PHP_ADD_LIBPATH($PHP_ADABAS/$PHP_LIBDIR)
    ODBC_OBJS="$PHP_ADABAS/$PHP_LIBDIR/odbclib.a"
    ODBC_LIB="$abs_builddir/ext/odbc/libodbc_adabas.a"
    $srcdir/build/shtool mkdir -f -p ext/odbc
    rm -f "$ODBC_LIB"
    cp "$ODBC_OBJS" "$ODBC_LIB"
    PHP_ADD_LIBRARY(sqlptc)
    PHP_ADD_LIBRARY(sqlrte)
    PHP_ADD_LIBRARY_WITH_PATH(odbc_adabas, $abs_builddir/ext/odbc)
    ODBC_TYPE=adabas
    ODBC_INCDIR=$PHP_ADABAS/incl
    PHP_ODBC_CHECK_HEADER(sqlext.h)
    AC_DEFINE(HAVE_ADABAS,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(sapdb,,
[  --with-sapdb[=DIR]        Include SAP DB support [/usr/local]])

  if test "$PHP_SAPDB" != "no"; then
    AC_MSG_CHECKING([for SAP DB support])
    if test "$PHP_SAPDB" = "yes"; then
      PHP_SAPDB=/usr/local
    fi
    PHP_ADD_INCLUDE($PHP_SAPDB/incl)
    PHP_ADD_LIBPATH($PHP_SAPDB/$PHP_LIBDIR)
    PHP_ADD_LIBRARY(sqlod)
    ODBC_TYPE=sapdb
    AC_DEFINE(HAVE_SAPDB,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(solid,,
[  --with-solid[=DIR]        Include Solid support [/usr/local/solid]])

  if test "$PHP_SOLID" != "no"; then
    AC_MSG_CHECKING(for Solid support)
    if test "$PHP_SOLID" = "yes"; then
      PHP_SOLID=/usr/local/solid
    fi
    ODBC_INCDIR=$PHP_SOLID/include
    ODBC_LIBDIR=$PHP_SOLID/$PHP_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_TYPE=solid
    if test -f $ODBC_LIBDIR/soc*35.a; then
      AC_DEFINE(HAVE_SOLID_35,1,[ ])
    elif test -f $ODBC_LIBDIR/scl*30.a; then
      AC_DEFINE(HAVE_SOLID_30,1,[ ])
    elif test -f $ODBC_LIBDIR/scl*23.a; then
      AC_DEFINE(HAVE_SOLID,1,[ ])
    fi
    AC_MSG_RESULT([$ext_output])
    PHP_ODBC_FIND_SOLID_LIBS($ODBC_LIBDIR)
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(ibm-db2,,
[  --with-ibm-db2[=DIR]      Include IBM DB2 support [/home/db2inst1/sqllib]])

  if test "$PHP_IBM_DB2" != "no"; then
    AC_MSG_CHECKING(for IBM DB2 support)
    if test "$PHP_IBM_DB2" = "yes"; then
      ODBC_INCDIR=/home/db2inst1/sqllib/include
      ODBC_LIBDIR=/home/db2inst1/sqllib/lib
    else
      ODBC_INCDIR=$PHP_IBM_DB2/include
      ODBC_LIBDIR=$PHP_IBM_DB2/$PHP_LIBDIR
    fi

    PHP_ODBC_CHECK_HEADER(sqlcli1.h)

    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_TYPE=ibm-db2
    ODBC_LIBS=-ldb2

    PHP_TEST_BUILD(SQLExecute, [
      AC_DEFINE(HAVE_IBMDB2,1,[ ])
      AC_MSG_RESULT([$ext_output])
    ], [
      AC_MSG_RESULT(no)
      AC_MSG_ERROR([
build test failed. Please check the config.log for details.
You need to source your DB2 environment before running PHP configure:
# . \$IBM_DB2/db2profile
])
    ], [
      $ODBC_LFLAGS $ODBC_LIBS
    ])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(ODBCRouter,,
[  --with-ODBCRouter[=DIR]   Include ODBCRouter.com support [/usr]])

  if test "$PHP_ODBCROUTER" != "no"; then
    AC_MSG_CHECKING(for ODBCRouter.com support)
    if test "$PHP_ODBCROUTER" = "yes"; then
      PHP_ODBCROUTER=/usr
    fi
    ODBC_INCDIR=$PHP_ODBCROUTER/include
    ODBC_LIBDIR=$PHP_ODBCROUTER/lib
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LIBS=-lodbcsdk
    ODBC_TYPE=ODBCRouter
    AC_DEFINE(HAVE_ODBC_ROUTER,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(empress,,
[  --with-empress[=DIR]      Include Empress support [\$EMPRESSPATH]
                          (Empress Version >= 8.60 required)])

  if test "$PHP_EMPRESS" != "no"; then
    AC_MSG_CHECKING(for Empress support)
    if test "$PHP_EMPRESS" = "yes"; then
      ODBC_INCDIR=$EMPRESSPATH/include/odbc
      ODBC_LIBDIR=$EMPRESSPATH/shlib
    else
      ODBC_INCDIR=$PHP_EMPRESS/include/odbc
      ODBC_LIBDIR=$PHP_EMPRESS/shlib
    fi
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_TYPE=empress
    AC_DEFINE(HAVE_EMPRESS,1,[ ])
    AC_MSG_RESULT([$ext_output])
    PHP_ODBC_FIND_EMPRESS_LIBS($ODBC_LIBDIR)
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(empress-bcs,,
[  --with-empress-bcs[=DIR]
                          Include Empress Local Access support [\$EMPRESSPATH]
                          (Empress Version >= 8.60 required)])

  if test "$PHP_EMPRESS_BCS" != "no"; then
    AC_MSG_CHECKING(for Empress local access support)
    if test "$PHP_EMPRESS_BCS" = "yes"; then
      ODBC_INCDIR=$EMPRESSPATH/include/odbc
      ODBC_LIBDIR=$EMPRESSPATH/shlib
    else
      ODBC_INCDIR=$PHP_EMPRESS_BCS/include/odbc
      ODBC_LIBDIR=$PHP_EMPRESS_BCS/shlib
    fi
    CC="empocc -bcs";export CC;
    LD="empocc -bcs";export LD;
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    LIST=`empocc -listlines -bcs -o a a.c`

    NEWLIST=
    for I in $LIST
    do
      case $I in
        $EMPRESSPATH/odbccl/lib/* | \
        $EMPRESSPATH/rdbms/lib/* | \
        $EMPRESSPATH/common/lib/*)
              NEWLIST="$NEWLIST $I"
              ;;
      esac
    done
    ODBC_LIBS="-lempphpbcs -lms -lmscfg -lbasic -lbasic_os -lnlscstab -lnlsmsgtab -lm -ldl -lcrypt"
    ODBC_TYPE=empress-bcs
    AC_DEFINE(HAVE_EMPRESS,1,[ ])
    AC_MSG_RESULT([$ext_output])
    PHP_ODBC_FIND_EMPRESS_BCS_LIBS($ODBC_LIBDIR)
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(birdstep,,
[  --with-birdstep[=DIR]     Include Birdstep support [/usr/local/birdstep]])
  
  if test "$PHP_BIRDSTEP" != "no"; then
    AC_MSG_CHECKING(for Birdstep support)
    if test "$PHP_BIRDSTEP" = "yes"; then
        ODBC_INCDIR=/usr/local/birdstep/include
        ODBC_LIBDIR=/usr/local/birdstep/lib
    else
        ODBC_INCDIR=$PHP_BIRDSTEP/include
        ODBC_LIBDIR=$PHP_BIRDSTEP/$PHP_LIBDIR
    fi
   
    case $host_alias in
      *aix*[)]
        AC_DEFINE(AIX,1,[ ]);;
      *hpux*[)]
        AC_DEFINE(HPUX,1,[ ]);;
      *linux*[)]
        AC_DEFINE(LINUX,1,[ ]);;
      *qnx*[)]
        AC_DEFINE(NEUTRINO,1,[ ]);;
      i?86-*-solaris*[)]
        AC_DEFINE(ISOLARIS,1,[ ]);;
      sparc-*-solaris*[)]
        AC_DEFINE(SOLARIS,1,[ ]);;
      *unixware*[)]
        AC_DEFINE(UNIXWARE,1,[ ]);;
    esac

    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_TYPE=birdstep
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_LIBS="-lCadm -lCdict -lCenc -lCrdm -lCrpc -lCrdbc -lCrm -lCuapi -lutil"

    if test -f "$ODBC_LIBDIR/libCrdbc32.$SHLIB_SUFFIX_NAME"; then
      ODBC_LIBS="-lCrdbc32 -lCadm32 -lCncp32 -lCrm32 -lCsql32 -lCdict32 -lCrdm32 -lCrpc32 -lutil"
    elif test -f "$ODBC_LIBDIR/libCrdbc.$SHLIB_SUFFIX_NAME"; then
      ODBC_LIBS="-lCrdbc -lCadm -lCncp -lCrm -lCsql -lCdict -lCrdm -lCrpc -lutil"
    fi

    AC_DEFINE(HAVE_BIRDSTEP,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(custom-odbc,,
[  --with-custom-odbc[=DIR]  Include user defined ODBC support. DIR is ODBC install base
                          directory [/usr/local]. Make sure to define CUSTOM_ODBC_LIBS and
                          have some odbc.h in your include dirs. f.e. you should define 
                          following for Sybase SQL Anywhere 5.5.00 on QNX, prior to
                          running this configure script:
                            CPPFLAGS=\"-DODBC_QNX -DSQLANY_BUG\"
                            LDFLAGS=-lunix
                            CUSTOM_ODBC_LIBS=\"-ldblib -lodbc\"])

  if test "$PHP_CUSTOM_ODBC" != "no"; then
    AC_MSG_CHECKING(for a custom ODBC support)
    if test "$PHP_CUSTOM_ODBC" = "yes"; then
      PHP_CUSTOM_ODBC=/usr/local
    fi
    ODBC_INCDIR=$PHP_CUSTOM_ODBC/include
    ODBC_LIBDIR=$PHP_CUSTOM_ODBC/$PHP_LIBDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LIBS=$CUSTOM_ODBC_LIBS
    ODBC_TYPE=custom-odbc
    AC_DEFINE(HAVE_CODBC,1,[ ])
    AC_MSG_RESULT([$ext_ouput])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(iodbc,,
[  --with-iodbc[=DIR]        Include iODBC support [/usr/local]])

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
PHP_ARG_WITH(esoob,,
[  --with-esoob[=DIR]        Include Easysoft OOB support [/usr/local/easysoft/oob/client]])

  if test "$PHP_ESOOB" != "no"; then
    AC_MSG_CHECKING(for Easysoft ODBC-ODBC Bridge support)
    if test "$PHP_ESOOB" = "yes"; then
      PHP_ESOOB=/usr/local/easysoft/oob/client
    fi
    ODBC_INCDIR=$PHP_ESOOB/include
    ODBC_LIBDIR=$PHP_ESOOB/$PHP_LIBDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LIBS=-lesoobclient
    ODBC_TYPE=esoob
    AC_DEFINE(HAVE_ESOOB,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(unixODBC,,
[  --with-unixODBC[=DIR]     Include unixODBC support [/usr/local]])

  if test "$PHP_UNIXODBC" != "no"; then
    AC_MSG_CHECKING(for unixODBC support)
    if test "$PHP_UNIXODBC" = "yes"; then
      PHP_UNIXODBC=/usr/local
    fi
    ODBC_INCDIR=$PHP_UNIXODBC/include
    ODBC_LIBDIR=$PHP_UNIXODBC/$PHP_LIBDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LIBS=-lodbc
    ODBC_TYPE=unixODBC
    PHP_ODBC_CHECK_HEADER(sqlext.h)
    AC_DEFINE(HAVE_UNIXODBC,1,[ ])
    AC_MSG_RESULT([$ext_output])
  fi
fi

if test -z "$ODBC_TYPE"; then
PHP_ARG_WITH(dbmaker,,
[  --with-dbmaker[=DIR]      Include DBMaker support])

  if test "$PHP_DBMAKER" != "no"; then
    AC_MSG_CHECKING(for DBMaker support)
    if test "$PHP_DBMAKER" = "yes"; then
      # find dbmaker's home directory
      DBMAKER_HOME=`grep "^dbmaker:" /etc/passwd | $AWK -F: '{print $6}'`

      # check DBMaker version (from 5.0 to 2.0)
      DBMAKER_VERSION=5.0

      while test ! -d $DBMAKER_HOME/$DBMAKER_VERSION -a "$DBMAKER_VERSION" != "2.9"; do
        DM_VER=`echo $DBMAKER_VERSION | sed -e 's/\.//' | $AWK '{ print $1-1;}'`
        MAJOR_V=`echo $DM_VER | $AWK '{ print $1/10; }'  | $AWK -F. '{ print $1; }'`
        MINOR_V=`echo $DM_VER | $AWK '{ print $1%10; }'`
        DBMAKER_VERSION=$MAJOR_V.$MINOR_V
      done

      if test "$DBMAKER_VERSION" = "2.9"; then
        PHP_DBMAKER=$DBMAKER_HOME
      else
        PHP_DBMAKER=$DBMAKER_HOME/$DBMAKER_VERSION
      fi
    fi

    ODBC_INCDIR=$PHP_DBMAKER/include
    ODBC_LIBDIR=$PHP_DBMAKER/$PHP_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LFLAGS=-L$ODBC_LIBDIR
    ODBC_INCLUDE=-I$ODBC_INCDIR
    ODBC_LIBS="-ldmapic -lc"
    ODBC_TYPE=dbmaker

    AC_DEFINE(HAVE_DBMAKER,1,[Whether you want DBMaker])

    if test "$ext_shared" = "yes"; then
      AC_MSG_RESULT([yes (shared)])
      ODBC_LIBS="-ldmapic -lc -lm"
      ODBC_SHARED="odbc.la"
    else
      AC_MSG_RESULT([yes (static)])
      PHP_ADD_LIBRARY_WITH_PATH(dmapic, $ODBC_LIBDIR)
      PHP_ADD_INCLUDE($ODBC_INCDIR)
      ODBC_STATIC="libphpext_odbc.la"
    fi
  fi
fi

dnl
dnl Extension setup
dnl
if test -n "$ODBC_TYPE"; then
  if test "$ODBC_TYPE" != "dbmaker"; then
    PHP_EVAL_LIBLINE([$ODBC_LFLAGS $ODBC_LIBS], ODBC_SHARED_LIBADD)
    if test "$ODBC_TYPE" != "birdstep" && test "$ODBC_TYPE" != "solid"; then
      AC_DEFINE(HAVE_SQLDATASOURCES,1,[ ])
    fi
  fi
  
  AC_DEFINE(HAVE_UODBC,1,[ ])
  PHP_SUBST(ODBC_SHARED_LIBADD)
  PHP_SUBST(ODBC_INCDIR)
  PHP_SUBST(ODBC_LIBDIR)
  PHP_SUBST_OLD(ODBC_INCLUDE)
  PHP_SUBST_OLD(ODBC_LIBS)
  PHP_SUBST_OLD(ODBC_LFLAGS)
  PHP_SUBST_OLD(ODBC_TYPE)

  PHP_NEW_EXTENSION(odbc, php_odbc.c, $ext_shared,, $ODBC_INCLUDE)
fi