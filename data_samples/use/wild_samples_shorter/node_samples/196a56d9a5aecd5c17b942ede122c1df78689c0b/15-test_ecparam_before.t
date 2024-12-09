subtest "Check loading of fips and non-fips params" => sub {
    plan skip_all => "FIPS is disabled"
        if $no_fips;
    plan tests => 3;

    my $fipsconf = srctop_file("test", "fips-and-base.cnf");
    my $defaultconf = srctop_file("test", "default.cnf");

                '-check'])),
       "Fail loading named non-fips curve");

    $ENV{OPENSSL_CONF} = $defaultconf;
};