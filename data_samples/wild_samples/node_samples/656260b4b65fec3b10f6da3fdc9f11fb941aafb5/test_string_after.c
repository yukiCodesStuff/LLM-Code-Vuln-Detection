#include <limits.h>  // INT_MAX
#include <string.h>
#include <js_native_api.h>
#include "../common.h"

static napi_value TestLatin1(napi_env env, napi_callback_info info) {
  size_t argc = 1;
  napi_value args[1];
  NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, NULL, NULL));

  NAPI_ASSERT(env, argc >= 1, "Wrong number of arguments");

  napi_valuetype valuetype;
  NAPI_CALL(env, napi_typeof(env, args[0], &valuetype));

  NAPI_ASSERT(env, valuetype == napi_string,
      "Wrong type of argment. Expects a string.");

  char buffer[128];
  size_t buffer_size = 128;
  size_t copied;

  NAPI_CALL(env,
      napi_get_value_string_latin1(env, args[0], buffer, buffer_size, &copied));

  napi_value output;
  NAPI_CALL(env, napi_create_string_latin1(env, buffer, copied, &output));

  return output;
}
  } else {
    // just throw the expected error as there is nothing to test
    // in this case since we can't overflow
    NAPI_CALL(env, napi_throw_error(env, NULL, "Invalid argument"));
  }

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
  napi_property_descriptor properties[] = {
    DECLARE_NAPI_PROPERTY("TestLatin1", TestLatin1),
    DECLARE_NAPI_PROPERTY("TestLatin1Insufficient", TestLatin1Insufficient),
    DECLARE_NAPI_PROPERTY("TestUtf8", TestUtf8),
    DECLARE_NAPI_PROPERTY("TestUtf8Insufficient", TestUtf8Insufficient),
    DECLARE_NAPI_PROPERTY("TestUtf16", TestUtf16),
    DECLARE_NAPI_PROPERTY("TestUtf16Insufficient", TestUtf16Insufficient),
    DECLARE_NAPI_PROPERTY("Utf16Length", Utf16Length),
    DECLARE_NAPI_PROPERTY("Utf8Length", Utf8Length),
    DECLARE_NAPI_PROPERTY("TestLargeUtf8", TestLargeUtf8),
    DECLARE_NAPI_PROPERTY("TestLargeLatin1", TestLargeLatin1),
    DECLARE_NAPI_PROPERTY("TestLargeUtf16", TestLargeUtf16),
    DECLARE_NAPI_PROPERTY("TestMemoryCorruption", TestMemoryCorruption),
  };

  NAPI_CALL(env, napi_define_properties(
      env, exports, sizeof(properties) / sizeof(*properties), properties));

  return exports;
}
EXTERN_C_END