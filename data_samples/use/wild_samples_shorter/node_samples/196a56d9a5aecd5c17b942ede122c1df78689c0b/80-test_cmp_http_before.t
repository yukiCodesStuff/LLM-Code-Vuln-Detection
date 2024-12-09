#! /usr/bin/env perl
# Copyright 2007-2021 The OpenSSL Project Authors. All Rights Reserved.
# Copyright Nokia 2007-2019
# Copyright Siemens AG 2015-2019
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# from $BLDTOP/test-runs/test_cmp_http and prepending the input files by SRCTOP.

indir data_dir() => sub {
    plan tests => @server_configurations * @all_aspects
        + (grep(/^Mock$/, @server_configurations)
           && grep(/^certstatus$/, @all_aspects));

    foreach my $server_name (@server_configurations) {
        $server_name = chop_dblquot($server_name);
                };
            };
            stop_mock_server($pid) if $pid;
          }
        }
    };
};
    my $pid = $_[0];
    print "Killing mock server with pid=$pid\n";
    kill('KILL', $pid);
}