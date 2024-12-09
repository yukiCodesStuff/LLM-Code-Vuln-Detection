void nghttp2_option_set_no_closed_streams(nghttp2_option *option, int val) {
  option->opt_set_mask |= NGHTTP2_OPT_NO_CLOSED_STREAMS;
  option->no_closed_streams = val;
}

void nghttp2_option_set_max_outbound_ack(nghttp2_option *option, size_t val) {
  option->opt_set_mask |= NGHTTP2_OPT_MAX_OUTBOUND_ACK;
  option->max_outbound_ack = val;
}