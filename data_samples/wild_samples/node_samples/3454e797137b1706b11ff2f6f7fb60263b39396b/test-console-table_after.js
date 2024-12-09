{
  const line = '─'.repeat(79);
  const header = `${' '.repeat(37)}name${' '.repeat(40)}`;
  const name = 'very long long long long long long long long long long long ' +
               'long long long long';
  test([{ name }], `
┌─────────┬──${line}──┐
│ (index) │  ${header}│
├─────────┼──${line}──┤
│    0    │ '${name}' │
└─────────┴──${line}──┘
`);
}

test({ foo: '￥', bar: '¥' }, `
┌─────────┬────────┐
│ (index) │ Values │
├─────────┼────────┤
│   foo   │  '￥'  │
│   bar   │  '¥'   │
└─────────┴────────┘
`);

test({ foo: '你好', bar: 'hello' }, `
┌─────────┬─────────┐
│ (index) │ Values  │
├─────────┼─────────┤
│   foo   │ '你好'  │
│   bar   │ 'hello' │
└─────────┴─────────┘
`);

// Regression test for prototype pollution via console.table. Earlier versions
// of Node.js created an object with a non-null prototype within console.table
// and then wrote to object[column][index], which lead to an error as well as
// modifications to Object.prototype.
test([{ foo: 10 }, { foo: 20 }], ['__proto__'], `
┌─────────┬───────────┐
│ (index) │ __proto__ │
├─────────┼───────────┤
│    0    │           │
│    1    │           │
└─────────┴───────────┘
`);
assert.strictEqual('0' in Object.prototype, false);
assert.strictEqual('1' in Object.prototype, false);