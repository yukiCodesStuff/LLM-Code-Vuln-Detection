
setup("test_dgst");

plan tests => 10;

sub tsignverify {
    my $testtext = shift;
    my $privkey = shift;
    ok($xofdata[1] =~ $expected,
       "XOF: Check second digest value is consistent with the first ($xofdata[1]) vs ($expected)");
};