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
#endif

// Look up the environment variable and allow the lookup if the current