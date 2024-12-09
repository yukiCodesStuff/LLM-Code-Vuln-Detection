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
      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    }

    // Directly write to socket.
    return connection.write(data, encoding, callback);