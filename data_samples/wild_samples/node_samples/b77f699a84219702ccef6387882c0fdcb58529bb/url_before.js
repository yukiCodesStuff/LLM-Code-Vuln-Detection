  if (slashesDenoteHost || proto || hostPattern.test(rest)) {
    var slashes = rest.charCodeAt(0) === CHAR_FORWARD_SLASH &&
                  rest.charCodeAt(1) === CHAR_FORWARD_SLASH;
    if (slashes && !(proto && hostlessProtocol[proto])) {
      rest = rest.slice(2);
      this.slashes = true;
    }