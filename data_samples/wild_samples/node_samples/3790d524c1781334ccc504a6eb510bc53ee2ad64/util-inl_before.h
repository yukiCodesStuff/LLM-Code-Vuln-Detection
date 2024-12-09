bool IsWindowsBatchFile(const char* filename) {
#ifdef _WIN32
  static constexpr bool kIsWindows = true;
#else
  static constexpr bool kIsWindows = false;
#endif  // _WIN32
  if (kIsWindows)
    if (!IsReverted(SECURITY_REVERT_CVE_2024_27980))
      if (const char* p = strrchr(filename, '.'))
        return StringEqualNoCase(p, ".bat") || StringEqualNoCase(p, ".cmd");
  return false;
}

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_UTIL_INL_H_