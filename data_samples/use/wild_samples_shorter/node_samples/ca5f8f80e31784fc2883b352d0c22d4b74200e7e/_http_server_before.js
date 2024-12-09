  if (statusCode < 100 || statusCode > 999)
    throw new RangeError(`Invalid status code: ${statusCode}`);

  var statusLine = 'HTTP/1.1 ' + statusCode.toString() + ' ' +
                   this.statusMessage + CRLF;

  if (statusCode === 204 || statusCode === 304 ||