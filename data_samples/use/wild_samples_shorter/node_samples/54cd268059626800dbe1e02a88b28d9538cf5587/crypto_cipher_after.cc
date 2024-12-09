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

  const EVP_MD* digest = nullptr;
  if (args[offset + 2]->IsString()) {
    const Utf8Value oaep_str(env->isolate(), args[offset + 2]);
    digest = EVP_get_digestbyname(*oaep_str);