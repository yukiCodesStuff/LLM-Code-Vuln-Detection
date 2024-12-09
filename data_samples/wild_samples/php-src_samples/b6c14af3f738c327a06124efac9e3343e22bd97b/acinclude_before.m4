{
  int n;

  n = write(1, TEXT, sizeof(TEXT)-1);
  return (!(n == sizeof(TEXT)-1));
}
    ],[
      ac_cv_write_stdout=yes
    ],[
      ac_cv_write_stdout=no
    ],[
      ac_cv_write_stdout=no
    ])
  ])
  if test "$ac_cv_write_stdout" = "yes"; then
    AC_DEFINE(PHP_WRITE_STDOUT, 1, [whether write(2) works])
  fi
])

dnl
dnl PHP_INIT_DTRACE(providerdesc, header-file, sources [, module])
dnl
AC_DEFUN([PHP_INIT_DTRACE],[
dnl Set paths properly when called from extension
  case "$4" in
    ""[)] ac_srcdir="$abs_srcdir/"; unset ac_bdir;;
    /*[)] ac_srcdir=`echo "$4"|cut -c 2-`"/"; ac_bdir=$ac_srcdir;;
    *[)] ac_srcdir="$abs_srcdir/$1/"; ac_bdir="$4/";;
  esac

dnl providerdesc
  ac_provsrc=$1
  old_IFS=[$]IFS
  IFS=.
  set $ac_provsrc
  ac_provobj=[$]1
  IFS=$old_IFS

dnl header-file
  ac_hdrobj=$2

dnl Add providerdesc.o into global objects when needed
  case $host_alias in
  *freebsd*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    PHP_LDFLAGS="$PHP_LDFLAGS -lelf"
    ;;
  *solaris*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    ;;
  *linux*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    ;;
  esac

dnl DTrace objects
  old_IFS=[$]IFS
  for ac_src in $3; do
    IFS=.
    set $ac_src
    ac_obj=[$]1
    IFS=$old_IFS

    PHP_DTRACE_OBJS="[$]PHP_DTRACE_OBJS [$]ac_bdir[$]ac_obj.lo"
  done;

  case [$]php_sapi_module in
  shared[)]
    for ac_lo in $PHP_DTRACE_OBJS; do
      dtrace_objs="[$]dtrace_objs `echo $ac_lo | $SED -e 's,\.lo$,.o,' -e 's#\(.*\)\/#\1\/.libs\/#'`"
    done;
    ;;
  *[)]
    dtrace_objs='$(PHP_DTRACE_OBJS:.lo=.o)'
    ;;
  esac

dnl Generate Makefile.objects entries
dnl The empty $ac_provsrc command stops an implicit circular dependency
dnl in GNU Make which causes the .d file to be overwritten (Bug 61268)
  cat>>Makefile.objects<<EOF

$abs_srcdir/$ac_provsrc:;

$ac_bdir[$]ac_hdrobj: $abs_srcdir/$ac_provsrc
	CFLAGS="\$(CFLAGS_CLEAN)" dtrace -h -C -s $ac_srcdir[$]ac_provsrc -o \$[]@.bak && \$(SED) 's,PHP_,DTRACE_,g' \$[]@.bak > \$[]@

\$(PHP_DTRACE_OBJS): $ac_bdir[$]ac_hdrobj

$ac_bdir[$]ac_provsrc.o: \$(PHP_DTRACE_OBJS)
	CFLAGS="\$(CFLAGS_CLEAN)" dtrace -G -o \$[]@ -s $abs_srcdir/$ac_provsrc $dtrace_objs

EOF
])

dnl
dnl PHP_CHECK_STDINT_TYPES
dnl
AC_DEFUN([PHP_CHECK_STDINT_TYPES], [
  AC_CHECK_SIZEOF([short], 2)
  AC_CHECK_SIZEOF([int], 4)
  AC_CHECK_SIZEOF([long], 4)
  AC_CHECK_SIZEOF([long long], 8)
  AC_CHECK_TYPES([int8, int16, int32, int64, int8_t, int16_t, int32_t, int64_t, uint8, uint16, uint32, uint64, uint8_t, uint16_t, uint32_t, uint64_t, u_int8_t, u_int16_t, u_int32_t, u_int64_t], [], [], [
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
  ])
  AC_DEFINE([PHP_HAVE_STDINT_TYPES], [1], [Checked for stdint types])
])
{
  int n;

  n = write(1, TEXT, sizeof(TEXT)-1);
  return (!(n == sizeof(TEXT)-1));
}
    ],[
      ac_cv_write_stdout=yes
    ],[
      ac_cv_write_stdout=no
    ],[
      ac_cv_write_stdout=no
    ])
  ])
  if test "$ac_cv_write_stdout" = "yes"; then
    AC_DEFINE(PHP_WRITE_STDOUT, 1, [whether write(2) works])
  fi
])

dnl
dnl PHP_INIT_DTRACE(providerdesc, header-file, sources [, module])
dnl
AC_DEFUN([PHP_INIT_DTRACE],[
dnl Set paths properly when called from extension
  case "$4" in
    ""[)] ac_srcdir="$abs_srcdir/"; unset ac_bdir;;
    /*[)] ac_srcdir=`echo "$4"|cut -c 2-`"/"; ac_bdir=$ac_srcdir;;
    *[)] ac_srcdir="$abs_srcdir/$1/"; ac_bdir="$4/";;
  esac

dnl providerdesc
  ac_provsrc=$1
  old_IFS=[$]IFS
  IFS=.
  set $ac_provsrc
  ac_provobj=[$]1
  IFS=$old_IFS

dnl header-file
  ac_hdrobj=$2

dnl Add providerdesc.o into global objects when needed
  case $host_alias in
  *freebsd*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    PHP_LDFLAGS="$PHP_LDFLAGS -lelf"
    ;;
  *solaris*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    ;;
  *linux*)
    PHP_GLOBAL_OBJS="[$]PHP_GLOBAL_OBJS [$]ac_bdir[$]ac_provsrc.o"
    ;;
  esac

dnl DTrace objects
  old_IFS=[$]IFS
  for ac_src in $3; do
    IFS=.
    set $ac_src
    ac_obj=[$]1
    IFS=$old_IFS

    PHP_DTRACE_OBJS="[$]PHP_DTRACE_OBJS [$]ac_bdir[$]ac_obj.lo"
  done;

  case [$]php_sapi_module in
  shared[)]
    for ac_lo in $PHP_DTRACE_OBJS; do
      dtrace_objs="[$]dtrace_objs `echo $ac_lo | $SED -e 's,\.lo$,.o,' -e 's#\(.*\)\/#\1\/.libs\/#'`"
    done;
    ;;
  *[)]
    dtrace_objs='$(PHP_DTRACE_OBJS:.lo=.o)'
    ;;
  esac

dnl Generate Makefile.objects entries
dnl The empty $ac_provsrc command stops an implicit circular dependency
dnl in GNU Make which causes the .d file to be overwritten (Bug 61268)
  cat>>Makefile.objects<<EOF

$abs_srcdir/$ac_provsrc:;

$ac_bdir[$]ac_hdrobj: $abs_srcdir/$ac_provsrc
	CFLAGS="\$(CFLAGS_CLEAN)" dtrace -h -C -s $ac_srcdir[$]ac_provsrc -o \$[]@.bak && \$(SED) 's,PHP_,DTRACE_,g' \$[]@.bak > \$[]@

\$(PHP_DTRACE_OBJS): $ac_bdir[$]ac_hdrobj

$ac_bdir[$]ac_provsrc.o: \$(PHP_DTRACE_OBJS)
	CFLAGS="\$(CFLAGS_CLEAN)" dtrace -G -o \$[]@ -s $abs_srcdir/$ac_provsrc $dtrace_objs

EOF
])

dnl
dnl PHP_CHECK_STDINT_TYPES
dnl
AC_DEFUN([PHP_CHECK_STDINT_TYPES], [
  AC_CHECK_SIZEOF([short], 2)
  AC_CHECK_SIZEOF([int], 4)
  AC_CHECK_SIZEOF([long], 4)
  AC_CHECK_SIZEOF([long long], 8)
  AC_CHECK_TYPES([int8, int16, int32, int64, int8_t, int16_t, int32_t, int64_t, uint8, uint16, uint32, uint64, uint8_t, uint16_t, uint32_t, uint64_t, u_int8_t, u_int16_t, u_int32_t, u_int64_t], [], [], [
#if HAVE_STDINT_H
# include <stdint.h>
#endif
#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif
  ])
  AC_DEFINE([PHP_HAVE_STDINT_TYPES], [1], [Checked for stdint types])
])