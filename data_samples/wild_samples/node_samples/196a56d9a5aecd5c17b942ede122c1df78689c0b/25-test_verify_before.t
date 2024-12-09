SKIP: {
    my $no_fips = disabled('fips') || ($ENV{NO_FIPS} // 0);
    skip "EC is not supported or FIPS is disabled", 3
        if disabled("ec") || $no_fips;

    my $provconf = srctop_file("test", "fips-and-base.cnf");
    my $provpath = bldtop_dir("providers");
    my @prov = ("-provider-path", $provpath);
    $ENV{OPENSSL_CONF} = $provconf;

    ok(!verify("ee-cert-ec-explicit", "", ["root-cert"],
               ["ca-cert-ec-named"], @prov),
        "reject explicit curve leaf with named curve intermediate w/fips");
    ok(!verify("ee-cert-ec-named-explicit", "", ["root-cert"],
               ["ca-cert-ec-explicit"], @prov),
        "reject named curve leaf with explicit curve intermediate w/fips");
    ok(verify("ee-cert-ec-named-named", "", ["root-cert"],
              ["ca-cert-ec-named"], @prov),
        "accept named curve leaf with named curve intermediate w/fips");

    delete $ENV{OPENSSL_CONF};
}