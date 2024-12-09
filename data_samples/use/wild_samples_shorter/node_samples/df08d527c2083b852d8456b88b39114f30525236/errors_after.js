E('ERR_HTTP_INVALID_HEADER_VALUE',
  'Invalid value "%s" for header "%s"', TypeError);
E('ERR_HTTP_INVALID_STATUS_CODE', 'Invalid status code: %s', RangeError);
E('ERR_HTTP_REQUEST_TIMEOUT', 'Request timeout', Error);
E('ERR_HTTP_SOCKET_ENCODING',
  'Changing the socket encoding is not allowed per RFC7230 Section 3.', Error);
E('ERR_HTTP_TRAILER_INVALID',
  'Trailers are invalid with this transfer encoding', Error);