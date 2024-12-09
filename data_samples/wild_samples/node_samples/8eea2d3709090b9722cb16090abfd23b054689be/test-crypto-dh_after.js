    assert.throws(() => {
      c.final('utf8');
    }, wrongBlockLength);
  }

  {
    const v = crypto.constants.OPENSSL_VERSION_NUMBER;
    const hasOpenSSL3WithNewErrorMessage = (v >= 0x300000c0 && v <= 0x30100000) || (v >= 0x30100040 && v <= 0x30200000);
    assert.throws(() => {
      dh3.computeSecret('');
    }, { message: common.hasOpenSSL3 && !hasOpenSSL3WithNewErrorMessage ?
      'error:02800080:Diffie-Hellman routines::invalid secret' :
      'Supplied key is too small' });
  }