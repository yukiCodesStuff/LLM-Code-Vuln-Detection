#! /usr/bin/env perl
# Copyright 2020-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
use strict;
use warnings;

use OpenSSL::Test qw(:DEFAULT data_file srctop_file);
use OpenSSL::Test::Utils;

#Tests for the dhparam CLI application


plan skip_all => "DH is not supported in this build"
    if disabled("dh");
plan tests => 21;

my $fipsconf = srctop_file("test", "fips-and-base.cnf");

sub checkdhparams {
    my $file = shift; #Filename containing params
    my $type = shift; #PKCS3 or X9.42?
        checkdhparams("gen-x942-0-512.der", "X9.42", 0, "DER", 512);
    };
}
SKIP: {
    skip "Skipping tests that are only supported in a fips build with security ".
        "checks", 4 if (disabled("fips") || disabled("fips-securitychecks"));

    $ENV{OPENSSL_CONF} = $fipsconf;

    ok(!run(app(['openssl', 'dhparam', '-check', '512'])),
        "Generating 512 bit DH params should fail in FIPS mode");

    ok(run(app(['openssl', 'dhparam', '-provider', 'default', '-propquery',
            '?fips!=yes', '-check', '512'])),
        "Generating 512 bit DH params should succeed in FIPS mode using".
        " non-FIPS property query");

    SKIP: {
        skip "Skipping tests that require DSA", 2 if disabled("dsa");

        ok(!run(app(['openssl', 'dhparam', '-dsaparam', '-check', '512'])),
            "Generating 512 bit DSA-style DH params should fail in FIPS mode");

        ok(run(app(['openssl', 'dhparam', '-provider', 'default', '-propquery',
                '?fips!=yes', '-dsaparam', '-check', '512'])),
            "Generating 512 bit DSA-style DH params should succeed in FIPS".
            " mode using non-FIPS property query");
    }

    delete $ENV{OPENSSL_CONF};
}

ok(run(app(["openssl", "dhparam", "-noout", "-text"],
           stdin => data_file("pkcs3-2-1024.pem"))),
   "stdinbuffer input test that uses BIO_gets");