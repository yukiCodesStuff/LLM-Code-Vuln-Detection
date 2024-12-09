  X509Certificate,
  createPrivateKey,
  generateKeyPairSync,
} = require('crypto');

const {
  isX509Certificate
  {
    // https://github.com/nodejs/node/issues/45377
    // https://github.com/nodejs/node/issues/45485
    // Confirm failures of X509Certificate:verify() and X509Certificate:CheckPrivateKey()
    // do not affect other functions that use OpenSSL.
    // Subsequent calls to e.g. createPrivateKey should not throw.
    const keyPair = generateKeyPairSync('ed25519');
    assert(!x509.verify(keyPair.publicKey));
    createPrivateKey(key);
    assert(!x509.checkPrivateKey(keyPair.privateKey));
    createPrivateKey(key);
  }

  // X509Certificate can be cloned via MessageChannel/MessagePort
  const mc = new MessageChannel();