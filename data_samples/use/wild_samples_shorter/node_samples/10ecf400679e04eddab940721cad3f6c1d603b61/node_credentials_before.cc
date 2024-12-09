static bool HasOnly(int capability) {
  DCHECK(cap_valid(capability));

  struct __user_cap_data_struct cap_data[2];
  struct __user_cap_header_struct cap_header_data = {
    _LINUX_CAPABILITY_VERSION_3,
    getpid()};

  if (syscall(SYS_capget, &cap_header_data, &cap_data) != 0) {
    return false;
  }
  if (capability < 32) {
    return cap_data[0].permitted ==
        static_cast<unsigned int>(CAP_TO_MASK(capability));
  }
  return cap_data[1].permitted ==
      static_cast<unsigned int>(CAP_TO_MASK(capability));
}
#endif

// Look up the environment variable and allow the lookup if the current