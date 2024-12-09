/*
 * Copyright 1995-2022 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/bio.h>
#include <openssl/crypto.h>
#include <openssl/trace.h>
#include <openssl/lhash.h>
#include <openssl/conf.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#ifndef OPENSSL_NO_ENGINE
# include <openssl/engine.h>
#endif
#include <openssl/err.h>
/* Needed to get the other O_xxx flags. */
#ifdef OPENSSL_SYS_VMS
# include <unixio.h>
#endif
#include "apps.h"
#include "progs.h"

/*
 * The LHASH callbacks ("hash" & "cmp") have been replaced by functions with
 * the base prototypes (we cast each variable inside the function to the
 * required type of "FUNCTION*"). This removes the necessity for
 * macro-generated wrapper functions.
 */
static LHASH_OF(FUNCTION) *prog_init(void);
static int do_cmd(LHASH_OF(FUNCTION) *prog, int argc, char *argv[]);
char *default_config_file = NULL;

BIO *bio_in = NULL;
BIO *bio_out = NULL;
BIO *bio_err = NULL;

static void warn_deprecated(const FUNCTION *fp)
{
    if (fp->deprecated_version != NULL)
        BIO_printf(bio_err, "The command %s was deprecated in version %s.",
                   fp->name, fp->deprecated_version);
    else
        BIO_printf(bio_err, "The command %s is deprecated.", fp->name);
    if (strcmp(fp->deprecated_alternative, DEPRECATED_NO_ALTERNATIVE) != 0)
        BIO_printf(bio_err, " Use '%s' instead.", fp->deprecated_alternative);
    BIO_printf(bio_err, "\n");
}
    } else {
        argv[0] = pname;
    }

    /* If there's a command, run with that, otherwise "help". */
    ret = argc == 0 || global_help
        ? do_cmd(prog, 1, help_argv)
        : do_cmd(prog, argc, argv);

 end:
    OPENSSL_free(default_config_file);
    lh_FUNCTION_free(prog);
    OPENSSL_free(arg.argv);
    if (!app_RAND_write())
        ret = EXIT_FAILURE;

    BIO_free(bio_in);
    BIO_free_all(bio_out);
    apps_shutdown();
    BIO_free_all(bio_err);
    EXIT(ret);
}

typedef enum HELP_CHOICE {
    OPT_hERR = -1, OPT_hEOF = 0, OPT_hHELP
} HELP_CHOICE;

const OPTIONS help_options[] = {
    {OPT_HELP_STR, 1, '-', "Usage: help [options] [command]\n"},

    OPT_SECTION("General"),
    {"help", OPT_hHELP, '-', "Display this summary"},

    OPT_PARAMETERS(),
    {"command", 0, 0, "Name of command to display help (optional)"},
    {NULL}