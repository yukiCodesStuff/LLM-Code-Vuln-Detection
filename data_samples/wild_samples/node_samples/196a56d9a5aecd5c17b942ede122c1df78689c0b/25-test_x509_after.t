#! /usr/bin/env perl
# Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use File::Spec;
use OpenSSL::Test::Utils;
use OpenSSL::Test qw/:DEFAULT srctop_file/;

setup("test_x509");

plan tests => 28;

# Prevent MSys2 filename munging for arguments that look like file paths but
# aren't
$ENV{MSYS2_ARG_CONV_EXCL} = "/CN=";

require_ok(srctop_file("test", "recipes", "tconversion.pl"));

my @certs = qw(test certs);
my $pem = srctop_file(@certs, "cyrillic.pem");
my $out_msb = "out-cyrillic.msb";
my $out_utf8 = "out-cyrillic.utf8";
my $der = "cyrillic.der";
my $der2 = "cyrillic.der";
my $msb = srctop_file(@certs, "cyrillic.msb");
my $utf = srctop_file(@certs, "cyrillic.utf8");

ok(run(app(["openssl", "x509", "-text", "-in", $pem, "-out", $out_msb,
            "-nameopt", "esc_msb"])));
is(cmp_text($out_msb, $msb),
   0, 'Comparing esc_msb output with cyrillic.msb');
ok(run(app(["openssl", "x509", "-text", "-in", $pem, "-out", $out_utf8,
            "-nameopt", "utf8"])));
is(cmp_text($out_utf8, $utf),
   0, 'Comparing utf8 output with cyrillic.utf8');

SKIP: {
    skip "DES disabled", 1 if disabled("des");
    skip "Platform doesn't support command line UTF-8", 1 if $^O =~ /^(VMS|msys)$/;

    my $p12 = srctop_file("test", "shibboleth.pfx");
    my $p12pass = "σύνθημα γνώρισμα";
    my $out_pem = "out.pem";
    ok(run(app(["openssl", "x509", "-text", "-in", $p12, "-out", $out_pem,
                "-passin", "pass:$p12pass"])));
    # not unlinking $out_pem
}
SKIP: {
    skip "sm2 not disabled", 1 if !disabled("sm2");

    ok(test_errors("Unable to load Public Key", "sm2.pem", '-text'),
       "error loading unsupported sm2 cert");
}

# 3 tests for -dateopts formats
ok(run(app(["openssl", "x509", "-noout", "-dates", "-dateopt", "rfc_822",
	     "-in", srctop_file("test/certs", "ca-cert.pem")])),
   "Run with rfc_8222 -dateopt format");
ok(run(app(["openssl", "x509", "-noout", "-dates", "-dateopt", "iso_8601",
	     "-in", srctop_file("test/certs", "ca-cert.pem")])),
   "Run with iso_8601 -dateopt format");
ok(!run(app(["openssl", "x509", "-noout", "-dates", "-dateopt", "invalid_format",
	     "-in", srctop_file("test/certs", "ca-cert.pem")])),
   "Run with invalid -dateopt format");

# extracts issuer from a -text formatted-output
sub get_issuer {
    my $f = shift(@_);
    my $issuer = "";
    open my $fh, $f or die;
    while (my $line = <$fh>) {
        if ($line =~ /Issuer:/) {
            $issuer = $line;
        }
    }
    close $fh;
    return $issuer;
}