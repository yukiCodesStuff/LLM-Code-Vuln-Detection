sub pubfrompriv {
    my $prefix = shift;
    my $key = shift;
    my $pub_key = shift;
    my $type = shift;

    ok(run(app(['openssl', 'pkey',
                '-in', $key,
                '-pubout',
                '-out', $pub_key])),
        $prefix.': '."Create the public key with $type parameters");

}

my $tsignverify_count = 8;
sub tsignverify {
    my $prefix = shift;
    my $fips_key = shift;
    my $fips_pub_key = shift;
    my $nonfips_key = shift;
    my $nonfips_pub_key = shift;
    my $fips_sigfile = $prefix.'.fips.sig';
    my $nonfips_sigfile = $prefix.'.nonfips.sig';
    my $sigfile = '';
    my $testtext = '';

    $ENV{OPENSSL_CONF} = $fipsconf;

    $sigfile = $fips_sigfile;
    $testtext = $prefix.': '.
        'Sign something with a FIPS key';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-sign', $fips_key,
                '-out', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a FIPS key';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-verify', $fips_pub_key,
                '-signature', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $fips_pub_key,
                 '-signature', $sigfile,
                 $bogus_data])),
       $testtext);

    $ENV{OPENSSL_CONF} = $defaultconf;

    $sigfile = $nonfips_sigfile;
    $testtext = $prefix.': '.
        'Sign something with a non-FIPS key'.
        ' with the default provider';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-sign', $nonfips_key,
                '-out', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a non-FIPS key'.
        ' with the default provider';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-verify', $nonfips_pub_key,
                '-signature', $sigfile,
                $tbs_data])),
       $testtext);

    $ENV{OPENSSL_CONF} = $fipsconf;

    $testtext = $prefix.': '.
        'Sign something with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-sign', $nonfips_key,
                 '-out', $prefix.'.nonfips.fail.sig',
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $nonfips_pub_key,
                 '-signature', $sigfile,
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $nonfips_pub_key,
                 '-signature', $sigfile,
                 $bogus_data])),
       $testtext);
}

SKIP : {
    skip "FIPS EC tests because of no ec in this build", 1
        if disabled("ec");

    subtest EC => sub {
        my $testtext_prefix = 'EC';
        my $a_fips_curve = 'prime256v1';
        my $fips_key = $testtext_prefix.'.fips.priv.pem';
        my $fips_pub_key = $testtext_prefix.'.fips.pub.pem';
        my $a_nonfips_curve = 'brainpoolP256r1';
        my $nonfips_key = $testtext_prefix.'.nonfips.priv.pem';
        my $nonfips_pub_key = $testtext_prefix.'.nonfips.pub.pem';
        my $testtext = '';
        my $curvename = '';

        plan tests => 5 + $tsignverify_count;

        $ENV{OPENSSL_CONF} = $defaultconf;
        $curvename = $a_nonfips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a non-FIPS algorithm with the default provider';
        ok(run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                    '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                    '-out', $nonfips_key])),
           $testtext);

        pubfrompriv($testtext_prefix, $nonfips_key, $nonfips_pub_key, "non-FIPS");

        $ENV{OPENSSL_CONF} = $fipsconf;

        $curvename = $a_fips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a FIPS algorithm';
        ok(run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                    '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                    '-out', $fips_key])),
           $testtext);

        pubfrompriv($testtext_prefix, $fips_key, $fips_pub_key, "FIPS");

        $curvename = $a_nonfips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a non-FIPS algorithm'.
            ' (should fail)';
        ok(!run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                     '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                     '-out', $testtext_prefix.'.'.$curvename.'.priv.pem'])),
           $testtext);

        tsignverify($testtext_prefix, $fips_key, $fips_pub_key, $nonfips_key,
                    $nonfips_pub_key);
    };
}
sub tsignverify {
    my $prefix = shift;
    my $fips_key = shift;
    my $fips_pub_key = shift;
    my $nonfips_key = shift;
    my $nonfips_pub_key = shift;
    my $fips_sigfile = $prefix.'.fips.sig';
    my $nonfips_sigfile = $prefix.'.nonfips.sig';
    my $sigfile = '';
    my $testtext = '';

    $ENV{OPENSSL_CONF} = $fipsconf;

    $sigfile = $fips_sigfile;
    $testtext = $prefix.': '.
        'Sign something with a FIPS key';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-sign', $fips_key,
                '-out', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a FIPS key';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-verify', $fips_pub_key,
                '-signature', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $fips_pub_key,
                 '-signature', $sigfile,
                 $bogus_data])),
       $testtext);

    $ENV{OPENSSL_CONF} = $defaultconf;

    $sigfile = $nonfips_sigfile;
    $testtext = $prefix.': '.
        'Sign something with a non-FIPS key'.
        ' with the default provider';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-sign', $nonfips_key,
                '-out', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a non-FIPS key'.
        ' with the default provider';
    ok(run(app(['openssl', 'dgst', '-sha256',
                '-verify', $nonfips_pub_key,
                '-signature', $sigfile,
                $tbs_data])),
       $testtext);

    $ENV{OPENSSL_CONF} = $fipsconf;

    $testtext = $prefix.': '.
        'Sign something with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-sign', $nonfips_key,
                 '-out', $prefix.'.nonfips.fail.sig',
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $nonfips_pub_key,
                 '-signature', $sigfile,
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',
                 '-verify', $nonfips_pub_key,
                 '-signature', $sigfile,
                 $bogus_data])),
       $testtext);
}

SKIP : {
    skip "FIPS EC tests because of no ec in this build", 1
        if disabled("ec");

    subtest EC => sub {
        my $testtext_prefix = 'EC';
        my $a_fips_curve = 'prime256v1';
        my $fips_key = $testtext_prefix.'.fips.priv.pem';
        my $fips_pub_key = $testtext_prefix.'.fips.pub.pem';
        my $a_nonfips_curve = 'brainpoolP256r1';
        my $nonfips_key = $testtext_prefix.'.nonfips.priv.pem';
        my $nonfips_pub_key = $testtext_prefix.'.nonfips.pub.pem';
        my $testtext = '';
        my $curvename = '';

        plan tests => 5 + $tsignverify_count;

        $ENV{OPENSSL_CONF} = $defaultconf;
        $curvename = $a_nonfips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a non-FIPS algorithm with the default provider';
        ok(run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                    '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                    '-out', $nonfips_key])),
           $testtext);

        pubfrompriv($testtext_prefix, $nonfips_key, $nonfips_pub_key, "non-FIPS");

        $ENV{OPENSSL_CONF} = $fipsconf;

        $curvename = $a_fips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a FIPS algorithm';
        ok(run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                    '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                    '-out', $fips_key])),
           $testtext);

        pubfrompriv($testtext_prefix, $fips_key, $fips_pub_key, "FIPS");

        $curvename = $a_nonfips_curve;
        $testtext = $testtext_prefix.': '.
            'Generate a key with a non-FIPS algorithm'.
            ' (should fail)';
        ok(!run(app(['openssl', 'genpkey', '-algorithm', 'EC',
                     '-pkeyopt', 'ec_paramgen_curve:'.$curvename,
                     '-out', $testtext_prefix.'.'.$curvename.'.priv.pem'])),
           $testtext);

        tsignverify($testtext_prefix, $fips_key, $fips_pub_key, $nonfips_key,
                    $nonfips_pub_key);
    };
}