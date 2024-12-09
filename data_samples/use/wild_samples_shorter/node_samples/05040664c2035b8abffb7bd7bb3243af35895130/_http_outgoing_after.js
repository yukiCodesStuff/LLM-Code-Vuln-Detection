    encoding = null;
  }

  var connection = this.connection;
  if (connection &&
      connection._httpMessage === this &&
      connection.writable &&
      this.output = [];
      this.outputEncodings = [];
      this.outputCallbacks = [];
    } else if (data.length === 0) {
      if (typeof callback === 'function')
        process.nextTick(callback);
      return true;
    }

    // Directly write to socket.
    return connection.write(data, encoding, callback);