  bool IsAllowedHost(const std::string& host_with_port) const {
    std::string host = TrimPort(host_with_port);
    return host.empty() || IsIPAddress(host)
           || node::StringEqualNoCase(host.data(), "localhost");
  }

  bool parsing_value_;
  llhttp_t parser_;
  llhttp_settings_t parser_settings;
  std::vector<HttpEvent> events_;
  std::string current_header_;
  std::map<std::string, std::string> headers_;
  std::string path_;
};

}  // namespace

// Any protocol
ProtocolHandler::ProtocolHandler(InspectorSocket* inspector,
                                 TcpHolder::Pointer tcp)
                                 : inspector_(inspector), tcp_(std::move(tcp)) {
  CHECK_NOT_NULL(tcp_);
  tcp_->SetHandler(this);
}

int ProtocolHandler::WriteRaw(const std::vector<char>& buffer,
                              uv_write_cb write_cb) {
  return tcp_->WriteRaw(buffer, write_cb);
}

InspectorSocket::Delegate* ProtocolHandler::delegate() {
  return tcp_->delegate();
}

std::string ProtocolHandler::GetHost() const {
  char ip[INET6_ADDRSTRLEN];
  sockaddr_storage addr;
  int len = sizeof(addr);
  int err = uv_tcp_getsockname(tcp_->tcp(),
                               reinterpret_cast<struct sockaddr*>(&addr),
                               &len);
  if (err != 0)
    return "";
  if (addr.ss_family == AF_INET6) {
    const sockaddr_in6* v6 = reinterpret_cast<const sockaddr_in6*>(&addr);
    err = uv_ip6_name(v6, ip, sizeof(ip));
  } else {
    const sockaddr_in* v4 = reinterpret_cast<const sockaddr_in*>(&addr);
    err = uv_ip4_name(v4, ip, sizeof(ip));
  }
  if (err != 0)
    return "";
  return ip;
}