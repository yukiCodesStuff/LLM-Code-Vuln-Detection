static bool HasOnly(int capability) {
  DCHECK(cap_valid(capability));

  struct __user_cap_data_struct cap_data[_LINUX_CAPABILITY_U32S_3];
  struct __user_cap_header_struct cap_header_data = {
    _LINUX_CAPABILITY_VERSION_3,
    getpid()};


  if (syscall(SYS_capget, &cap_header_data, &cap_data) != 0) {
    return false;
  }

  static_assert(arraysize(cap_data) == 2);
  return cap_data[CAP_TO_INDEX(capability)].permitted ==
             static_cast<unsigned int>(CAP_TO_MASK(capability)) &&
         cap_data[1 - CAP_TO_INDEX(capability)].permitted == 0;
}
  if (syscall(SYS_capget, &cap_header_data, &cap_data) != 0) {
    return false;
  }

  static_assert(arraysize(cap_data) == 2);
  return cap_data[CAP_TO_INDEX(capability)].permitted ==
             static_cast<unsigned int>(CAP_TO_MASK(capability)) &&
         cap_data[1 - CAP_TO_INDEX(capability)].permitted == 0;
}
#endif

// Look up the environment variable and allow the lookup if the current
// process only has the capability CAP_NET_BIND_SERVICE set. If the current
// process does not have any capabilities set and the process is running as
// setuid root then lookup will not be allowed.
bool SafeGetenv(const char* key,
                std::string* text,
                std::shared_ptr<KVStore> env_vars,
                v8::Isolate* isolate) {
#if !defined(__CloudABI__) && !defined(_WIN32)
#if defined(__linux__)
  if ((!HasOnly(CAP_NET_BIND_SERVICE) && linux_at_secure()) ||
      getuid() != geteuid() || getgid() != getegid())
#else
  if (linux_at_secure() || getuid() != geteuid() || getgid() != getegid())
#endif
    goto fail;
#endif

  if (env_vars != nullptr) {
    DCHECK_NOT_NULL(isolate);
    HandleScope handle_scope(isolate);
    TryCatch ignore_errors(isolate);
    MaybeLocal<String> maybe_value = env_vars->Get(
        isolate, String::NewFromUtf8(isolate, key).ToLocalChecked());
    Local<String> value;
    if (!maybe_value.ToLocal(&value)) goto fail;
    String::Utf8Value utf8_value(isolate, value);
    if (*utf8_value == nullptr) goto fail;
    *text = std::string(*utf8_value, utf8_value.length());
    return true;
  }

  {
    Mutex::ScopedLock lock(per_process::env_var_mutex);

    size_t init_sz = 256;
    MaybeStackBuffer<char, 256> val;
    int ret = uv_os_getenv(key, *val, &init_sz);

    if (ret == UV_ENOBUFS) {
      // Buffer is not large enough, reallocate to the updated init_sz
      // and fetch env value again.
      val.AllocateSufficientStorage(init_sz);
      ret = uv_os_getenv(key, *val, &init_sz);
    }