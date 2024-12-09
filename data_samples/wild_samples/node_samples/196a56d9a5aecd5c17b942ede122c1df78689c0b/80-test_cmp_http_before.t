#! /usr/bin/env perl
# Copyright 2007-2021 The OpenSSL Project Authors. All Rights Reserved.
# Copyright Nokia 2007-2019
# Copyright Siemens AG 2015-2019
#
# Licensed under the Apache License 2.0 (the "License").  You may not use
# this file except in compliance with the License.  You can obtain a copy
# in the file LICENSE in the source distribution or at
# https://www.openssl.org/source/license.html

use strict;
use warnings;

use POSIX;
use OpenSSL::Test qw/:DEFAULT cmdstr data_file data_dir srctop_dir bldtop_dir result_dir/;
use OpenSSL::Test::Utils;

BEGIN {
    setup("test_cmp_http");
}
        foreach (@$tests) {
            test_cmp_http($server_name, $aspect, $n, $i++, $$_[0], $$_[1], $$_[2]);
            sleep($sleep);
        }
    };
    # not unlinking test.certout*.pem, test.cacerts.pem, and test.extracerts.pem
}

# The input files for the tests done here dynamically depend on the test server
# selected (where the Mock server used by default is just one possibility).
# On the other hand the main test configuration file test.cnf, which references
# several server-dependent input files by relative file names, is static.
# Moreover the tests use much greater variety of input files than output files.
# Therefore we chose the current directory as a subdirectory of $SRCTOP and it
# was simpler to prepend the output file names by BLDTOP than doing the tests
# from $BLDTOP/test-runs/test_cmp_http and prepending the input files by SRCTOP.

indir data_dir() => sub {
    plan tests => @server_configurations * @all_aspects
        + (grep(/^Mock$/, @server_configurations)
           && grep(/^certstatus$/, @all_aspects));

    foreach my $server_name (@server_configurations) {
        $server_name = chop_dblquot($server_name);
        load_config($server_name, $server_name);
        {
          SKIP: {
            my $pid;
            if ($server_name eq "Mock") {
                indir "Mock" => sub {
                    $pid = start_mock_server("");
                    die "Cannot start or find the started CMP mock server" unless $pid;
                }
            }
            foreach my $aspect (@all_aspects) {
                $aspect = chop_dblquot($aspect);
                next if $server_name eq "Mock" && $aspect eq "certstatus";
                load_config($server_name, $aspect); # update with any aspect-specific settings
                indir $server_name => sub {
                    my $tests = load_tests($server_name, $aspect);
                    test_cmp_http_aspect($server_name, $aspect, $tests);
                };
            };
            stop_mock_server($pid) if $pid;
          }
        }
    };
};

close($faillog) if $faillog;

sub load_tests {
    my $server_name = shift;
    my $aspect = shift;
    my $test_config = $ENV{OPENSSL_CMP_CONFIG} // "$server_name/test.cnf";
    my $file = data_file("test_$aspect.csv");
    my $result_dir = result_dir();
    my @result;

    open(my $data, '<', $file) || die "Cannot open $file for reading: $!";
  LOOP:
    while (my $line = <$data>) {
        chomp $line;
        $line =~ s{\r\n}{\n}g; # adjust line endings
        $line =~ s{_CA_DN}{$ca_dn}g;
        $line =~ s{_SERVER_DN}{$server_dn}g;
        $line =~ s{_SERVER_HOST}{$server_host}g;
        $line =~ s{_SERVER_PORT}{$server_port}g;
        $line =~ s{_SERVER_TLS}{$server_tls}g;
        $line =~ s{_SERVER_PATH}{$server_path}g;
        $line =~ s{_SERVER_CERT}{$server_cert}g;
        $line =~ s{_KUR_PORT}{$kur_port}g;
        $line =~ s{_PBM_PORT}{$pbm_port}g;
        $line =~ s{_PBM_REF}{$pbm_ref}g;
        $line =~ s{_PBM_SECRET}{$pbm_secret}g;
        $line =~ s{_RESULT_DIR}{$result_dir}g;

        next LOOP if $server_tls == 0 && $line =~ m/,\s*-tls_used\s*,/;
        my $noproxy = $no_proxy;
        if ($line =~ m/,\s*-no_proxy\s*,(.*?)(,|$)/) {
            $noproxy = $1;
        } elsif ($server_host eq "127.0.0.1") {
            # do connections to localhost (e.g., Mock server) without proxy
            $line =~ s{-section,,}{-section,,-no_proxy,127.0.0.1,} ;
        }
        if ($line =~ m/,\s*-proxy\s*,/) {
            next LOOP if $no_proxy && ($noproxy =~ $server_host);
        } else {
            $line =~ s{-section,,}{-section,,-proxy,$proxy,};
        }
        $line =~ s{-section,,}{-section,,-certout,$result_dir/test.cert.pem,};
        $line =~ s{-section,,}{-config,../$test_config,-section,$server_name $aspect,};

        my @fields = grep /\S/, split ",", $line;
        s/^<EMPTY>$// for (@fields); # used for proxy=""
        s/^\s+// for (@fields); # remove leading whitespace from elements
        s/\s+$// for (@fields); # remove trailing whitespace from elements
        s/^\"(\".*?\")\"$/$1/ for (@fields); # remove escaping from quotation marks from elements
        my $expected_result = $fields[$column];
        my $description = 1;
        my $title = $fields[$description];
        next LOOP if (!defined($expected_result)
                      || ($expected_result ne 0 && $expected_result ne 1));
        @fields = grep {$_ ne 'BLANK'} @fields[$description + 1 .. @fields - 1];
        push @result, [$title, \@fields, $expected_result];
    }
    close($data);
    return \@result;
}

