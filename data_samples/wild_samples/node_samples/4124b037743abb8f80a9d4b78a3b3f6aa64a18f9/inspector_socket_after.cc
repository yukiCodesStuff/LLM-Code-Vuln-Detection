static std::string TrimPort(const std::string& host) {
  size_t last_colon_pos = host.rfind(':');
  if (last_colon_pos == std::string::npos)
    return host;
  size_t bracket = host.rfind(']');
  if (bracket == std::string::npos || last_colon_pos > bracket)
    return host.substr(0, last_colon_pos);
  return host;
}

static bool IsIPAddress(const std::string& host) {
  // TODO(tniessen): add CVEs to the following bullet points
  // To avoid DNS rebinding attacks, we are aware of the following requirements:
  // * the host name must be an IP address,
  // * the IP address must be routable, and
  // * the IP address must be formatted unambiguously.

  // The logic below assumes that the string is null-terminated, so ensure that
  // we did not somehow end up with null characters within the string.
  if (host.find('\0') != std::string::npos) return false;

  // All IPv6 addresses must be enclosed in square brackets, and anything
  // enclosed in square brackets must be an IPv6 address.
  if (host.length() >= 4 && host.front() == '[' && host.back() == ']') {
    // INET6_ADDRSTRLEN is the maximum length of the dual format (including the
    // terminating null character), which is the longest possible representation
    // of an IPv6 address: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd
    if (host.length() - 2 >= INET6_ADDRSTRLEN) return false;

    // Annoyingly, libuv's implementation of inet_pton() deviates from other
    // implementations of the function in that it allows '%' in IPv6 addresses.
    if (host.find('%') != std::string::npos) return false;

    // Parse the IPv6 address to ensure it is syntactically valid.
    char ipv6_str[INET6_ADDRSTRLEN];
    std::copy(host.begin() + 1, host.end() - 1, ipv6_str);
    ipv6_str[host.length()] = '\0';
    unsigned char ipv6[sizeof(struct in6_addr)];
    if (uv_inet_pton(AF_INET6, ipv6_str, ipv6) != 0) return false;

    // The only non-routable IPv6 address is ::/128. It should not be necessary
    // to explicitly reject it because it will still be enclosed in square
    // brackets and not even macOS should make DNS requests in that case, but
    // history has taught us that we cannot be careful enough.
    // Note that RFC 4291 defines both "IPv4-Compatible IPv6 Addresses" and
    // "IPv4-Mapped IPv6 Addresses", which means that there are IPv6 addresses
    // (other than ::/128) that represent non-routable IPv4 addresses. However,
    // this translation assumes that the host is interpreted as an IPv6 address
    // in the first place, at which point DNS rebinding should not be an issue.
    if (std::all_of(ipv6, ipv6 + sizeof(ipv6), [](auto b) { return b == 0; })) {
      return false;
    }

    // It is a syntactically valid and routable IPv6 address enclosed in square
    // brackets. No client should be able to misinterpret this.
    return true;
  }

  // Anything not enclosed in square brackets must be an IPv4 address. It is
  // important here that inet_pton() accepts only the so-called dotted-decimal
  // notation, which is a strict subset of the so-called numbers-and-dots
  // notation that is allowed by inet_aton() and inet_addr(). This subset does
  // not allow hexadecimal or octal number formats.
  unsigned char ipv4[sizeof(struct in_addr)];
  if (uv_inet_pton(AF_INET, host.c_str(), ipv4) != 0) return false;

  // The only strictly non-routable IPv4 address is 0.0.0.0, and macOS will make
  // DNS requests for this IP address, so we need to explicitly reject it. In
  // fact, we can safely reject all of 0.0.0.0/8 (see Section 3.2 of RFC 791 and
  // Section 3.2.1.3 of RFC 1122).
  // Note that inet_pton() stores the IPv4 address in network byte order.
  if (ipv4[0] == 0) return false;

  // It is a routable IPv4 address in dotted-decimal notation.
  return true;
}