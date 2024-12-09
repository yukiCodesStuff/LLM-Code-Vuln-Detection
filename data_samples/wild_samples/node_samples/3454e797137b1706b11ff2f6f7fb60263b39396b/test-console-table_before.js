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