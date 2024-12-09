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

  if (EVP_PKEY_cipher == EVP_PKEY_decrypt &&
      operation == PublicKeyCipher::kPrivate && padding == RSA_PKCS1_PADDING) {
    EVPKeyCtxPointer ctx(EVP_PKEY_CTX_new(pkey.get(), nullptr));
    CHECK(ctx);

    if (EVP_PKEY_decrypt_init(ctx.get()) <= 0) {
      return ThrowCryptoError(env, ERR_get_error());
    }

    int rsa_pkcs1_implicit_rejection =
        EVP_PKEY_CTX_ctrl_str(ctx.get(), "rsa_pkcs1_implicit_rejection", "1");
    // From the doc -2 means that the option is not supported.
    // The default for the option is enabled and if it has been
    // specifically disabled we want to respect that so we will
    // not throw an error if the option is supported regardless
    // of how it is set. The call to set the value
    // will not affect what is used since a different context is
    // used in the call if the option is supported
    if (rsa_pkcs1_implicit_rejection <= 0) {
      return THROW_ERR_INVALID_ARG_VALUE(
          env,
          "RSA_PKCS1_PADDING is no longer supported for private decryption,"
          " this can be reverted with --security-revert=CVE-2024-PEND");
    }
  }