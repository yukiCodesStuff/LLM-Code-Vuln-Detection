
void X509Certificate::CheckCA(const FunctionCallbackInfo<Value>& args) {
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  args.GetReturnValue().Set(X509_check_ca(cert->get()) == 1);
}

  X509Certificate* issuer;
  ASSIGN_OR_RETURN_UNWRAP(&issuer, args[0]);

  args.GetReturnValue().Set(
    X509_check_issued(issuer->get(), cert->get()) == X509_V_OK);
}

  Environment* env = Environment::GetCurrent(args);
  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  Local<Value> ret;
  if (X509ToObject(env, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}