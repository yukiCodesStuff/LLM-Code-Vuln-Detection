#include <cmath>
#include <cstring>
#include <locale>
#include <regex>  // NOLINT(build/c++11)
#include "node_revert.h"
#include "util.h"

#define CHAR_TEST(bits, name, expr)                                           \
#else
  static constexpr bool kIsWindows = false;
#endif  // _WIN32
  if (kIsWindows) {
    std::string file_with_extension = filename;
    // Regex to match the last extension part after the last dot, ignoring
    // trailing spaces and dots
    std::regex extension_regex(R"(\.([a-zA-Z0-9]+)\s*[\.\s]*$)");
    std::smatch match;
    std::string extension;

    if (std::regex_search(file_with_extension, match, extension_regex)) {
      extension = ToLower(match[1].str());
    }

    return !extension.empty() && (extension == "cmd" || extension == "bat");
  }
  return false;
}

}  // namespace node