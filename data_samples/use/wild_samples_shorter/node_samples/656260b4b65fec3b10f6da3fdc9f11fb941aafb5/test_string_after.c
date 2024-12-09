#include <limits.h>  // INT_MAX
#include <string.h>
#include <js_native_api.h>
#include "../common.h"

static napi_value TestLatin1(napi_env env, napi_callback_info info) {
  return output;
}

static napi_value TestMemoryCorruption(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc == 1, "Wrong number of arguments");

  char buf[10] = { 0 };
  NAPI_CALL(env, napi_get_value_string_utf8(env, args[0], buf, 0, NULL));

  char zero[10] = { 0 };
  if (memcmp(buf, zero, sizeof(buf)) != 0) {
    NAPI_CALL(env, napi_throw_error(env, NULL, "Buffer overwritten"));
  }

  return NULL;
}

EXTERN_C_START
napi_value Init(napi_env env, napi_value exports) {
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("TestLatin1", TestLatin1),
    DECLARE_NAPI_PROPERTY("TestLargeUtf8", TestLargeUtf8),
    DECLARE_NAPI_PROPERTY("TestLargeLatin1", TestLargeLatin1),
    DECLARE_NAPI_PROPERTY("TestLargeUtf16", TestLargeUtf16),
    DECLARE_NAPI_PROPERTY("TestMemoryCorruption", TestMemoryCorruption),
  };

  NAPI_CALL(env, napi_define_properties(
      env, exports, sizeof(properties) / sizeof(*properties), properties));