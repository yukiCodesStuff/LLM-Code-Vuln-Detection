
setup("test_x509");

plan tests => 28;

# Prevent MSys2 filename munging for arguments that look like file paths but
# aren't
$ENV{MSYS2_ARG_CONV_EXCL} = "/CN=";
ok(!run(app(["openssl", "x509", "-noout", "-dates", "-dateopt", "invalid_format",
	     "-in", srctop_file("test/certs", "ca-cert.pem")])),
   "Run with invalid -dateopt format");

# extracts issuer from a -text formatted-output
sub get_issuer {
    my $f = shift(@_);
    my $issuer = "";
    open my $fh, $f or die;
    while (my $line = <$fh>) {
        if ($line =~ /Issuer:/) {
            $issuer = $line;
        }
    }
    close $fh;
    return $issuer;
}

# Tests for signing certs (broken in 1.1.1o)
my $a_key = "a-key.pem";
my $a_cert = "a-cert.pem";
my $a2_cert = "a2-cert.pem";
my $ca_key = "ca-key.pem";
my $ca_cert = "ca-cert.pem";
my $cnf = srctop_file('apps', 'openssl.cnf');

# Create cert A
ok(run(app(["openssl", "req", "-x509", "-newkey", "rsa:2048",
            "-config", $cnf,
            "-keyout", $a_key, "-out", $a_cert, "-days", "365",
            "-nodes", "-subj", "/CN=test.example.com"])));
# Create cert CA - note key size
ok(run(app(["openssl", "req", "-x509", "-newkey", "rsa:4096",
            "-config", $cnf,
            "-keyout", $ca_key, "-out", $ca_cert, "-days", "3650",
            "-nodes", "-subj", "/CN=ca.example.com"])));
# Sign cert A with CA (errors on 1.1.1o)
ok(run(app(["openssl", "x509", "-in", $a_cert, "-CA", $ca_cert,
            "-CAkey", $ca_key, "-set_serial", "1234567890",
            "-preserve_dates", "-sha256", "-text", "-out", $a2_cert])));
# verify issuer is CA
ok (get_issuer($a2_cert) =~ /CN = ca.example.com/);

# Tests for issue #16080 (fixed in 1.1.1o)
my $b_key = "b-key.pem";
my $b_csr = "b-cert.csr";
my $b_cert = "b-cert.pem";
# Create the CSR
ok(run(app(["openssl", "req", "-new", "-newkey", "rsa:4096",
            "-keyout", $b_key, "-out", $b_csr, "-nodes",
            "-config", $cnf,
            "-subj", "/CN=b.example.com"])));
# Sign it - position of "-text" matters!
ok(run(app(["openssl", "x509", "-req", "-text", "-CAcreateserial",
            "-CA", $ca_cert, "-CAkey", $ca_key,
            "-in", $b_csr, "-out", $b_cert])));
# Verify issuer is CA
ok(get_issuer($b_cert) =~ /CN = ca.example.com/);