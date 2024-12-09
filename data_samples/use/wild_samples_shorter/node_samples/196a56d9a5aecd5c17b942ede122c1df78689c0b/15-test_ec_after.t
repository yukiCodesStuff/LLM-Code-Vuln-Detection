#! /usr/bin/env perl
# Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at

plan skip_all => 'EC is not supported in this build' if disabled('ec');

plan tests => 15;

my $no_fips = disabled('fips') || ($ENV{NO_FIPS} // 0);

require_ok(srctop_file('test','recipes','tconversion.pl'));

ok(run(test(["ectest"])), "running ectest");
                 -in => srctop_file("test", "tested448pub.pem"),
                 -args => ["pkey", "-pubin", "-pubout"] );
};

subtest 'Check loading of fips and non-fips keys' => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;

    plan tests => 2;

    my $fipsconf = srctop_file("test", "fips-and-base.cnf");
    $ENV{OPENSSL_CONF} = $fipsconf;

    ok(!run(app(['openssl', 'pkey',
                 '-check', '-in', srctop_file("test", "testec-p112r1.pem")])),
        "Checking non-fips curve key fails in FIPS provider");

    ok(run(app(['openssl', 'pkey',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-check', '-in', srctop_file("test", "testec-p112r1.pem")])),
        "Checking non-fips curve key succeeds with non-fips property query");

    delete $ENV{OPENSSL_CONF};
}