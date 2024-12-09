#include <cmath>
#include <cstring>
#include <locale>
#include "node_revert.h"
#include "util.h"

// These are defined by <sys/byteorder.h> or <netinet/in.h> on some systems.
// To avoid warnings, undefine them before redefining them.
  return name_;
}

// Inline so the compiler can fully optimize it away on Unix platforms.
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
