my $no_fips = disabled('fips') || ($ENV{NO_FIPS} // 0);

plan tests =>
    ($no_fips ? 0 : 3)          # Extra FIPS related tests
    + 15;

# We want to know that an absurdly small number of bits isn't support
is(run(app([ 'openssl', 'genpkey', '-out', 'genrsatest.pem',
                '-out', 'genrsatest3072.pem'])),
       "Generating RSA key with 3072 bits");

    # We want to know that an absurdly large number of bits fails the RNG check
    is(run(app([ 'openssl', 'genpkey',
                 @prov,
                 '-algorithm', 'RSA',