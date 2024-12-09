
setup("test_x509");

plan tests => 21;

# Prevent MSys2 filename munging for arguments that look like file paths but
# aren't
$ENV{MSYS2_ARG_CONV_EXCL} = "/CN=";
ok(!run(app(["openssl", "x509", "-noout", "-dates", "-dateopt", "invalid_format",
	     "-in", srctop_file("test/certs", "ca-cert.pem")])),
   "Run with invalid -dateopt format");