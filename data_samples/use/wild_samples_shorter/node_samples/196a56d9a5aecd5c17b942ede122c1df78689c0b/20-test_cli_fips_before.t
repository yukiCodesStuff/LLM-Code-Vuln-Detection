
}

my $tsignverify_count = 8;
sub tsignverify {
    my $prefix = shift;
    my $fips_key = shift;
    my $fips_pub_key = shift;
                 $tbs_data])),
       $testtext);

    $testtext = $prefix.': '.
        'Verify a valid signature against the wrong data with a non-FIPS key'.
        ' (should fail)';
    ok(!run(app(['openssl', 'dgst', '-sha256',