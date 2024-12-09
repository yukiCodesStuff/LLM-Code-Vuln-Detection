  if (slashesDenoteHost || proto || hostPattern.test(rest)) {
    var slashes = rest.charCodeAt(0) === CHAR_FORWARD_SLASH &&
                  rest.charCodeAt(1) === CHAR_FORWARD_SLASH;
    if (slashes && !(proto && hostlessProtocol[proto])) {
      rest = rest.slice(2);
      this.slashes = true;
    }
  }

  if (!hostlessProtocol[proto] &&
      (slashes || (proto && !slashedProtocol[proto]))) {

    // there's a hostname.
    // the first instance of /, ?, ;, or # ends the host.