
  CHECK_EQ(args.Length(), 2);

  if (UNLIKELY(env->permission()->enabled())) {
    return THROW_ERR_CRYPTO_CUSTOM_ENGINE_NOT_SUPPORTED(
        env,
        "Programmatic selection of OpenSSL engines is unsupported while the "
        "experimental permission model is enabled");
  }

  CryptoErrorStore errors;
  Utf8Value engine_id(env->isolate(), args[1]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {
  // support multiple calls to SetClientCertEngine.
  CHECK(!sc->client_cert_engine_provided_);

  if (UNLIKELY(env->permission()->enabled())) {
    return THROW_ERR_CRYPTO_CUSTOM_ENGINE_NOT_SUPPORTED(
        env,
        "Programmatic selection of OpenSSL engines is unsupported while the "
        "experimental permission model is enabled");
  }

  CryptoErrorStore errors;
  const Utf8Value engine_id(env->isolate(), args[0]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {