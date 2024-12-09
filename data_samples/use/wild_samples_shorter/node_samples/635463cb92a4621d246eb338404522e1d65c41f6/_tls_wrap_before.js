    this.authorized = false;
    this.authorizationError = verifyError.code || verifyError.message;

    if (options.rejectUnauthorized) {
      this.destroy(verifyError);
      return;
    }
    debug('client emit secureConnect. rejectUnauthorized: %s, ' +
    signal: options.signal,
  });

  tlssock[kConnectOptions] = options;

  if (cb)
    tlssock.once('secureConnect', cb);