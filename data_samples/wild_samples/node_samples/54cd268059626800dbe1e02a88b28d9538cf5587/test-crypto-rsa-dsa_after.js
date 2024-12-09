  const encryptedBuffer = crypto.publicEncrypt({
    key: rsaPubPem,
    padding: padding,
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
test_rsa('RSA_PKCS1_OAEP_PADDING');

// Test OAEP with different hash functions.
test_rsa('RSA_PKCS1_OAEP_PADDING', undefined, 'sha1');
test_rsa('RSA_PKCS1_OAEP_PADDING', 'sha1', undefined);
test_rsa('RSA_PKCS1_OAEP_PADDING', 'sha256', 'sha256');
test_rsa('RSA_PKCS1_OAEP_PADDING', 'sha512', 'sha512');
assert.throws(() => {
  test_rsa('RSA_PKCS1_OAEP_PADDING', 'sha256', 'sha512');
}, {
  code: 'ERR_OSSL_RSA_OAEP_DECODING_ERROR'
});

// The following RSA-OAEP test cases were created using the WebCrypto API to
// ensure compatibility when using non-SHA1 hash functions.
{
  const { decryptionTests } =
      JSON.parse(fixtures.readSync('rsa-oaep-test-vectors.js', 'utf8'));

  for (const { ct, oaepHash, oaepLabel } of decryptionTests) {
    const label = oaepLabel ? Buffer.from(oaepLabel, 'hex') : undefined;
    const copiedLabel = oaepLabel ? getBufferCopy(label) : undefined;

    const decrypted = crypto.privateDecrypt({
      key: rsaPkcs8KeyPem,
      oaepHash,
      oaepLabel: oaepLabel ? label : undefined
    }, Buffer.from(ct, 'hex'));

    assert.strictEqual(decrypted.toString('utf8'), 'Hello Node.js');

    const otherDecrypted = crypto.privateDecrypt({
      key: rsaPkcs8KeyPem,
      oaepHash,
      oaepLabel: copiedLabel
    }, Buffer.from(ct, 'hex'));

    assert.strictEqual(otherDecrypted.toString('utf8'), 'Hello Node.js');
  }
}

// Test invalid oaepHash and oaepLabel options.
for (const fn of [crypto.publicEncrypt, crypto.privateDecrypt]) {
  assert.throws(() => {
    fn({
      key: rsaPubPem,
      oaepHash: 'Hello world'
    }, Buffer.alloc(10));
  }, {
    code: 'ERR_OSSL_EVP_INVALID_DIGEST'
  });

  for (const oaepHash of [0, false, null, Symbol(), () => {}]) {
    assert.throws(() => {
      fn({
        key: rsaPubPem,
        oaepHash
      }, Buffer.alloc(10));
    }, {
      code: 'ERR_INVALID_ARG_TYPE'
    });
  }

  for (const oaepLabel of [0, false, null, Symbol(), () => {}, {}]) {
    assert.throws(() => {
      fn({
        key: rsaPubPem,
        oaepLabel
      }, Buffer.alloc(10));
    }, {
      code: 'ERR_INVALID_ARG_TYPE'
    });
  }
}

// Test RSA key signing/verification
let rsaSign = crypto.createSign('SHA1');
let rsaVerify = crypto.createVerify('SHA1');
assert.ok(rsaSign);
assert.ok(rsaVerify);

const expectedSignature = fixtures.readKey(
  'rsa_public_sha1_signature_signedby_rsa_private_pkcs8.sha1',
  'hex'
);

rsaSign.update(rsaPubPem);
let rsaSignature = rsaSign.sign(rsaKeyPem, 'hex');
assert.strictEqual(rsaSignature, expectedSignature);

rsaVerify.update(rsaPubPem);
assert.strictEqual(rsaVerify.verify(rsaPubPem, rsaSignature, 'hex'), true);

// Test RSA PKCS#8 key signing/verification
rsaSign = crypto.createSign('SHA1');
rsaSign.update(rsaPubPem);
rsaSignature = rsaSign.sign(rsaPkcs8KeyPem, 'hex');
assert.strictEqual(rsaSignature, expectedSignature);

rsaVerify = crypto.createVerify('SHA1');
rsaVerify.update(rsaPubPem);
assert.strictEqual(rsaVerify.verify(rsaPubPem, rsaSignature, 'hex'), true);

// Test RSA key signing/verification with encrypted key
rsaSign = crypto.createSign('SHA1');
rsaSign.update(rsaPubPem);
const signOptions = { key: rsaKeyPemEncrypted, passphrase: 'password' };
rsaSignature = rsaSign.sign(signOptions, 'hex');
assert.strictEqual(rsaSignature, expectedSignature);

rsaVerify = crypto.createVerify('SHA1');
rsaVerify.update(rsaPubPem);
assert.strictEqual(rsaVerify.verify(rsaPubPem, rsaSignature, 'hex'), true);

rsaSign = crypto.createSign('SHA1');
rsaSign.update(rsaPubPem);
assert.throws(() => {
  const signOptions = { key: rsaKeyPemEncrypted, passphrase: 'wrong' };
  rsaSign.sign(signOptions, 'hex');
}, decryptPrivateKeyError);

//
// Test RSA signing and verification
//
{
  const privateKey = fixtures.readKey('rsa_private_b.pem');
  const publicKey = fixtures.readKey('rsa_public_b.pem');

  const input = 'I AM THE WALRUS';

  const signature = fixtures.readKey(
    'I_AM_THE_WALRUS_sha256_signature_signedby_rsa_private_b.sha256',
    'hex'
  );

  const sign = crypto.createSign('SHA256');
  sign.update(input);

  const output = sign.sign(privateKey, 'hex');
  assert.strictEqual(output, signature);

  const verify = crypto.createVerify('SHA256');
  verify.update(input);

  assert.strictEqual(verify.verify(publicKey, signature, 'hex'), true);

  // Test the legacy signature algorithm name.
  const sign2 = crypto.createSign('RSA-SHA256');
  sign2.update(input);

  const output2 = sign2.sign(privateKey, 'hex');
  assert.strictEqual(output2, signature);

  const verify2 = crypto.createVerify('SHA256');
  verify2.update(input);

  assert.strictEqual(verify2.verify(publicKey, signature, 'hex'), true);
}


//
// Test DSA signing and verification
//
{
  const input = 'I AM THE WALRUS';

  // DSA signatures vary across runs so there is no static string to verify
  // against.
  const sign = crypto.createSign('SHA1');
  sign.update(input);
  const signature = sign.sign(dsaKeyPem, 'hex');

  const verify = crypto.createVerify('SHA1');
  verify.update(input);

  assert.strictEqual(verify.verify(dsaPubPem, signature, 'hex'), true);

  // Test the legacy 'DSS1' name.
  const sign2 = crypto.createSign('DSS1');
  sign2.update(input);
  const signature2 = sign2.sign(dsaKeyPem, 'hex');

  const verify2 = crypto.createVerify('DSS1');
  verify2.update(input);

  assert.strictEqual(verify2.verify(dsaPubPem, signature2, 'hex'), true);
}


//
// Test DSA signing and verification with PKCS#8 private key
//
{
  const input = 'I AM THE WALRUS';

  // DSA signatures vary across runs so there is no static string to verify
  // against.
  const sign = crypto.createSign('SHA1');
  sign.update(input);
  const signature = sign.sign(dsaPkcs8KeyPem, 'hex');

  const verify = crypto.createVerify('SHA1');
  verify.update(input);

  assert.strictEqual(verify.verify(dsaPubPem, signature, 'hex'), true);
}


//
// Test DSA signing and verification with encrypted key
//
const input = 'I AM THE WALRUS';

{
  const sign = crypto.createSign('SHA1');
  sign.update(input);
  assert.throws(() => {
    sign.sign({ key: dsaKeyPemEncrypted, passphrase: 'wrong' }, 'hex');
  }, decryptPrivateKeyError);
}

{
  // DSA signatures vary across runs so there is no static string to verify
  // against.
  const sign = crypto.createSign('SHA1');
  sign.update(input);
  const signOptions = { key: dsaKeyPemEncrypted, passphrase: 'password' };
  const signature = sign.sign(signOptions, 'hex');

  const verify = crypto.createVerify('SHA1');
  verify.update(input);

  assert.strictEqual(verify.verify(dsaPubPem, signature, 'hex'), true);
}