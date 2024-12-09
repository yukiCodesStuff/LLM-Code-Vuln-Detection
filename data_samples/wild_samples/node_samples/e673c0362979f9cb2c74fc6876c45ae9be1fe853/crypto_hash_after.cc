void Hash::Initialize(Environment* env, Local<Object> target) {
  Isolate* isolate = env->isolate();
  Local<Context> context = env->context();
  Local<FunctionTemplate> t = NewFunctionTemplate(isolate, New);

  t->InstanceTemplate()->SetInternalFieldCount(Hash::kInternalFieldCount);

  SetProtoMethod(isolate, t, "update", HashUpdate);
  SetProtoMethod(isolate, t, "digest", HashDigest);

  SetConstructorFunction(context, target, "Hash", t);

  SetMethodNoSideEffect(context, target, "getHashes", GetHashes);

  HashJob::Initialize(env, target);

  SetMethodNoSideEffect(
      context, target, "internalVerifyIntegrity", InternalVerifyIntegrity);
}

void Hash::RegisterExternalReferences(ExternalReferenceRegistry* registry) {
  registry->Register(New);
  registry->Register(HashUpdate);
  registry->Register(HashDigest);
  registry->Register(GetHashes);

  HashJob::RegisterExternalReferences(registry);

  registry->Register(InternalVerifyIntegrity);
}

void Hash::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  const Hash* orig = nullptr;
  const EVP_MD* md = nullptr;

  if (args[0]->IsObject()) {
    ASSIGN_OR_RETURN_UNWRAP(&orig, args[0].As<Object>());
    md = EVP_MD_CTX_md(orig->mdctx_.get());
  } else {
    const Utf8Value hash_type(env->isolate(), args[0]);
    md = EVP_get_digestbyname(*hash_type);
  }

  Maybe<unsigned int> xof_md_len = Nothing<unsigned int>();
  if (!args[1]->IsUndefined()) {
    CHECK(args[1]->IsUint32());
    xof_md_len = Just<unsigned int>(args[1].As<Uint32>()->Value());
  }

  Hash* hash = new Hash(env, args.This());
  if (md == nullptr || !hash->HashInit(md, xof_md_len)) {
    return ThrowCryptoError(env, ERR_get_error(),
                            "Digest method not supported");
  }

  if (orig != nullptr &&
      0 >= EVP_MD_CTX_copy(hash->mdctx_.get(), orig->mdctx_.get())) {
    return ThrowCryptoError(env, ERR_get_error(), "Digest copy error");
  }
}
void Hash::RegisterExternalReferences(ExternalReferenceRegistry* registry) {
  registry->Register(New);
  registry->Register(HashUpdate);
  registry->Register(HashDigest);
  registry->Register(GetHashes);

  HashJob::RegisterExternalReferences(registry);

  registry->Register(InternalVerifyIntegrity);
}

void Hash::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);

  const Hash* orig = nullptr;
  const EVP_MD* md = nullptr;

  if (args[0]->IsObject()) {
    ASSIGN_OR_RETURN_UNWRAP(&orig, args[0].As<Object>());
    md = EVP_MD_CTX_md(orig->mdctx_.get());
  } else {
    const Utf8Value hash_type(env->isolate(), args[0]);
    md = EVP_get_digestbyname(*hash_type);
  }

  Maybe<unsigned int> xof_md_len = Nothing<unsigned int>();
  if (!args[1]->IsUndefined()) {
    CHECK(args[1]->IsUint32());
    xof_md_len = Just<unsigned int>(args[1].As<Uint32>()->Value());
  }

  Hash* hash = new Hash(env, args.This());
  if (md == nullptr || !hash->HashInit(md, xof_md_len)) {
    return ThrowCryptoError(env, ERR_get_error(),
                            "Digest method not supported");
  }

  if (orig != nullptr &&
      0 >= EVP_MD_CTX_copy(hash->mdctx_.get(), orig->mdctx_.get())) {
    return ThrowCryptoError(env, ERR_get_error(), "Digest copy error");
  }
}
  if (LIKELY(params.length > 0)) {
    unsigned int length = params.length;
    ByteSource::Builder buf(length);

    size_t expected = EVP_MD_CTX_size(ctx.get());

    int ret =
        (length == expected)
            ? EVP_DigestFinal_ex(ctx.get(), buf.data<unsigned char>(), &length)
            : EVP_DigestFinalXOF(ctx.get(), buf.data<unsigned char>(), length);

    if (UNLIKELY(ret != 1))
      return false;

    *out = std::move(buf).release();
  }

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