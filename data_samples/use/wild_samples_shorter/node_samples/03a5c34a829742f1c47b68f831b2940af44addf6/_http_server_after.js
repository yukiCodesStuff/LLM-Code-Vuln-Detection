  'Connection: close\r\n\r\n', 'ascii',
);

const requestChunkExtensionsTooLargeResponse = Buffer.from(
  `HTTP/1.1 413 ${STATUS_CODES[413]}\r\n` +
  'Connection: close\r\n\r\n', 'ascii',
);

function socketOnError(e) {
  // Ignore further errors
  this.removeListener('error', socketOnError);

        case 'HPE_HEADER_OVERFLOW':
          response = requestHeaderFieldsTooLargeResponse;
          break;
        case 'HPE_CHUNK_EXTENSIONS_OVERFLOW':
          response = requestChunkExtensionsTooLargeResponse;
          break;
        case 'ERR_HTTP_REQUEST_TIMEOUT':
          response = requestTimeoutResponse;
          break;
        default: