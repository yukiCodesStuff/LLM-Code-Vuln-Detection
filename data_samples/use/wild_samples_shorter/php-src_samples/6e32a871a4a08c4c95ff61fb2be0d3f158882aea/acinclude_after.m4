
AC_DEFUN([LIBZEND_BISON_CHECK],[
  # we only support certain bison versions
  bison_version_list="1.28 1.35 1.75 1.875 2.0 2.1 2.2 2.3 2.4 2.4.1 2.4.2 2.4.3 2.5 2.5.1 2.6 2.6.1 2.6.2 2.6.4"

  # for standalone build of Zend Engine
  test -z "$SED" && SED=sed
