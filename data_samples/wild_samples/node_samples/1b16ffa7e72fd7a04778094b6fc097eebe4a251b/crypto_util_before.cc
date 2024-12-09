void SetEngine(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK(args.Length() >= 2 && args[0]->IsString());
  uint32_t flags;
  if (!args[1]->Uint32Value(env->context()).To(&flags)) return;

  const node::Utf8Value engine_id(env->isolate(), args[0]);

  args.GetReturnValue().Set(SetEngine(*engine_id, flags));
}
#endif  // !OPENSSL_NO_ENGINE

MaybeLocal<Value> EncodeBignum(
    Environment* env,
    const BIGNUM* bn,
    int size,
    Local<Value>* error) {
  std::vector<uint8_t> buf(size);
  CHECK_EQ(BN_bn2binpad(bn, buf.data(), size), size);
  return StringBytes::Encode(
      env->isolate(),
      reinterpret_cast<const char*>(buf.data()),
      buf.size(),
      BASE64URL,
      error);
}

Maybe<bool> SetEncodedValue(
    Environment* env,
    Local<Object> target,
    Local<String> name,
    const BIGNUM* bn,
    int size) {
  Local<Value> value;
  Local<Value> error;
  CHECK_NOT_NULL(bn);
  if (size == 0)
    size = BN_num_bytes(bn);
  if (!EncodeBignum(env, bn, size, &error).ToLocal(&value)) {
    if (!error.IsEmpty())
      env->isolate()->ThrowException(error);
    return Nothing<bool>();
  }