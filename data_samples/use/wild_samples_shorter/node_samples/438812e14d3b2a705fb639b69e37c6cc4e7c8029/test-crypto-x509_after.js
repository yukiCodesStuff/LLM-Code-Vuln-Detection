  X509Certificate,
  createPrivateKey,
  generateKeyPairSync,
  createSign,
} = require('crypto');

const {
  isX509Certificate
  {
    // https://github.com/nodejs/node/issues/45377
    // https://github.com/nodejs/node/issues/45485
    // Confirm failures of
    // X509Certificate:verify()
    // X509Certificate:CheckPrivateKey()
    // X509Certificate:CheckCA()
    // X509Certificate:CheckIssued()
    // X509Certificate:ToLegacy()
    // do not affect other functions that use OpenSSL.
    // Subsequent calls to e.g. createPrivateKey should not throw.
    const keyPair = generateKeyPairSync('ed25519');
    assert(!x509.verify(keyPair.publicKey));
    createPrivateKey(key);
    assert(!x509.checkPrivateKey(keyPair.privateKey));
    createPrivateKey(key);
    const certPem = `
-----BEGIN CERTIFICATE-----
MIID6zCCAtOgAwIBAgIUTUREAaNcNL0zPkxAlMX0GJtJ/FcwDQYJKoZIhvcNAQEN
BQAwgYkxCzAJBgNVBAYTAlVTMRMwEQYDVQQIDApDYWxpZm9ybmlhMREwDwYDVQQH
DAhDYXJsc2JhZDEPMA0GA1UECgwGVmlhc2F0MR0wGwYDVQQLDBRWaWFzYXQgU2Vj
dXJlIE1vYmlsZTEiMCAGA1UEAwwZSGFja2VyT25lIHJlcG9ydCAjMTgwODU5NjAi
GA8yMDIyMTIxNjAwMDAwMFoYDzIwMjMxMjE1MjM1OTU5WjCBiTELMAkGA1UEBhMC
VVMxEzARBgNVBAgMCkNhbGlmb3JuaWExETAPBgNVBAcMCENhcmxzYmFkMQ8wDQYD
VQQKDAZWaWFzYXQxHTAbBgNVBAsMFFZpYXNhdCBTZWN1cmUgTW9iaWxlMSIwIAYD
VQQDDBlIYWNrZXJPbmUgcmVwb3J0ICMxODA4NTk2MIIBIjANBgkqhkiG9w0BAQEF
AAOCAQ8AMIIBCgKCAQEA6I7RBPm4E/9rIrCHV5lfsHI/yYzXtACJmoyP8OMkjbeB
h21oSJJF9FEnbivk6bYaHZIPasa+lSAydRM2rbbmfhF+jQoWYCIbV2ztrbFR70S1
wAuJrlYYm+8u+1HUru5UBZWUr/p1gFtv3QjpA8+43iwE4pXytTBKPXFo1f5iZwGI
D5Bz6DohT7Tyb8cpQ1uMCMCT0EJJ4n8wUrvfBgwBO94O4qlhs9vYgnDKepJDjptc
uSuEpvHALO8+EYkQ7nkM4Xzl/WK1yFtxxE93Jvd1OvViDGVrRVfsq+xYTKknGLX0
QIeoDDnIr0OjlYPd/cqyEgMcFyFxwDSzSc1esxdCpQIDAQABo0UwQzAdBgNVHQ4E
FgQUurygsEKdtQk0T+sjM0gEURdveRUwEgYDVR0TAQH/BAgwBgEB/wIB/zAOBgNV
HQ8BAf8EBAMCB4AwDQYJKoZIhvcNAQENBQADggEBAH7mIIXiQsQ4/QGNNFOQzTgP
/bUbMSZJsY5TPAvS9rF9yQVzs4dJZnQk5kEb/qrDQSe27oP0L0hfFm1wTGy+aKfa
BVGHdRmmvHtDUPLA9URCFShqKuS+GXp+6zt7dyZPRrPmiZaciiCMPHOnx59xSdPm
AZG8cD3fmK2ThC4FAMyvRb0qeobka3s22xTQ2kjwJO5gykTkZ+BR6SzRHQTjYMuT
iry9Bu8Kvbzu3r5n+/bmNz+xRNmEeehgT2qsHjA5b2YBVTr9MdN9Ro3H3saA3upr
oans248kpal88CGqsN2so/wZKxVnpiXlPHMdiNL7hRSUqlHkUi07FrP2Htg8kjI=
-----END CERTIFICATE-----`.trim();
    const c = new X509Certificate(certPem);
    assert(!c.ca);
    const signer = createSign('SHA256');
    assert(signer.sign(key, 'hex'));

    const c1 = new X509Certificate(certPem);
    assert(!c1.checkIssued(c1));
    const signer1 = createSign('SHA256');
    assert(signer1.sign(key, 'hex'));

    const c2 = new X509Certificate(certPem);
    assert(c2.toLegacyObject());
    const signer2 = createSign('SHA256');
    assert(signer2.sign(key, 'hex'));
  }

  // X509Certificate can be cloned via MessageChannel/MessagePort
  const mc = new MessageChannel();