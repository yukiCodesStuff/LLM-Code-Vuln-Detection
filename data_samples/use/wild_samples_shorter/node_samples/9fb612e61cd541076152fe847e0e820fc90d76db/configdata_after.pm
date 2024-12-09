    ],
    "dynamic_engines" => "0",
    "ex_libs" => [],
    "full_version" => "3.0.7+quic",
    "includes" => [],
    "lflags" => [],
    "lib_defines" => [
        "OPENSSL_PIC"
    "openssl_sys_defines" => [],
    "openssldir" => "",
    "options" => "enable-ssl-trace enable-fips no-afalgeng no-asan no-asm no-buildtest-c++ no-comp no-crypto-mdebug no-crypto-mdebug-backtrace no-devcryptoeng no-dynamic-engine no-ec_nistp_64_gcc_128 no-egd no-external-tests no-fuzz-afl no-fuzz-libfuzzer no-ktls no-loadereng no-md2 no-msan no-rc5 no-sctp no-shared no-ssl3 no-ssl3-method no-trace no-ubsan no-unit-test no-uplink no-weak-ssl-ciphers no-zlib no-zlib-dynamic",
    "patch" => "7",
    "perl_archname" => "x86_64-linux-gnu-thread-multi",
    "perl_cmd" => "/usr/bin/perl",
    "perl_version" => "5.30.0",
    "perlargv" => [
    "prerelease" => "",
    "processor" => "",
    "rc4_int" => "unsigned char",
    "release_date" => "1 Nov 2022",
    "shlib_version" => "81.3",
    "sourcedir" => ".",
    "target" => "linux64-s390x",
    "version" => "3.0.7"
);
our %target = (
    "AR" => "ar",
    "ARFLAGS" => "qc",
            "test/exptest" => {
                "noinst" => "1"
            },
            "test/ext_internal_test" => {
                "noinst" => "1"
            },
            "test/fatalerrtest" => {
                "noinst" => "1"
            },
            "test/ffc_internal_test" => {
                "noinst" => "1"
            },
            "test/fips_version_test" => {
                "noinst" => "1"
            },
            "test/gmdifftest" => {
                "noinst" => "1"
            },
            "test/hexstr_test" => {
            "test/provider_test" => {
                "noinst" => "1"
            },
            "test/punycode_test" => {
                "noinst" => "1"
            },
            "test/rand_status_test" => {
                "noinst" => "1"
            },
            "test/rand_test" => {
        "doc/html/man3/OPENSSL_fork_prepare.html" => [
            "doc/man3/OPENSSL_fork_prepare.pod"
        ],
        "doc/html/man3/OPENSSL_gmtime.html" => [
            "doc/man3/OPENSSL_gmtime.pod"
        ],
        "doc/html/man3/OPENSSL_hexchar2int.html" => [
            "doc/man3/OPENSSL_hexchar2int.pod"
        ],
        "doc/html/man3/OPENSSL_ia32cap.html" => [
        "doc/man/man3/OPENSSL_fork_prepare.3" => [
            "doc/man3/OPENSSL_fork_prepare.pod"
        ],
        "doc/man/man3/OPENSSL_gmtime.3" => [
            "doc/man3/OPENSSL_gmtime.pod"
        ],
        "doc/man/man3/OPENSSL_hexchar2int.3" => [
            "doc/man3/OPENSSL_hexchar2int.pod"
        ],
        "doc/man/man3/OPENSSL_ia32cap.3" => [
            "libcrypto",
            "test/libtestutil.a"
        ],
        "test/ext_internal_test" => [
            "libcrypto.a",
            "libssl.a",
            "test/libtestutil.a"
        ],
        "test/fatalerrtest" => [
            "libcrypto",
            "libssl",
            "test/libtestutil.a"
            "libcrypto.a",
            "test/libtestutil.a"
        ],
        "test/fips_version_test" => [
            "libcrypto",
            "test/libtestutil.a"
        ],
        "test/gmdifftest" => [
            "libcrypto",
            "test/libtestutil.a"
        ],
            "libcrypto.a",
            "test/libtestutil.a"
        ],
        "test/punycode_test" => [
            "libcrypto.a",
            "test/libtestutil.a"
        ],
        "test/rand_status_test" => [
            "libcrypto",
            "test/libtestutil.a"
        ],
                "providers/implementations/digests/libdefault-lib-md5_prov.o",
                "providers/implementations/digests/libdefault-lib-md5_sha1_prov.o",
                "providers/implementations/digests/libdefault-lib-null_prov.o",
                "providers/implementations/digests/libdefault-lib-ripemd_prov.o",
                "providers/implementations/digests/libdefault-lib-sha2_prov.o",
                "providers/implementations/digests/libdefault-lib-sha3_prov.o",
                "providers/implementations/digests/libdefault-lib-sm3_prov.o",
                "providers/implementations/digests/libfips-lib-sha2_prov.o",
        "doc/html/man3/OPENSSL_fork_prepare.html" => [
            "doc/man3/OPENSSL_fork_prepare.pod"
        ],
        "doc/html/man3/OPENSSL_gmtime.html" => [
            "doc/man3/OPENSSL_gmtime.pod"
        ],
        "doc/html/man3/OPENSSL_hexchar2int.html" => [
            "doc/man3/OPENSSL_hexchar2int.pod"
        ],
        "doc/html/man3/OPENSSL_ia32cap.html" => [
        "doc/man/man3/OPENSSL_fork_prepare.3" => [
            "doc/man3/OPENSSL_fork_prepare.pod"
        ],
        "doc/man/man3/OPENSSL_gmtime.3" => [
            "doc/man3/OPENSSL_gmtime.pod"
        ],
        "doc/man/man3/OPENSSL_hexchar2int.3" => [
            "doc/man3/OPENSSL_hexchar2int.pod"
        ],
        "doc/man/man3/OPENSSL_ia32cap.3" => [
            "doc/html/man3/OPENSSL_LH_stats.html",
            "doc/html/man3/OPENSSL_config.html",
            "doc/html/man3/OPENSSL_fork_prepare.html",
            "doc/html/man3/OPENSSL_gmtime.html",
            "doc/html/man3/OPENSSL_hexchar2int.html",
            "doc/html/man3/OPENSSL_ia32cap.html",
            "doc/html/man3/OPENSSL_init_crypto.html",
            "doc/html/man3/OPENSSL_init_ssl.html",
            "include",
            "apps/include"
        ],
        "test/ext_internal_test" => [
            ".",
            "include",
            "apps/include"
        ],
        "test/fatalerrtest" => [
            "include",
            "apps/include"
        ],
            "include",
            "apps/include"
        ],
        "test/fips_version_test" => [
            "include",
            "apps/include"
        ],
        "test/gmdifftest" => [
            "include",
            "apps/include"
        ],
            "apps/include",
            "."
        ],
        "test/punycode_test" => [
            "include",
            "apps/include"
        ],
        "test/rand_status_test" => [
            "include",
            "apps/include"
        ],
            "doc/man/man3/OPENSSL_LH_stats.3",
            "doc/man/man3/OPENSSL_config.3",
            "doc/man/man3/OPENSSL_fork_prepare.3",
            "doc/man/man3/OPENSSL_gmtime.3",
            "doc/man/man3/OPENSSL_hexchar2int.3",
            "doc/man/man3/OPENSSL_ia32cap.3",
            "doc/man/man3/OPENSSL_init_crypto.3",
            "doc/man/man3/OPENSSL_init_ssl.3",
        "test/evp_test",
        "test/exdatatest",
        "test/exptest",
        "test/ext_internal_test",
        "test/fatalerrtest",
        "test/ffc_internal_test",
        "test/fips_version_test",
        "test/gmdifftest",
        "test/hexstr_test",
        "test/hmactest",
        "test/http_test",
        "test/provider_pkey_test",
        "test/provider_status_test",
        "test/provider_test",
        "test/punycode_test",
        "test/rand_status_test",
        "test/rand_test",
        "test/rc2test",
        "test/rc4test",
        "providers/implementations/digests/libdefault-lib-null_prov.o" => [
            "providers/implementations/digests/null_prov.c"
        ],
        "providers/implementations/digests/libdefault-lib-ripemd_prov.o" => [
            "providers/implementations/digests/ripemd_prov.c"
        ],
        "providers/implementations/digests/libdefault-lib-sha2_prov.o" => [
            "providers/implementations/digests/sha2_prov.c"
        ],
        "providers/implementations/digests/libdefault-lib-sha3_prov.o" => [
            "providers/implementations/digests/libdefault-lib-md5_prov.o",
            "providers/implementations/digests/libdefault-lib-md5_sha1_prov.o",
            "providers/implementations/digests/libdefault-lib-null_prov.o",
            "providers/implementations/digests/libdefault-lib-ripemd_prov.o",
            "providers/implementations/digests/libdefault-lib-sha2_prov.o",
            "providers/implementations/digests/libdefault-lib-sha3_prov.o",
            "providers/implementations/digests/libdefault-lib-sm3_prov.o",
            "providers/implementations/encode_decode/libdefault-lib-decode_der2key.o",
        "test/exptest-bin-exptest.o" => [
            "test/exptest.c"
        ],
        "test/ext_internal_test" => [
            "test/ext_internal_test-bin-ext_internal_test.o"
        ],
        "test/ext_internal_test-bin-ext_internal_test.o" => [
            "test/ext_internal_test.c"
        ],
        "test/fatalerrtest" => [
            "test/fatalerrtest-bin-fatalerrtest.o",
            "test/helpers/fatalerrtest-bin-ssltestlib.o"
        ],
        "test/ffc_internal_test-bin-ffc_internal_test.o" => [
            "test/ffc_internal_test.c"
        ],
        "test/fips_version_test" => [
            "test/fips_version_test-bin-fips_version_test.o"
        ],
        "test/fips_version_test-bin-fips_version_test.o" => [
            "test/fips_version_test.c"
        ],
        "test/gmdifftest" => [
            "test/gmdifftest-bin-gmdifftest.o"
        ],
        "test/gmdifftest-bin-gmdifftest.o" => [
        "test/provider_test-bin-provider_test.o" => [
            "test/provider_test.c"
        ],
        "test/punycode_test" => [
            "test/punycode_test-bin-punycode_test.o"
        ],
        "test/punycode_test-bin-punycode_test.o" => [
            "test/punycode_test.c"
        ],
        "test/rand_status_test" => [
            "test/rand_status_test-bin-rand_status_test.o"
        ],
        "test/rand_status_test-bin-rand_status_test.o" => [