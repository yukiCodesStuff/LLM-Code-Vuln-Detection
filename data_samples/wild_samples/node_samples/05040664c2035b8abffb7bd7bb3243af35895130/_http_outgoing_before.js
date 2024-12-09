  if (typeof encoding === 'function') {
    callback = encoding;
    encoding = null;
  }

  if (data.length === 0) {
    if (typeof callback === 'function')
      process.nextTick(callback);
    return true;
  }

  var connection = this.connection;
  if (connection &&
      connection._httpMessage === this &&
      connection.writable &&
      !connection.destroyed) {
    // There might be pending data in the this.output buffer.
    var outputLength = this.output.length;
    if (outputLength > 0) {
      var output = this.output;
      var outputEncodings = this.outputEncodings;
      var outputCallbacks = this.outputCallbacks;
      for (var i = 0; i < outputLength; i++) {
        connection.write(output[i], outputEncodings[i],
                         outputCallbacks[i]);
      }

      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    }
      for (var i = 0; i < outputLength; i++) {
        connection.write(output[i], outputEncodings[i],
                         outputCallbacks[i]);
      }

      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    }

    // Directly write to socket.
    return connection.write(data, encoding, callback);
  } else if (connection && connection.destroyed) {
    // The socket was destroyed.  If we're still trying to write to it,
    // then we haven't gotten the 'close' event yet.
    return false;
  } else {
    // buffer, as long as we're not destroyed.
    return this._buffer(data, encoding, callback);
  }