  SetMethodNoSideEffect(context, target, "getHashes", GetHashes);

  HashJob::Initialize(env, target);

  SetMethodNoSideEffect(
      context, target, "internalVerifyIntegrity", InternalVerifyIntegrity);
}

void Hash::RegisterExternalReferences(ExternalReferenceRegistry* registry) {
  registry->Register(New);
  registry->Register(GetHashes);

  HashJob::RegisterExternalReferences(registry);

  registry->Register(InternalVerifyIntegrity);
}

void Hash::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  return true;
}

void InternalVerifyIntegrity(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  CHECK_EQ(args.Length(), 3);

  CHECK(args[0]->IsString());
  Utf8Value algorithm(env->isolate(), args[0]);

  CHECK(args[1]->IsString() || IsAnyBufferSource(args[1]));
  ByteSource content = ByteSource::FromStringOrBuffer(env, args[1]);

  CHECK(args[2]->IsArrayBufferView());
  ArrayBufferOrViewContents<unsigned char> expected(args[2]);

  const EVP_MD* md_type = EVP_get_digestbyname(*algorithm);
  unsigned char digest[EVP_MAX_MD_SIZE];
  unsigned int digest_size;
  if (md_type == nullptr || EVP_Digest(content.data(),
                                       content.size(),
                                       digest,
                                       &digest_size,
                                       md_type,
                                       nullptr) != 1) {
    return ThrowCryptoError(
        env, ERR_get_error(), "Digest method not supported");
  }

  if (digest_size != expected.size() ||
      CRYPTO_memcmp(digest, expected.data(), digest_size) != 0) {
    Local<Value> error;
    MaybeLocal<Value> rc =
        StringBytes::Encode(env->isolate(),
                            reinterpret_cast<const char*>(digest),
                            digest_size,
                            BASE64,
                            &error);
    if (rc.IsEmpty()) {
      CHECK(!error.IsEmpty());
      env->isolate()->ThrowException(error);
      return;
    }
    args.GetReturnValue().Set(rc.FromMaybe(Local<Value>()));
  }
}

}  // namespace crypto
}  // namespace node