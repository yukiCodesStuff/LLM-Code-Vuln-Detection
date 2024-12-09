
setup("test_dgst");

plan tests => 12;

sub tsignverify {
    my $testtext = shift;
    my $privkey = shift;
    ok($xofdata[1] =~ $expected,
       "XOF: Check second digest value is consistent with the first ($xofdata[1]) vs ($expected)");
};

subtest "SHAKE digest generation with no xoflen set `dgst` CLI" => sub {
    plan tests => 1;

    my $testdata = srctop_file('test', 'data.bin');
    my @xofdata = run(app(['openssl', 'dgst', '-shake128', $testdata], stderr => "outerr.txt"), capture => 1);
    chomp(@xofdata);
    my $expected = qr/SHAKE-128\(\Q$testdata\E\)= bb565dac72640109e1c926ef441d3fa6/;
    ok($xofdata[0] =~ $expected, "Check short digest is output");
};

SKIP: {
    skip "ECDSA is not supported by this OpenSSL build", 1
        if disabled("ec");

    subtest "signing with xoflen is not supported `dgst` CLI" => sub {
        plan tests => 1;
        my $data_to_sign = srctop_file('test', 'data.bin');

        ok(!run(app(['openssl', 'dgst', '-shake256', '-xoflen', '64',
                     '-sign', srctop_file("test","testec-p256.pem"),
                     '-out', 'test.sig',
                     srctop_file('test', 'data.bin')])),
                     "Generating signature with xoflen should fail");
    }
}