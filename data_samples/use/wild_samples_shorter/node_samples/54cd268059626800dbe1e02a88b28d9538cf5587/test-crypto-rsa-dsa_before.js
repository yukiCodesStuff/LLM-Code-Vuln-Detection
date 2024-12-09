    oaepHash: encryptOaepHash
  }, bufferToEncrypt);

  let decryptedBuffer = crypto.privateDecrypt({
    key: rsaKeyPem,
    padding: padding,
    oaepHash: decryptOaepHash
  }, encryptedBuffer);
  assert.deepStrictEqual(decryptedBuffer, input);

  decryptedBuffer = crypto.privateDecrypt({
    key: rsaPkcs8KeyPem,
    padding: padding,
    oaepHash: decryptOaepHash
  }, encryptedBuffer);
  assert.deepStrictEqual(decryptedBuffer, input);
}

test_rsa('RSA_NO_PADDING');
test_rsa('RSA_PKCS1_PADDING');