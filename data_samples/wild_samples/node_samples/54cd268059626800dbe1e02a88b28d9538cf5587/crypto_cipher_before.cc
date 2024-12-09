void PublicKeyCipher::Cipher(const FunctionCallbackInfo<Value>& args) {
  MarkPopErrorOnReturn mark_pop_error_on_return;
  Environment* env = Environment::GetCurrent(args);

  unsigned int offset = 0;
  ManagedEVPPKey pkey =
      ManagedEVPPKey::GetPublicOrPrivateKeyFromJs(args, &offset);
  if (!pkey)
    return;

  ArrayBufferOrViewContents<unsigned char> buf(args[offset]);
  if (UNLIKELY(!buf.CheckSizeInt32()))
    return THROW_ERR_OUT_OF_RANGE(env, "buffer is too long");

  uint32_t padding;
  if (!args[offset + 1]->Uint32Value(env->context()).To(&padding)) return;

  const EVP_MD* digest = nullptr;
  if (args[offset + 2]->IsString()) {
    const Utf8Value oaep_str(env->isolate(), args[offset + 2]);
    digest = EVP_get_digestbyname(*oaep_str);
    if (digest == nullptr)
      return THROW_ERR_OSSL_EVP_INVALID_DIGEST(env);
  }