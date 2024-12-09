
[Migration guide]: https://github.com/openssl/openssl/tree/master/doc/man7/migration_guide.pod

### Changes between 3.0.5 and 3.0.5+quic [5 Jul 2022]

 * Add QUIC API support from BoringSSL.

   *Todd Short*

### Changes between 3.0.4 and 3.0.5 [5 Jul 2022]

 * The OpenSSL 3.0.4 release introduced a serious bug in the RSA
   implementation for X86_64 CPUs supporting the AVX512IFMA instructions.