    pathname: '/*',
    path: '/*',
    href: 'https:///*'
  }
};

for (const u in parseTests) {
  assert.strictEqual(actual, expected,
                     `format(${u}) == ${u}\nactual:${actual}`);
}