    infoAccess: Object.assign({ __proto__: null }, {
      'OCSP - URI': ['http://ocsp.nodejs.org/'],
      'CA Issuers - URI': ['http://ca.nodejs.org/ca.cert']
    }),
    modulus: 'D456320AFB20D3827093DC2C4284ED04DFBABD56E1DDAE529E28B790CD42' +
              '56DB273349F3735FFD337C7A6363ECCA5A27B7F73DC7089A96C6D886DB0C' +
              '62388F1CDD6A963AFCD599D5800E587A11F908960F84ED50BA25A28303EC' +
              'DA6E684FBE7BAEDC9CE8801327B1697AF25097CEE3F175E400984C0DB6A8' +
              'EB87BE03B4CF94774BA56FFFC8C63C68D6ADEB60ABBE69A7B14AB6A6B9E7' +
              'BAA89B5ADAB8EB07897C07F6D4FA3D660DFF574107D28E8F63467A788624' +
              'C574197693E959CEA1362FFAE1BBA10C8C0D88840ABFEF103631B2E8F5C3' +
              '9B5548A7EA57E8A39F89291813F45A76C448033A2B7ED8403F4BAA147CF3' +
              '5E2D2554AA65CE49695797095BF4DC6B',
    bits: 2048,
    exponent: '0x10001',
    valid_from: 'Sep  3 21:40:37 2022 GMT',
    valid_to: 'Jun 17 21:40:37 2296 GMT',
    fingerprint: '8B:89:16:C4:99:87:D2:13:1A:64:94:36:38:A5:32:01:F0:95:3B:53',
    fingerprint256:
      '2C:62:59:16:91:89:AB:90:6A:3E:98:88:A6:D3:C5:58:58:6C:AE:FF:9C:33:' +
      '22:7C:B6:77:D3:34:E7:53:4B:05',
    fingerprint512:
      '51:62:18:39:E2:E2:77:F5:86:11:E8:C0:CA:54:43:7C:76:83:19:05:D0:03:' +
      '24:21:B8:EB:14:61:FB:24:16:EB:BD:51:1A:17:91:04:30:03:EB:68:5F:DC:' +
      '86:E1:D1:7C:FB:AF:78:ED:63:5F:29:9C:32:AF:A1:8E:22:96:D1:02',
    serialNumber: '147D36C1C2F74206DE9FAB5F2226D78ADB00A426'
  };

  const legacyObject = x509.toLegacyObject();

  assert.deepStrictEqual(legacyObject.raw, x509.raw);
  assert.deepStrictEqual(legacyObject.subject, legacyObjectCheck.subject);
  assert.deepStrictEqual(legacyObject.issuer, legacyObjectCheck.issuer);
  assert.deepStrictEqual(legacyObject.infoAccess, legacyObjectCheck.infoAccess);
  assert.strictEqual(legacyObject.modulus, legacyObjectCheck.modulus);
  assert.strictEqual(legacyObject.bits, legacyObjectCheck.bits);
  assert.strictEqual(legacyObject.exponent, legacyObjectCheck.exponent);
  assert.strictEqual(legacyObject.valid_from, legacyObjectCheck.valid_from);
  assert.strictEqual(legacyObject.valid_to, legacyObjectCheck.valid_to);
  assert.strictEqual(legacyObject.fingerprint, legacyObjectCheck.fingerprint);
  assert.strictEqual(
    legacyObject.fingerprint256,
    legacyObjectCheck.fingerprint256);
  assert.strictEqual(
    legacyObject.serialNumber,
    legacyObjectCheck.serialNumber);
}

{
  // This X.509 Certificate can be parsed by OpenSSL because it contains a
  // structurally sound TBSCertificate structure. However, the SPKI field of the
  // TBSCertificate contains the subjectPublicKey as a BIT STRING, and this bit
  // sequence is not a valid public key. Ensure that X509Certificate.publicKey
  // does not abort in this case.

  const certPem = `-----BEGIN CERTIFICATE-----
MIIDpDCCAw0CFEc1OZ8g17q+PZnna3iQ/gfoZ7f3MA0GCSqGSIb3DQEBBQUAMIHX
MRMwEQYLKwYBBAGCNzwCAQMTAkdJMR0wGwYDVQQPExRQcml2YXRlIE9yZ2FuaXph
dGlvbjEOMAwGA1UEBRMFOTkxOTExCzAJBgNVBAYTAkdJMRIwEAYDVQQIFAlHaWJy
YWx0YXIxEjAQBgNVBAcUCUdpYnJhbHRhcjEgMB4GA1UEChQXV0hHIChJbnRlcm5h
dGlvbmFsKSBMdGQxHDAaBgNVBAsUE0ludGVyYWN0aXZlIEJldHRpbmcxHDAaBgNV
BAMUE3d3dy53aWxsaWFtaGlsbC5jb20wIhgPMjAxNDAyMDcwMDAwMDBaGA8yMDE1
MDIyMTIzNTk1OVowgbAxCzAJBgNVBAYTAklUMQ0wCwYDVQQIEwRSb21lMRAwDgYD
VQQHEwdQb21lemlhMRYwFAYDVQQKEw1UZWxlY29taXRhbGlhMRIwEAYDVQQrEwlB
RE0uQVAuUE0xHTAbBgNVBAMTFHd3dy50ZWxlY29taXRhbGlhLml0MTUwMwYJKoZI
hvcNAQkBFiZ2YXNlc2VyY2l6aW9wb3J0YWxpY29AdGVsZWNvbWl0YWxpYS5pdDCB
nzANBgkqhkiG9w0BAQEFAAOBjQA4gYkCgYEA5m/Vf7PevH+inMfUJOc8GeR7WVhM
CQwcMM5k46MSZo7kCk7VZuaq5G2JHGAGnLPaPUkeXlrf5qLpTxXXxHNtz+WrDlFt
boAdnTcqpX3+72uBGOaT6Wi/9YRKuCs5D5/cAxAc3XjHfpRXMoXObj9Vy7mLndfV
/wsnTfU9QVeBkgsCAwEAAaOBkjCBjzAdBgNVHQ4EFgQUfLjAjEiC83A+NupGrx5+
Qe6nhRMwbgYIKwYBBQUHAQwEYjBgoV6gXDBaMFgwVhYJaW1hZ2UvZ2lmMCEwHzAH
BgUrDgMCGgQUS2u5KJYGDLvQUjibKaxLB4shBRgwJhYkaHR0cDovL2xvZ28udmVy
aXNpZ24uY29tL3ZzbG9nbzEuZ2lmMA0GCSqGSIb3DQEBBQUAA4GBALLiAMX0cIMp
+V/JgMRhMEUKbrt5lYKfv9dil/f22ezZaFafb070jGMMPVy9O3/PavDOkHtTv3vd
tAt3hIKFD1bJt6c6WtMH2Su3syosWxmdmGk5ihslB00lvLpfj/wed8i3bkcB1doq
UcXd/5qu2GhokrKU2cPttU+XAN2Om6a0
-----END CERTIFICATE-----`;

  const cert = new X509Certificate(certPem);
  assert.throws(() => cert.publicKey, {
    message: common.hasOpenSSL3 ? /decode error/ : /wrong tag/,
    name: 'Error'
  });

  assert.strictEqual(cert.checkIssued(cert), false);
}