const uint32_t kOnTimeout = 6;
// Any more fields than this will be flushed into JS
const size_t kMaxHeaderFieldsCount = 32;
// Maximum size of chunk extensions
const size_t kMaxChunkExtensionsSize = 16384;

const uint32_t kLenientNone = 0;
const uint32_t kLenientHeaders = 1 << 0;
const uint32_t kLenientChunkedLength = 1 << 1;

    num_fields_ = num_values_ = 0;
    headers_completed_ = false;
    chunk_extensions_nread_ = 0;
    last_message_start_ = uv_hrtime();
    url_.Reset();
    status_message_.Reset();

    return 0;
  }

  int on_chunk_extension(const char* at, size_t length) {
    chunk_extensions_nread_ += length;

    if (chunk_extensions_nread_ > kMaxChunkExtensionsSize) {
      llhttp_set_error_reason(&parser_,
          "HPE_CHUNK_EXTENSIONS_OVERFLOW:Chunk extensions overflow");
      return HPE_USER;
    }

    return 0;
  }

  // Reset nread for the next chunk and also reset the extensions counter
  int on_chunk_header() {
    header_nread_ = 0;
    chunk_extensions_nread_ = 0;
    return 0;
  }


  bool headers_completed_ = false;
  bool pending_pause_ = false;
  uint64_t header_nread_ = 0;
  uint64_t chunk_extensions_nread_ = 0;
  uint64_t max_http_header_size_;
  uint64_t last_message_start_;
  ConnectionsList* connectionsList_;

    Proxy<DataCall, &Parser::on_header_value>::Raw,

    // on_chunk_extension_name
    Proxy<DataCall, &Parser::on_chunk_extension>::Raw,
    // on_chunk_extension_value
    Proxy<DataCall, &Parser::on_chunk_extension>::Raw,
    Proxy<Call, &Parser::on_headers_complete>::Raw,
    Proxy<DataCall, &Parser::on_body>::Raw,
    Proxy<Call, &Parser::on_message_complete>::Raw,
