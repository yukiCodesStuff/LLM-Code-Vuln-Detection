void X509Certificate::CheckCA(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  args.GetReturnValue().Set(X509_check_ca(cert->get()) == 1);
}
void X509Certificate::CheckIssued(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  CHECK(args[0]->IsObject());
  CHECK(X509Certificate::HasInstance(env, args[0].As<Object>()));

  X509Certificate* issuer;
  ASSIGN_OR_RETURN_UNWRAP(&issuer, args[0]);

  args.GetReturnValue().Set(
    X509_check_issued(issuer->get(), cert->get()) == X509_V_OK);
}

void X509Certificate::CheckPrivateKey(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  CHECK(args[0]->IsObject());
  KeyObjectHandle* key;
  ASSIGN_OR_RETURN_UNWRAP(&key, args[0]);
  CHECK_EQ(key->Data()->GetKeyType(), kKeyTypePrivate);

  ClearErrorOnReturn clear_error_on_return;

  args.GetReturnValue().Set(
      X509_check_private_key(
          cert->get(),
          key->Data()->GetAsymmetricKey().get()) == 1);
}

void X509Certificate::Verify(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  CHECK(args[0]->IsObject());
  KeyObjectHandle* key;
  ASSIGN_OR_RETURN_UNWRAP(&key, args[0]);
  CHECK_EQ(key->Data()->GetKeyType(), kKeyTypePublic);

  ClearErrorOnReturn clear_error_on_return;

  args.GetReturnValue().Set(
      X509_verify(
          cert->get(),
          key->Data()->GetAsymmetricKey().get()) > 0);
}

void X509Certificate::ToLegacy(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (X509ToObject(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::GetIssuerCert(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  if (cert->issuer_cert_)
    args.GetReturnValue().Set(cert->issuer_cert_->object());
}

X509Certificate::X509Certificate(
    Environment* env,
    Local<Object> object,
    std::shared_ptr<ManagedX509> cert,
    STACK_OF(X509)* issuer_chain)
    : BaseObject(env, object),
      cert_(std::move(cert)) {
  MakeWeak();

  if (issuer_chain != nullptr && sk_X509_num(issuer_chain)) {
    X509Pointer cert(X509_dup(sk_X509_value(issuer_chain, 0)));
    sk_X509_delete(issuer_chain, 0);
    Local<Object> obj = sk_X509_num(issuer_chain)
        ? X509Certificate::New(env, std::move(cert), issuer_chain)
            .ToLocalChecked()
        : X509Certificate::New(env, std::move(cert))
            .ToLocalChecked();
    issuer_cert_.reset(Unwrap<X509Certificate>(obj));
  }
void X509Certificate::ToLegacy(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (X509ToObject(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::GetIssuerCert(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  if (cert->issuer_cert_)
    args.GetReturnValue().Set(cert->issuer_cert_->object());
}

X509Certificate::X509Certificate(
    Environment* env,
    Local<Object> object,
    std::shared_ptr<ManagedX509> cert,
    STACK_OF(X509)* issuer_chain)
    : BaseObject(env, object),
      cert_(std::move(cert)) {
  MakeWeak();

  if (issuer_chain != nullptr && sk_X509_num(issuer_chain)) {
    X509Pointer cert(X509_dup(sk_X509_value(issuer_chain, 0)));
    sk_X509_delete(issuer_chain, 0);
    Local<Object> obj = sk_X509_num(issuer_chain)
        ? X509Certificate::New(env, std::move(cert), issuer_chain)
            .ToLocalChecked()
        : X509Certificate::New(env, std::move(cert))
            .ToLocalChecked();
    issuer_cert_.reset(Unwrap<X509Certificate>(obj));
  }