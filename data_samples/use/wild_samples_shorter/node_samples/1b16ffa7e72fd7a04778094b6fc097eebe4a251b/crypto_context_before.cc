
  CHECK_EQ(args.Length(), 2);

  CryptoErrorStore errors;
  Utf8Value engine_id(env->isolate(), args[1]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {
  // support multiple calls to SetClientCertEngine.
  CHECK(!sc->client_cert_engine_provided_);

  CryptoErrorStore errors;
  const Utf8Value engine_id(env->isolate(), args[0]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {