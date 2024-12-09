    this.authorized = false;
    this.authorizationError = verifyError.code || verifyError.message;

    // rejectUnauthorized property can be explicitly defined as `undefined`
    // causing the assignment to default value (`true`) fail. Before assigning
    // it to the tlssock connection options, explicitly check if it is false
    // and update rejectUnauthorized property. The property gets used by
    // TLSSocket connection handler to allow or reject connection if
    // unauthorized.
    // This check is potentially redundant, however it is better to keep it
    // in case the option object gets modified somewhere.
    if (options.rejectUnauthorized !== false) {
      this.destroy(verifyError);
      return;
    }
    debug('client emit secureConnect. rejectUnauthorized: %s, ' +
    signal: options.signal,
  });

  // rejectUnauthorized property can be explicitly defined as `undefined`
  // causing the assignment to default value (`true`) fail. Before assigning
  // it to the tlssock connection options, explicitly check if it is false
  // and update rejectUnauthorized property. The property gets used by TLSSocket
  // connection handler to allow or reject connection if unauthorized
  options.rejectUnauthorized = options.rejectUnauthorized !== false;

  tlssock[kConnectOptions] = options;

  if (cb)
    tlssock.once('secureConnect', cb);