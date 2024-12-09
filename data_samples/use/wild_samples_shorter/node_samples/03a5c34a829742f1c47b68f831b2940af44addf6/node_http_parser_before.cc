const uint32_t kOnTimeout = 6;
// Any more fields than this will be flushed into JS
const size_t kMaxHeaderFieldsCount = 32;

const uint32_t kLenientNone = 0;
const uint32_t kLenientHeaders = 1 << 0;
const uint32_t kLenientChunkedLength = 1 << 1;

    num_fields_ = num_values_ = 0;
    headers_completed_ = false;
    last_message_start_ = uv_hrtime();
    url_.Reset();
    status_message_.Reset();

    return 0;
  }

  // Reset nread for the next chunk
  int on_chunk_header() {
    header_nread_ = 0;
    return 0;
  }


  bool headers_completed_ = false;
  bool pending_pause_ = false;
  uint64_t header_nread_ = 0;
  uint64_t max_http_header_size_;
  uint64_t last_message_start_;
  ConnectionsList* connectionsList_;

    Proxy<DataCall, &Parser::on_header_value>::Raw,

    // on_chunk_extension_name
    nullptr,
    // on_chunk_extension_value
    nullptr,

    Proxy<Call, &Parser::on_headers_complete>::Raw,
    Proxy<DataCall, &Parser::on_body>::Raw,
    Proxy<Call, &Parser::on_message_complete>::Raw,
