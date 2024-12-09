  'https://*': {
    protocol: 'https:',
    slashes: true,
    auth: null,
    host: '',
    port: null,
    hostname: '',
    hash: null,
    search: null,
    query: null,
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
  let actual = url.parse(u);
  const spaced = url.parse(`     \t  ${u}\n\t`);
  let expected = Object.assign(new url.Url(), parseTests[u]);

  Object.keys(actual).forEach(function(i) {
    if (expected[i] === undefined && actual[i] === null) {
      expected[i] = null;
    }
  });

  assert.deepStrictEqual(
    actual,
    expected,
    `expected ${inspect(expected)}, got ${inspect(actual)}`
  );
  assert.deepStrictEqual(
    spaced,
    expected,
    `expected ${inspect(expected)}, got ${inspect(spaced)}`
  );

  expected = parseTests[u].href;
  actual = url.format(parseTests[u]);

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
    if (expected[i] === undefined && actual[i] === null) {
      expected[i] = null;
    }
  });

  assert.deepStrictEqual(
    actual,
    expected,
    `expected ${inspect(expected)}, got ${inspect(actual)}`
  );
  assert.deepStrictEqual(
    spaced,
    expected,
    `expected ${inspect(expected)}, got ${inspect(spaced)}`
  );

  expected = parseTests[u].href;
  actual = url.format(parseTests[u]);

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