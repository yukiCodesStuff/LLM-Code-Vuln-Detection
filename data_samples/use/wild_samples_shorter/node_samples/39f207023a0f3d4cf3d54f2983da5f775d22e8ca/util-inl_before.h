#include <cmath>
#include <cstring>
#include <locale>
#include "node_revert.h"
#include "util.h"

#define CHAR_TEST(bits, name, expr)                                           \
#else
  static constexpr bool kIsWindows = false;
#endif  // _WIN32
  if (kIsWindows)
    if (const char* p = strrchr(filename, '.'))
      return StringEqualNoCase(p, ".bat") || StringEqualNoCase(p, ".cmd");
  return false;
}

}  // namespace node