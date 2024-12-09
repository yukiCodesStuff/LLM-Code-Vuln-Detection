static bool IsIPAddress(const std::string& host) {
  if (host.length() >= 4 && host.front() == '[' && host.back() == ']')
    return true;
  uint_fast16_t accum = 0;
  uint_fast8_t quads = 0;
  bool empty = true;
  auto endOctet = [&accum, &quads, &empty](bool final = false) {