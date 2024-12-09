  () => crypto.createDiffieHellman('', 'base64', generator),
  { code: 'ERR_INVALID_ARG_TYPE' }
));