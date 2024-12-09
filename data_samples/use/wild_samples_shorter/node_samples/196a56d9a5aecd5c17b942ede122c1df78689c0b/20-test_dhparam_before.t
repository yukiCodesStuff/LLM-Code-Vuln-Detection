#! /usr/bin/env perl
# Copyright 2020-2021 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
use strict;
use warnings;

use OpenSSL::Test qw(:DEFAULT data_file);
use OpenSSL::Test::Utils;

#Tests for the dhparam CLI application


plan skip_all => "DH is not supported in this build"
    if disabled("dh");
plan tests => 17;

sub checkdhparams {
    my $file = shift; #Filename containing params
    my $type = shift; #PKCS3 or X9.42?
        checkdhparams("gen-x942-0-512.der", "X9.42", 0, "DER", 512);
    };
}

ok(run(app(["openssl", "dhparam", "-noout", "-text"],
           stdin => data_file("pkcs3-2-1024.pem"))),
   "stdinbuffer input test that uses BIO_gets");