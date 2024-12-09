  } else {
    parser->lenient_flags &= ~LENIENT_KEEP_ALIVE;
  }
}

void llhttp_set_lenient_transfer_encoding(llhttp_t* parser, int enabled) {
  if (enabled) {
    parser->lenient_flags |= LENIENT_TRANSFER_ENCODING;
  } else {
    parser->lenient_flags &= ~LENIENT_TRANSFER_ENCODING;
  }
}