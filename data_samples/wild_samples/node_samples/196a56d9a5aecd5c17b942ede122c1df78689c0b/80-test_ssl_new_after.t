#! /usr/bin/env perl
# Copyright 2015-2022 The OpenSSL Project Authors. All Rights Reserved.
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html

# For manually running these tests, set specific environment variables like this:
# CTLOG_FILE=test/ct/log_list.cnf
# TEST_CERTS_DIR=test/certs
# For details on the environment variables needed, see test/README.ssltest.md

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

      my $msg = "running CTLOG_FILE=test/ct/log_list.cnf". # $ENV{CTLOG_FILE}.
          " TEST_CERTS_DIR=test/certs". # $ENV{TEST_CERTS_DIR}.
          " test/ssl_test test/ssl-tests/$conf $provider";
      if ($provider eq "fips") {
          ok(run(test(["ssl_test", $output_file, $provider,
                       srctop_file("test", "fips-and-base.cnf")])), $msg);
      } else {
          ok(run(test(["ssl_test", $output_file, $provider])), $msg);
      }
    }