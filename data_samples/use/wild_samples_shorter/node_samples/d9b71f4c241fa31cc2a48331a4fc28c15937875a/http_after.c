    return 2;
  } else if (parser->flags & F_TRANSFER_ENCODING) {
    if (parser->type == HTTP_REQUEST &&
        (parser->lenient_flags & LENIENT_CHUNKED_LENGTH) == 0 &&
        (parser->lenient_flags & LENIENT_TRANSFER_ENCODING) == 0) {
      /* RFC 7230 3.3.3 */

      /* If a Transfer-Encoding header field
       * is present in a request and the chunked transfer coding is not