    oaepHash: encryptOaepHash
  }, bufferToEncrypt);


  if (padding === constants.RSA_PKCS1_PADDING) {
    assert.throws(() => {
      crypto.privateDecrypt({
        key: rsaKeyPem,
        padding: padding,
        oaepHash: decryptOaepHash
      }, encryptedBuffer);
    }, { code: 'ERR_INVALID_ARG_VALUE' });
    assert.throws(() => {
      crypto.privateDecrypt({
        key: rsaPkcs8KeyPem,
        padding: padding,
        oaepHash: decryptOaepHash
      }, encryptedBuffer);
    }, { code: 'ERR_INVALID_ARG_VALUE' });
  } else {
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
}

test_rsa('RSA_NO_PADDING');
test_rsa('RSA_PKCS1_PADDING');