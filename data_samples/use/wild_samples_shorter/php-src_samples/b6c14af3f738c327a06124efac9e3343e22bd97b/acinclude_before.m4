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