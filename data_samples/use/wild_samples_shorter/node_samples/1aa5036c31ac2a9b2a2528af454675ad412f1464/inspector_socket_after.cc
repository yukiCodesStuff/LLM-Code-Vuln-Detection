static bool IsIPAddress(const std::string& host) {
  if (host.length() >= 4 && host.front() == '[' && host.back() == ']')
    return true;
  uint_fast16_t accum = 0;
  uint_fast8_t quads = 0;
  bool empty = true;
  auto endOctet = [&accum, &quads, &empty](bool final = false) {
    return !empty && accum <= 0xff && ++quads <= 4 && final == (quads == 4) &&
           (empty = true) && !(accum = 0);
  };
  for (char c : host) {
    if (isdigit(c)) {
      if ((accum = (accum * 10) + (c - '0')) > 0xff) return false;
      empty = false;
    } else if (c != '.' || !endOctet()) {
      return false;
    }
  }
  return endOctet(true);
}

// Constants for hybi-10 frame format.
