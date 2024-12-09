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