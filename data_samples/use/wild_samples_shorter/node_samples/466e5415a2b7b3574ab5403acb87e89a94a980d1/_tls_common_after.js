
const {
  ArrayPrototypePush,
  JSONParse,
  ObjectCreate,
  StringPrototypeReplace,
} = primordials;

    // XXX: More key validation?
    StringPrototypeReplace(info, /([^\n:]*):([^\n]*)(?:\n|$)/g,
                           (all, key, val) => {
                             if (val.charCodeAt(0) === 0x22) {
                               // The translatePeerCertificate function is only
                               // used on internally created legacy certificate
                               // objects, and any value that contains a quote
                               // will always be a valid JSON string literal,
                               // so this should never throw.
                               val = JSONParse(val);
                             }
                             if (key in c.infoAccess)
                               ArrayPrototypePush(c.infoAccess[key], val);
                             else
                               c.infoAccess[key] = [val];