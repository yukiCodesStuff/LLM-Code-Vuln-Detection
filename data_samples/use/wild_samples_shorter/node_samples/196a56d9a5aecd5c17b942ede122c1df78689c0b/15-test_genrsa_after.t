my $no_fips = disabled('fips') || ($ENV{NO_FIPS} // 0);

plan tests =>
    ($no_fips ? 0 : 5)          # Extra FIPS related tests
    + 15;

# We want to know that an absurdly small number of bits isn't support
is(run(app([ 'openssl', 'genpkey', '-out', 'genrsatest.pem',
                '-out', 'genrsatest3072.pem'])),
       "Generating RSA key with 3072 bits");

   ok(!run(app(['openssl', 'genrsa', @prov, '512'])),
       "Generating RSA key with 512 bits should fail in FIPS provider");

   ok(!run(app(['openssl', 'genrsa',
                @prov,
                '-provider', 'default',
                '-propquery', '?fips!=yes',
                '512'])),
       "Generating RSA key with 512 bits should succeed with FIPS provider as".
       " default with a non-FIPS property query");

    # We want to know that an absurdly large number of bits fails the RNG check
    is(run(app([ 'openssl', 'genpkey',
                 @prov,
                 '-algorithm', 'RSA',