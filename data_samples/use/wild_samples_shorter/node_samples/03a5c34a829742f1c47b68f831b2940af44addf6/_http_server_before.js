  'Connection: close\r\n\r\n', 'ascii',
);

function socketOnError(e) {
  // Ignore further errors
  this.removeListener('error', socketOnError);

        case 'HPE_HEADER_OVERFLOW':
          response = requestHeaderFieldsTooLargeResponse;
          break;
        case 'ERR_HTTP_REQUEST_TIMEOUT':
          response = requestTimeoutResponse;
          break;
        default: