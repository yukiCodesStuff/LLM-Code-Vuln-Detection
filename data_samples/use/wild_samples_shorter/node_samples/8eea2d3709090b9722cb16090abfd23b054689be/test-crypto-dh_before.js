    }, wrongBlockLength);
  }

  assert.throws(() => {
    dh3.computeSecret('');
  }, { message: common.hasOpenSSL3 ?
    'error:02800080:Diffie-Hellman routines::invalid secret' :
    'Supplied key is too small' });
}

// Through a fluke of history, g=0 defaults to DH_GENERATOR (2).
{