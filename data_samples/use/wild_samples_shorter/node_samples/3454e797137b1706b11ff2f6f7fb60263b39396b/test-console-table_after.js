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