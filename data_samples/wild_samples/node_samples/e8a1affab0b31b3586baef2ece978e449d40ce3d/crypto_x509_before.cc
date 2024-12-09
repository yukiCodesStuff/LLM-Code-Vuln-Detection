void X509Certificate::PublicKey(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  EVPKeyPointer pkey(X509_get_pubkey(cert->get()));
  ManagedEVPPKey epkey(std::move(pkey));
  std::shared_ptr<KeyObjectData> key_data =
      KeyObjectData::CreateAsymmetric(kKeyTypePublic, epkey);

  Local<Value> ret;
  if (KeyObjectHandle::Create(env, key_data).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Pem(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  CHECK(bio);
  if (PEM_write_bio_X509(bio.get(), cert->get()))
    args.GetReturnValue().Set(ToV8Value(env, bio));
}

void X509Certificate::CheckCA(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ClearErrorOnReturn clear_error_on_return;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  args.GetReturnValue().Set(X509_check_ca(cert->get()) == 1);
}

void X509Certificate::CheckHost(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  CHECK(args[0]->IsString());  // name
  CHECK(args[1]->IsUint32());  // flags

  Utf8Value name(env->isolate(), args[0]);
  uint32_t flags = args[1].As<Uint32>()->Value();
  char* peername;

  switch (X509_check_host(
              cert->get(),
              *name,
              name.length(),
              flags,
              &peername)) {
    case 1:  {  // Match!
      Local<Value> ret = args[0];
      if (peername != nullptr) {
        ret = OneByteString(env->isolate(), peername);
        OPENSSL_free(peername);
      }
      return args.GetReturnValue().Set(ret);
    }