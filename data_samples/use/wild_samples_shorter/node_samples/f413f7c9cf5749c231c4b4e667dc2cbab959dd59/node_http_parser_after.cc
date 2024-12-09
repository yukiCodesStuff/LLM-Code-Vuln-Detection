
  static const parser_settings_t settings;
#ifdef NODE_EXPERIMENTAL_HTTP
  static const uint64_t kMaxHeaderSize = 8 * 1024;
#endif  /* NODE_EXPERIMENTAL_HTTP */
};

const parser_settings_t Parser::settings = {