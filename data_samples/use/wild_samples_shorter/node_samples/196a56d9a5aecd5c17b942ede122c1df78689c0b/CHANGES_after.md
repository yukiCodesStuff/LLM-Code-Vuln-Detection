
[Migration guide]: https://github.com/openssl/openssl/tree/master/doc/man7/migration_guide.pod

### Changes between 3.0.7 and 3.0.7+quic [1 Nov 2022]

 * Add QUIC API support from BoringSSL.

   *Todd Short*

### Changes between 3.0.6 and 3.0.7 [1 Nov 2022]

 * Fixed two buffer overflows in punycode decoding functions.

   A buffer overrun can be triggered in X.509 certificate verification,
   specifically in name constraint checking. Note that this occurs after
   certificate chain signature verification and requires either a CA to
   have signed the malicious certificate or for the application to continue
   certificate verification despite failure to construct a path to a trusted
   issuer.

   In a TLS client, this can be triggered by connecting to a malicious
   server.  In a TLS server, this can be triggered if the server requests
   client authentication and a malicious client connects.

   An attacker can craft a malicious email address to overflow
   an arbitrary number of bytes containing the `.`  character (decimal 46)
   on the stack.  This buffer overflow could result in a crash (causing a
   denial of service).
   ([CVE-2022-3786])

   An attacker can craft a malicious email address to overflow four
   attacker-controlled bytes on the stack.  This buffer overflow could
   result in a crash (causing a denial of service) or potentially remote code
   execution depending on stack layout for any given platform/compiler.
   ([CVE-2022-3602])

   *Paul Dale*

 * Removed all references to invalid OSSL_PKEY_PARAM_RSA names for CRT
   parameters in OpenSSL code.
   Applications should not use the names OSSL_PKEY_PARAM_RSA_FACTOR,
   OSSL_PKEY_PARAM_RSA_EXPONENT and OSSL_PKEY_PARAM_RSA_COEFFICIENT.
   Use the numbered names such as OSSL_PKEY_PARAM_RSA_FACTOR1 instead.
   Using these invalid names may cause algorithms to use slower methods
   that ignore the CRT parameters.

   *Shane Lontis*

 * Fixed a regression introduced in 3.0.6 version raising errors on some stack
   operations.

   *Tomáš Mráz*

 * Fixed a regression introduced in 3.0.6 version not refreshing the certificate
   data to be signed before signing the certificate.

   *Gibeom Gwon*

 * Added RIPEMD160 to the default provider.

   *Paul Dale*

 * Ensured that the key share group sent or accepted for the key exchange
   is allowed for the protocol version.

   *Matt Caswell*

### Changes between 3.0.5 and 3.0.6 [11 Oct 2022]

 * OpenSSL supports creating a custom cipher via the legacy
   EVP_CIPHER_meth_new() function and associated function calls. This function
   was deprecated in OpenSSL 3.0 and application authors are instead encouraged
   to use the new provider mechanism in order to implement custom ciphers.

   OpenSSL versions 3.0.0 to 3.0.5 incorrectly handle legacy custom ciphers
   passed to the EVP_EncryptInit_ex2(), EVP_DecryptInit_ex2() and
   EVP_CipherInit_ex2() functions (as well as other similarly named encryption
   and decryption initialisation functions). Instead of using the custom cipher
   directly it incorrectly tries to fetch an equivalent cipher from the
   available providers. An equivalent cipher is found based on the NID passed to
   EVP_CIPHER_meth_new(). This NID is supposed to represent the unique NID for a
   given cipher. However it is possible for an application to incorrectly pass
   NID_undef as this value in the call to EVP_CIPHER_meth_new(). When NID_undef
   is used in this way the OpenSSL encryption/decryption initialisation function
   will match the NULL cipher as being equivalent and will fetch this from the
   available providers. This will succeed if the default provider has been
   loaded (or if a third party provider has been loaded that offers this
   cipher). Using the NULL cipher means that the plaintext is emitted as the
   ciphertext.

   Applications are only affected by this issue if they call
   EVP_CIPHER_meth_new() using NID_undef and subsequently use it in a call to an
   encryption/decryption initialisation function. Applications that only use
   SSL/TLS are not impacted by this issue.
   ([CVE-2022-3358])

   *Matt Caswell*

 * Fix LLVM vs Apple LLVM version numbering confusion that caused build failures
   on MacOS 10.11

   *Richard Levitte*

 * Fixed the linux-mips64 Configure target which was missing the
   SIXTY_FOUR_BIT bn_ops flag. This was causing heap corruption on that
   platform.

   *Adam Joseph*

 * Fix handling of a ticket key callback that returns 0 in TLSv1.3 to not send a
   ticket

   *Matt Caswell*

 * Correctly handle a retransmitted ClientHello in DTLS

   *Matt Caswell*

 * Fixed detection of ktls support in cross-compile environment on Linux

   *Tomas Mraz*

 * Fixed some regressions and test failures when running the 3.0.0 FIPS provider
   against 3.0.x

   *Paul Dale*

 * Fixed SSL_pending() and SSL_has_pending() with DTLS which were failing to
   report correct results in some cases

   *Matt Caswell*

 * Fix UWP builds by defining VirtualLock

   *Charles Milette*

 * For known safe primes use the minimum key length according to RFC 7919.
   Longer private key sizes unnecessarily raise the cycles needed to compute the
   shared secret without any increase of the real security. This fixes a
   regression from 1.1.1 where these shorter keys were generated for the known
   safe primes.

   *Tomas Mraz*

 * Added the loongarch64 target

   *Shi Pujin*

 * Fixed EC ASM flag passing. Flags for ASM implementations of EC curves were
   only passed to the FIPS provider and not to the default or legacy provider.

   *Juergen Christ*

 * Fixed reported performance degradation on aarch64. Restored the
   implementation prior to commit 2621751 ("aes/asm/aesv8-armx.pl: avoid
   32-bit lane assignment in CTR mode") for 64bit targets only, since it is
   reportedly 2-17% slower and the silicon errata only affects 32bit targets.
   The new algorithm is still used for 32 bit targets.

   *Bernd Edlinger*

 * Added a missing header for memcmp that caused compilation failure on some
   platforms

   *Gregor Jasny*

### Changes between 3.0.4 and 3.0.5 [5 Jul 2022]

 * The OpenSSL 3.0.4 release introduced a serious bug in the RSA
   implementation for X86_64 CPUs supporting the AVX512IFMA instructions.