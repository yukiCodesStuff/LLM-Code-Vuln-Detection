subtest "Check loading of fips and non-fips params" => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;
    plan tests => 8;

    my $fipsconf = srctop_file("test", "fips-and-base.cnf");
    my $defaultconf = srctop_file("test", "default.cnf");

    $ENV{OPENSSL_CONF} = $fipsconf;

    ok(run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp384r1-explicit.pem'),
                '-check'])),
       "Loading explicitly encoded valid curve");

    ok(run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp384r1-named.pem'),
                '-check'])),
       "Loading named valid curve");

    ok(!run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Fail loading named non-fips curve");

    ok(!run(app(['openssl', 'pkeyparam',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Fail loading named non-fips curve using pkeyparam");

    ok(run(app(['openssl', 'ecparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Loading named non-fips curve in FIPS mode with non-FIPS property".
       " query");

    ok(run(app(['openssl', 'pkeyparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Loading named non-fips curve in FIPS mode with non-FIPS property".
       " query using pkeyparam");

    ok(!run(app(['openssl', 'ecparam',
                '-genkey', '-name', 'secp112r1'])),
       "Fail generating key for named non-fips curve");

    ok(run(app(['openssl', 'ecparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-genkey', '-name', 'secp112r1'])),
       "Generating key for named non-fips curve with non-FIPS property query");

    $ENV{OPENSSL_CONF} = $defaultconf;
};
subtest "Check loading of fips and non-fips params" => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;
    plan tests => 8;

    my $fipsconf = srctop_file("test", "fips-and-base.cnf");
    my $defaultconf = srctop_file("test", "default.cnf");

    $ENV{OPENSSL_CONF} = $fipsconf;

    ok(run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp384r1-explicit.pem'),
                '-check'])),
       "Loading explicitly encoded valid curve");

    ok(run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp384r1-named.pem'),
                '-check'])),
       "Loading named valid curve");

    ok(!run(app(['openssl', 'ecparam',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Fail loading named non-fips curve");

    ok(!run(app(['openssl', 'pkeyparam',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Fail loading named non-fips curve using pkeyparam");

    ok(run(app(['openssl', 'ecparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Loading named non-fips curve in FIPS mode with non-FIPS property".
       " query");

    ok(run(app(['openssl', 'pkeyparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-in', data_file('valid', 'secp112r1-named.pem'),
                '-check'])),
       "Loading named non-fips curve in FIPS mode with non-FIPS property".
       " query using pkeyparam");

    ok(!run(app(['openssl', 'ecparam',
                '-genkey', '-name', 'secp112r1'])),
       "Fail generating key for named non-fips curve");

    ok(run(app(['openssl', 'ecparam',
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '-genkey', '-name', 'secp112r1'])),
       "Generating key for named non-fips curve with non-FIPS property query");

    $ENV{OPENSSL_CONF} = $defaultconf;
};