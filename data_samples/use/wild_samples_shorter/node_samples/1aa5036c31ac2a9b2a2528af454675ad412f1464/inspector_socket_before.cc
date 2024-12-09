static bool IsIPAddress(const std::string& host) {
  if (host.length() >= 4 && host.front() == '[' && host.back() == ']')
    return true;
  int quads = 0;
  for (char c : host) {
    if (c == '.')
      quads++;
    else if (!isdigit(c))
      return false;
  }
  return quads == 3;
}

// Constants for hybi-10 frame format.
