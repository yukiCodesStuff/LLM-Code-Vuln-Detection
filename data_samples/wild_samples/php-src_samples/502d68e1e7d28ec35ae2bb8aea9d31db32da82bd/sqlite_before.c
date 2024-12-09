/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2007 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Authors: Wez Furlong <wez@thebrainroom.com>                          |
   |          Tal Peer <tal@php.net>                                      |
   |          Marcus Boerger <helly@php.net>                              |
   +----------------------------------------------------------------------+

   $Id$
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#define PHP_SQLITE_MODULE_VERSION	"2.0-dev"

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
#include "ext/session/php_session.h"
#endif
#include "php_sqlite.h"

#if HAVE_TIME_H
# include <time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <sqlite.h>

#include "zend_exceptions.h"
#include "zend_interfaces.h"

#if defined(HAVE_SPL) && ((PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION >= 1))
extern PHPAPI zend_class_entry *spl_ce_RuntimeException;
extern PHPAPI zend_class_entry *spl_ce_Countable;
#endif

#if PHP_SQLITE2_HAVE_PDO
# include "pdo/php_pdo.h"
# include "pdo/php_pdo_driver.h"
extern pdo_driver_t pdo_sqlite2_driver;
#endif

#ifndef safe_emalloc
# define safe_emalloc(a,b,c) emalloc((a)*(b)+(c))
#endif

ZEND_DECLARE_MODULE_GLOBALS(sqlite)
static PHP_GINIT_FUNCTION(sqlite);

#if HAVE_PHP_SESSION && !defined(COMPILE_DL_SESSION)
extern ps_module ps_mod_sqlite;
#define ps_sqlite_ptr &ps_mod_sqlite
#endif

extern int sqlite_encode_binary(const unsigned char *in, int n, unsigned char *out);
extern int sqlite_decode_binary(const unsigned char *in, unsigned char *out);

#define php_sqlite_encode_binary(in, n, out) sqlite_encode_binary((const unsigned char *)in, n, (unsigned char *)out)
#define php_sqlite_decode_binary(in, out)    sqlite_decode_binary((const unsigned char *)in, (unsigned char *)out)

static int sqlite_count_elements(zval *object, long *count TSRMLS_DC);

static int le_sqlite_db, le_sqlite_result, le_sqlite_pdb;

static inline void php_sqlite_strtoupper(char *s)
{
	while (*s!='\0') {
		*s = toupper(*s);
		s++;
	}
}