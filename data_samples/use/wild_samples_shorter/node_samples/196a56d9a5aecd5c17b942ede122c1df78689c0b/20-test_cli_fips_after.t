
}

my $tsignverify_count = 9;
sub tsignverify {
    my $prefix = shift;
    my $fips_key = shift;
    my $fips_pub_key = shift;
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify something with a non-FIPS key'.
		' in FIPS mode but with a non-FIPS property query';
    ok(run(app(['openssl', 'dgst',
				'-provider', 'default',
				'-propquery', '?fips!=yes',
				'-sha256',
                '-verify', $nonfips_pub_key,
                '-signature', $sigfile,
                $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',