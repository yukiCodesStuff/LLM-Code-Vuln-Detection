
  const node::Utf8Value engine_id(env->isolate(), args[0]);

  args.GetReturnValue().Set(SetEngine(*engine_id, flags));
}
#endif  // !OPENSSL_NO_ENGINE
