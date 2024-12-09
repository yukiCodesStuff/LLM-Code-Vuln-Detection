#! /usr/bin/env perl
# Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at

plan skip_all => 'EC is not supported in this build' if disabled('ec');

plan tests => 14;

require_ok(srctop_file('test','recipes','tconversion.pl'));

ok(run(test(["ectest"])), "running ectest");
                 -in => srctop_file("test", "tested448pub.pem"),
                 -args => ["pkey", "-pubin", "-pubout"] );
};