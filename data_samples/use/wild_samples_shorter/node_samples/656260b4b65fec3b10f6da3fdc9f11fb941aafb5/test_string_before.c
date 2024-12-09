#include <limits.h>  // INT_MAX
#include <js_native_api.h>
#include "../common.h"

static napi_value TestLatin1(napi_env env, napi_callback_info info) {
  return output;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("TestLatin1", TestLatin1),
    DECLARE_NAPI_PROPERTY("TestLargeUtf8", TestLargeUtf8),
    DECLARE_NAPI_PROPERTY("TestLargeLatin1", TestLargeLatin1),
    DECLARE_NAPI_PROPERTY("TestLargeUtf16", TestLargeUtf16),
  };

  NAPI_CALL(env, napi_define_properties(
      env, exports, sizeof(properties) / sizeof(*properties), properties));