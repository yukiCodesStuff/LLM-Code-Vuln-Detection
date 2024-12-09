});

const kIncomingMessage = Symbol('IncomingMessage');
const kRequestTimeout = Symbol('RequestTimeout');
const kOnHeaders = HTTPParser.kOnHeaders | 0;
const kOnHeadersComplete = HTTPParser.kOnHeadersComplete | 0;
const kOnBody = HTTPParser.kOnBody | 0;
const kOnMessageComplete = HTTPParser.kOnMessageComplete | 0;
  incoming.url = url;
  incoming.upgrade = upgrade;

  if (socket) {
    debug('requestTimeout timer moved to req');
    incoming[kRequestTimeout] = incoming.socket[kRequestTimeout];
    incoming.socket[kRequestTimeout] = undefined;
  }

  let n = headers.length;

  // If parser.maxHeaderPairs <= 0 assume that there's no limit.
  if (parser.maxHeaderPairs > 0)
  methods,
  parsers,
  kIncomingMessage,
  kRequestTimeout,
  HTTPParser,
  isLenient,
  prepareError,
};