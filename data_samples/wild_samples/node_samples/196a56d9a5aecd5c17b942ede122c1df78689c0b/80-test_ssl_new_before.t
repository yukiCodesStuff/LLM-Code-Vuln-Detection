#! /usr/bin/env perl
# Copyright 2015-2021 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html


use strict;
use warnings;

use File::Basename;
use File::Compare qw/compare_text/;
use OpenSSL::Glob;
use OpenSSL::Test qw/:DEFAULT srctop_dir srctop_file bldtop_file bldtop_dir/;
use OpenSSL::Test::Utils qw/disabled alldisabled available_protocols/;

BEGIN {
setup("test_ssl_new");
}
    SKIP: {
        # Test 2. Compare against existing output in test/ssl-tests/
        skip "Skipping generated source test for $conf", 1
          if !$check_source;

        $run_test = is(cmp_text($output_file, $conf_file), 0,
                       "Comparing generated $output_file with $conf_file.");
      }

      # Test 3. Run the test.
      skip "No tests available; skipping tests", 1 if $skip;
      skip "Stale sources; skipping tests", 1 if !$run_test;

      if ($provider eq "fips") {
          ok(run(test(["ssl_test", $output_file, $provider,
                       srctop_file("test", "fips-and-base.cnf")])),
             "running ssl_test $conf");
      } else {
          ok(run(test(["ssl_test", $output_file, $provider])),
             "running ssl_test $conf");
      }
    }