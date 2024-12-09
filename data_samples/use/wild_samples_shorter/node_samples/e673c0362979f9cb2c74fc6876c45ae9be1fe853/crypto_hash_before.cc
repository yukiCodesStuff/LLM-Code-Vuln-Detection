  SetMethodNoSideEffect(context, target, "getHashes", GetHashes);

  HashJob::Initialize(env, target);
}

void Hash::RegisterExternalReferences(ExternalReferenceRegistry* registry) {
  registry->Register(New);
  registry->Register(GetHashes);

  HashJob::RegisterExternalReferences(registry);
}

void Hash::New(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  return true;
}

}  // namespace crypto
}  // namespace node