#! /usr/bin/env perl
# Copyright 2017-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use File::Spec;
use File::Basename;
use OpenSSL::Test qw/:DEFAULT with srctop_file bldtop_dir/;
use OpenSSL::Test::Utils;

setup("test_dgst");

plan tests => 12;

sub tsignverify {
    my $testtext = shift;
    my $privkey = shift;
    my $pubkey = shift;

    my $data_to_sign = srctop_file('test', 'data.bin');
    my $other_data = srctop_file('test', 'data2.bin');

    my $sigfile = basename($privkey, '.pem') . '.sig';
    plan tests => 4;

    ok(run(app(['openssl', 'dgst', '-sign', $privkey,
                '-out', $sigfile,
                $data_to_sign])),
       $testtext.": Generating signature");

    ok(run(app(['openssl', 'dgst', '-prverify', $privkey,
                '-signature', $sigfile,
                $data_to_sign])),
       $testtext.": Verify signature with private key");

    ok(run(app(['openssl', 'dgst', '-verify', $pubkey,
                '-signature', $sigfile,
                $data_to_sign])),
       $testtext.": Verify signature with public key");

    ok(!run(app(['openssl', 'dgst', '-verify', $pubkey,
                 '-signature', $sigfile,
                 $other_data])),
       $testtext.": Expect failure verifying mismatching data");
}
subtest "Custom length XOF digest generation with `dgst` CLI" => sub {
    plan tests => 2;

    my $testdata = srctop_file('test', 'data.bin');
    #Digest the data twice to check consistency
    my @xofdata = run(app(['openssl', 'dgst', '-shake128', '-xoflen', '64',
                           $testdata, $testdata]), capture => 1);
    chomp(@xofdata);
    my $expected = qr/SHAKE-128\(\Q$testdata\E\)= bb565dac72640109e1c926ef441d3fa64ffd0b3e2bf8cd73d5182dfba19b6a8a2eab96d2df854b647b3795ef090582abe41ba4e0717dc4df40bc4e17d88e4677/;
    ok($xofdata[0] =~ $expected, "XOF: Check digest value is as expected ($xofdata[0]) vs ($expected)");
    ok($xofdata[1] =~ $expected,
       "XOF: Check second digest value is consistent with the first ($xofdata[1]) vs ($expected)");
};

subtest "SHAKE digest generation with no xoflen set `dgst` CLI" => sub {
    plan tests => 1;

    my $testdata = srctop_file('test', 'data.bin');
    my @xofdata = run(app(['openssl', 'dgst', '-shake128', $testdata], stderr => "outerr.txt"), capture => 1);
    chomp(@xofdata);
    my $expected = qr/SHAKE-128\(\Q$testdata\E\)= bb565dac72640109e1c926ef441d3fa6/;
    ok($xofdata[0] =~ $expected, "Check short digest is output");
};

SKIP: {
    skip "ECDSA is not supported by this OpenSSL build", 1
        if disabled("ec");

    subtest "signing with xoflen is not supported `dgst` CLI" => sub {
        plan tests => 1;
        my $data_to_sign = srctop_file('test', 'data.bin');

        ok(!run(app(['openssl', 'dgst', '-shake256', '-xoflen', '64',
                     '-sign', srctop_file("test","testec-p256.pem"),
                     '-out', 'test.sig',
                     srctop_file('test', 'data.bin')])),
                     "Generating signature with xoflen should fail");
    }
}