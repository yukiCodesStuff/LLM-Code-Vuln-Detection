subtest "Check loading of fips and non-fips params" => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;
    plan tests => 3;

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

    $ENV{OPENSSL_CONF} = $defaultconf;
};
subtest "Check loading of fips and non-fips params" => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;
    plan tests => 3;

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

    $ENV{OPENSSL_CONF} = $defaultconf;
};