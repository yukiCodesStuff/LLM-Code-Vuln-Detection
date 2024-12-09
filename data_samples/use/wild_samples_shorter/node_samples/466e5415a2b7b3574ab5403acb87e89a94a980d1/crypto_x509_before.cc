  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetInfoString<NID_subject_alt_name>(env, bio, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::InfoAccess(const FunctionCallbackInfo<Value>& args) {
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());
  BIOPointer bio(BIO_new(BIO_s_mem()));
  Local<Value> ret;
  if (GetInfoString<NID_info_access>(env, bio, cert->get()).ToLocal(&ret))
    args.GetReturnValue().Set(ret);
}

void X509Certificate::ValidFrom(const FunctionCallbackInfo<Value>& args) {