    pathname: '/*',
    path: '/*',
    href: 'https:///*'
  },

  // The following two URLs are the same, but they differ for
  // a capital A: it is important that we verify that the protocol
  // is checked in a case-insensitive manner.
  'javascript:alert(1);a=\x27@white-listed.com\x27': {
    protocol: 'javascript:',
    slashes: null,
    auth: null,
    host: null,
    port: null,
    hostname: null,
    hash: null,
    search: null,
    query: null,
    pathname: "alert(1);a='@white-listed.com'",
    path: "alert(1);a='@white-listed.com'",
    href: "javascript:alert(1);a='@white-listed.com'"
  },

  'javAscript:alert(1);a=\x27@white-listed.com\x27': {
    protocol: 'javascript:',
    slashes: null,
    auth: null,
    host: null,
    port: null,
    hostname: null,
    hash: null,
    search: null,
    query: null,
    pathname: "alert(1);a='@white-listed.com'",
    path: "alert(1);a='@white-listed.com'",
    href: "javascript:alert(1);a='@white-listed.com'"
  }
};

for (const u in parseTests) {
  assert.strictEqual(actual, expected,
                     `format(${u}) == ${u}\nactual:${actual}`);
}

{
  const parsed = url.parse('http://nodejs.org/')
    .resolveObject('jAvascript:alert(1);a=\x27@white-listed.com\x27');

  const expected = Object.assign(new url.Url(), {
    protocol: 'javascript:',
    slashes: null,
    auth: null,
    host: null,
    port: null,
    hostname: null,
    hash: null,
    search: null,
    query: null,
    pathname: "alert(1);a='@white-listed.com'",
    path: "alert(1);a='@white-listed.com'",
    href: "javascript:alert(1);a='@white-listed.com'"
  });

  assert.deepStrictEqual(parsed, expected);
}