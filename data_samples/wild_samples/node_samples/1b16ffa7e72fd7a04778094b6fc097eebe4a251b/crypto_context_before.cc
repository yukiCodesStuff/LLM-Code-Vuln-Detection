void SecureContext::SetEngineKey(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  SecureContext* sc;
  ASSIGN_OR_RETURN_UNWRAP(&sc, args.Holder());

  CHECK_EQ(args.Length(), 2);

  CryptoErrorStore errors;
  Utf8Value engine_id(env->isolate(), args[1]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {
    Local<Value> exception;
    if (errors.ToException(env).ToLocal(&exception))
      env->isolate()->ThrowException(exception);
    return;
  }
    const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsString());

  SecureContext* sc;
  ASSIGN_OR_RETURN_UNWRAP(&sc, args.Holder());

  MarkPopErrorOnReturn mark_pop_error_on_return;

  // SSL_CTX_set_client_cert_engine does not itself support multiple
  // calls by cleaning up before overwriting the client_cert_engine
  // internal context variable.
  // Instead of trying to fix up this problem we in turn also do not
  // support multiple calls to SetClientCertEngine.
  CHECK(!sc->client_cert_engine_provided_);

  CryptoErrorStore errors;
  const Utf8Value engine_id(env->isolate(), args[0]);
  EnginePointer engine = LoadEngineById(*engine_id, &errors);
  if (!engine) {
    Local<Value> exception;
    if (errors.ToException(env).ToLocal(&exception))
      env->isolate()->ThrowException(exception);
    return;
  }