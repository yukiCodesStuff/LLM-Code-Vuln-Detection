  uint32_t padding;
  if (!args[offset + 1]->Uint32Value(env->context()).To(&padding)) return;

  const EVP_MD* digest = nullptr;
  if (args[offset + 2]->IsString()) {
    const Utf8Value oaep_str(env->isolate(), args[offset + 2]);
    digest = EVP_get_digestbyname(*oaep_str);