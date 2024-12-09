  {
    code: 'ERR_INVALID_ARG_TYPE'
  }
);
[true, Symbol(), {}, () => {}, []].forEach((generator) => assert.throws(
  () => crypto.createDiffieHellman('', 'base64', generator),
  { code: 'ERR_INVALID_ARG_TYPE' }
));