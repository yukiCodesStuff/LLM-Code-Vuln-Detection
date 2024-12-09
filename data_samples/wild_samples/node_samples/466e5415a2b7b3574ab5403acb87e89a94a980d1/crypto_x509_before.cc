void X509Certificate::SubjectAltName(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetInfoString<NID_subject_alt_name>(env, bio, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::InfoAccess(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetInfoString<NID_info_access>(env, bio, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::ValidFrom(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetValidFrom(env, cert->get(), bio).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::ValidTo(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetValidTo(env, cert->get(), bio).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha1(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint256(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha256(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint512(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha512(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::KeyUsage(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetKeyUsage(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::SerialNumber(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetSerialNumber(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Raw(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetRawDERCertificate(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

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
  if (PEM_write_bio_X509(bio.get(), cert->get()))
    args.GetReturnValue().Set(ToV8Value(env, bio));
}

void X509Certificate::CheckCA(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
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
void X509Certificate::InfoAccess(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetInfoString<NID_info_access>(env, bio, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::ValidFrom(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetValidFrom(env, cert->get(), bio).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::ValidTo(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetValidTo(env, cert->get(), bio).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha1(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint256(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha256(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Fingerprint512(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetFingerprintDigest(env, EVP_sha512(), cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::KeyUsage(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetKeyUsage(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::SerialNumber(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetSerialNumber(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::Raw(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (GetRawDERCertificate(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

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
  if (PEM_write_bio_X509(bio.get(), cert->get()))
    args.GetReturnValue().Set(ToV8Value(env, bio));
}

void X509Certificate::CheckCA(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
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