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