sub start_mock_server {
    my $args = $_[0]; # optional further CLI arguments
    my $cmd = cmdstr(app([@app, '-config', 'server.cnf',
                          $args ? $args : ()]), display => 1);
    print "Current directory is ".getcwd()."\n";
    print "Launching mock server: $cmd\n";
    die "Invalid port: $server_port" unless $server_port =~ m/^\d+$/;
    my $pid = open($server_fh, "$cmd|") or die "Trying to $cmd";
    print "Pid is: $pid\n";
    if ($server_port == 0) {
        # Find out the actual server port
        while (<$server_fh>) {
            print "Server output: $_";
            next if m/using section/;
            s/\R$//;                # Better chomp
            ($server_port, $pid) = ($1, $2) if /^ACCEPT\s.*:(\d+) PID=(\d+)$/;
            last; # Do not loop further to prevent hangs on server misbehavior
        }
    }
    unless ($server_port > 0) {
        stop_mock_server($pid);
        return 0;
    }
    $server_tls = $kur_port = $pbm_port = $server_port;
    return $pid;
}

sub stop_mock_server {
    my $pid = $_[0];
    print "Killing mock server with pid=$pid\n";
    kill('KILL', $pid);
}
                indir $server_name => sub {
                    my $tests = load_tests($server_name, $aspect);
                    test_cmp_http_aspect($server_name, $aspect, $tests);
                };
            };
            stop_mock_server($pid) if $pid;
          }
        }
    };
};

close($faillog) if $faillog;

sub load_tests {
    my $server_name = shift;
    my $aspect = shift;
    my $test_config = $ENV{OPENSSL_CMP_CONFIG} // "$server_name/test.cnf";
    my $file = data_file("test_$aspect.csv");
    my $result_dir = result_dir();
    my @result;

    open(my $data, '<', $file) || die "Cannot open $file for reading: $!";
  LOOP:
    while (my $line = <$data>) {
        chomp $line;
        $line =~ s{\r\n}{\n}g; # adjust line endings
        $line =~ s{_CA_DN}{$ca_dn}g;
        $line =~ s{_SERVER_DN}{$server_dn}g;
        $line =~ s{_SERVER_HOST}{$server_host}g;
        $line =~ s{_SERVER_PORT}{$server_port}g;
        $line =~ s{_SERVER_TLS}{$server_tls}g;
        $line =~ s{_SERVER_PATH}{$server_path}g;
        $line =~ s{_SERVER_CERT}{$server_cert}g;
        $line =~ s{_KUR_PORT}{$kur_port}g;
        $line =~ s{_PBM_PORT}{$pbm_port}g;
        $line =~ s{_PBM_REF}{$pbm_ref}g;
        $line =~ s{_PBM_SECRET}{$pbm_secret}g;
        $line =~ s{_RESULT_DIR}{$result_dir}g;

        next LOOP if $server_tls == 0 && $line =~ m/,\s*-tls_used\s*,/;
        my $noproxy = $no_proxy;
        if ($line =~ m/,\s*-no_proxy\s*,(.*?)(,|$)/) {
            $noproxy = $1;
        } elsif ($server_host eq "127.0.0.1") {
            # do connections to localhost (e.g., Mock server) without proxy
            $line =~ s{-section,,}{-section,,-no_proxy,127.0.0.1,} ;
        }
        if ($line =~ m/,\s*-proxy\s*,/) {
            next LOOP if $no_proxy && ($noproxy =~ $server_host);
        } else {
            $line =~ s{-section,,}{-section,,-proxy,$proxy,};
        }
        $line =~ s{-section,,}{-section,,-certout,$result_dir/test.cert.pem,};
        $line =~ s{-section,,}{-config,../$test_config,-section,$server_name $aspect,};

        my @fields = grep /\S/, split ",", $line;
        s/^<EMPTY>$// for (@fields); # used for proxy=""
        s/^\s+// for (@fields); # remove leading whitespace from elements
        s/\s+$// for (@fields); # remove trailing whitespace from elements
        s/^\"(\".*?\")\"$/$1/ for (@fields); # remove escaping from quotation marks from elements
        my $expected_result = $fields[$column];
        my $description = 1;
        my $title = $fields[$description];
        next LOOP if (!defined($expected_result)
                      || ($expected_result ne 0 && $expected_result ne 1));
        @fields = grep {$_ ne 'BLANK'} @fields[$description + 1 .. @fields - 1];
        push @result, [$title, \@fields, $expected_result];
    }
    close($data);
    return \@result;
}
sub stop_mock_server {
    my $pid = $_[0];
    print "Killing mock server with pid=$pid\n";
    kill('KILL', $pid);
}