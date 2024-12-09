
  const node::Utf8Value engine_id(env->isolate(), args[0]);

  if (UNLIKELY(env->permission()->enabled())) {
    return THROW_ERR_CRYPTO_CUSTOM_ENGINE_NOT_SUPPORTED(
        env,
        "Programmatic selection of OpenSSL engines is unsupported while the "
        "experimental permission model is enabled");
  }

  args.GetReturnValue().Set(SetEngine(*engine_id, flags));
}
#endif  // !OPENSSL_NO_ENGINE
