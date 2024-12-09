/*
   +----------------------------------------------------------------------+
   | PHP Version 5                                                        |
   +----------------------------------------------------------------------+
   | Copyright (c) 1997-2012 The PHP Group                                |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
   | Author: Wez Furlong <wez@thebrainroom.com>                           |
   +----------------------------------------------------------------------+
 */
/* $Id$ */

#if 0 && (defined(__linux__) || defined(sun) || defined(__IRIX__))
# define _BSD_SOURCE 		/* linux wants this when XOPEN mode is on */
# define _BSD_COMPAT		/* irix: uint */
# define _XOPEN_SOURCE 500  /* turn on Unix98 */
# define __EXTENSIONS__	1	/* Solaris: uint */
#endif

#include "php.h"
#include <stdio.h>
#include <ctype.h>
#include "php_string.h"
#include "ext/standard/head.h"
#include "ext/standard/basic_functions.h"
#include "ext/standard/file.h"
#include "exec.h"
#include "php_globals.h"
#include "SAPI.h"

#ifdef NETWARE
#include <proc.h>
#include <library.h>
#endif

#if HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif
#if HAVE_SIGNAL_H
#include <signal.h>
#endif

#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif

/* This symbol is defined in ext/standard/config.m4.
 * Essentially, it is set if you HAVE_FORK || PHP_WIN32
 * Otherplatforms may modify that configure check and add suitable #ifdefs
 * around the alternate code.
 * */
#ifdef PHP_CAN_SUPPORT_PROC_OPEN

#if 0 && HAVE_PTSNAME && HAVE_GRANTPT && HAVE_UNLOCKPT && HAVE_SYS_IOCTL_H && HAVE_TERMIOS_H
# include <sys/ioctl.h>
# include <termios.h>
# define PHP_CAN_DO_PTS	1
#endif

#include "proc_open.h"

static int le_proc_open;

/* {{{ _php_array_to_envp */
static php_process_env_t _php_array_to_envp(zval *environment, int is_persistent TSRMLS_DC)
{
	zval **element;
	php_process_env_t env;
	char *string_key, *data;
#ifndef PHP_WIN32
	char **ep;
#endif
	char *p;
	uint string_length, cnt, l, sizeenv=0, el_len;
	ulong num_key;
	HashTable *target_hash;
	HashPosition pos;

	memset(&env, 0, sizeof(env));

	if (!environment) {
		return env;
	}

	cnt = zend_hash_num_elements(Z_ARRVAL_P(environment));

	if (cnt < 1) {
#ifndef PHP_WIN32
		env.envarray = (char **) pecalloc(1, sizeof(char *), is_persistent);
#endif
		env.envp = (char *) pecalloc(4, 1, is_persistent);
		return env;
	}

	target_hash = HASH_OF(environment);
	if (!target_hash) {
		return env;
	}

	/* first, we have to get the size of all the elements in the hash */
	for (zend_hash_internal_pointer_reset_ex(target_hash, &pos);
			zend_hash_get_current_data_ex(target_hash, (void **) &element, &pos) == SUCCESS;
			zend_hash_move_forward_ex(target_hash, &pos)) {

		convert_to_string_ex(element);
		el_len = Z_STRLEN_PP(element);
		if (el_len == 0) {
			continue;
		}

		sizeenv += el_len+1;

		switch (zend_hash_get_current_key_ex(target_hash, &string_key, &string_length, &num_key, 0, &pos)) {
			case HASH_KEY_IS_STRING:
				if (string_length == 0) {
					continue;
				}
				sizeenv += string_length+1;
				break;
		}
	}

#ifndef PHP_WIN32
	ep = env.envarray = (char **) pecalloc(cnt + 1, sizeof(char *), is_persistent);
#endif
	p = env.envp = (char *) pecalloc(sizeenv + 4, 1, is_persistent);

	for (zend_hash_internal_pointer_reset_ex(target_hash, &pos);
			zend_hash_get_current_data_ex(target_hash, (void **) &element, &pos) == SUCCESS;
			zend_hash_move_forward_ex(target_hash, &pos)) {

		convert_to_string_ex(element);
		el_len = Z_STRLEN_PP(element);

		if (el_len == 0) {
			continue;
		}

		data = Z_STRVAL_PP(element);
		switch (zend_hash_get_current_key_ex(target_hash, &string_key, &string_length, &num_key, 0, &pos)) {
			case HASH_KEY_IS_STRING:
				if (string_length == 0) {
					continue;
				}

				l = string_length + el_len + 1;
				memcpy(p, string_key, string_length);
				strncat(p, "=", 1);
				strncat(p, data, el_len);

#ifndef PHP_WIN32
				*ep = p;
				++ep;
#endif
				p += l;
				break;
			case HASH_KEY_IS_LONG:
				memcpy(p,data,el_len);
#ifndef PHP_WIN32
				*ep = p;
				++ep;
#endif
				p += el_len + 1;
				break;
			case HASH_KEY_NON_EXISTANT:
				break;
		}
	}

	assert((uint)(p - env.envp) <= sizeenv);

	zend_hash_internal_pointer_reset_ex(target_hash, &pos);

	return env;
}