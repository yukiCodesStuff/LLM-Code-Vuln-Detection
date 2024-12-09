// Note: Node.js specific errors must begin with the prefix ERR_

E('ERR_ACCESS_DENIED',
  function(msg, permission = '', resource = '') {
    this.permission = permission;
    this.resource = resource;
    return msg;
  },
  Error);
E('ERR_AMBIGUOUS_ARGUMENT', 'The "%s" argument is ambiguous. %s', TypeError);
E('ERR_ARG_NOT_ITERABLE', '%s must be iterable', TypeError);
E('ERR_ASSERTION', '%s', Error);