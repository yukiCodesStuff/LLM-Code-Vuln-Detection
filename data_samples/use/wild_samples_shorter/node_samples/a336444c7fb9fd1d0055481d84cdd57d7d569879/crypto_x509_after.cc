  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (X509ToObject(env, cert->get(), true).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::GetIssuerCert(const FunctionCallbackInfo<Value>& args) {