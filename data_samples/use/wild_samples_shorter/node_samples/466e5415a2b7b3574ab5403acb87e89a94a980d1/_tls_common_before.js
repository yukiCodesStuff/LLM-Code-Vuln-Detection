
const {
  ArrayPrototypePush,
  ObjectCreate,
  StringPrototypeReplace,
} = primordials;

    // XXX: More key validation?
    StringPrototypeReplace(info, /([^\n:]*):([^\n]*)(?:\n|$)/g,
                           (all, key, val) => {
                             if (key in c.infoAccess)
                               ArrayPrototypePush(c.infoAccess[key], val);
                             else
                               c.infoAccess[key] = [val];