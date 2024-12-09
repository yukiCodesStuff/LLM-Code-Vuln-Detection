#include "env-inl.h"
#include "node_errors.h"
#include "node_external_reference.h"
#include "node_internals.h"
#include "util-inl.h"

#include <unistd.h>  // setuid, getuid
#endif
#ifdef __linux__
#include <dlfcn.h>  // dlsym()
#include <linux/capability.h>
#include <sys/auxv.h>
#include <sys/syscall.h>
#endif  // __linux__
  }
}

#ifdef __linux__
extern "C" {
int uv__node_patch_is_using_io_uring(void);

int uv__node_patch_is_using_io_uring(void) __attribute__((weak));

typedef int (*is_using_io_uring_fn)(void);
}
#endif  // __linux__

static bool UvMightBeUsingIoUring() {
#ifdef __linux__
  // Support for io_uring is only included in libuv 1.45.0 and later, and only
  // on Linux (and Android, but there it is always disabled). The patch that we
  // apply to libuv to work around the io_uring security issue adds a function
  // that tells us whether io_uring is being used. If that function is not
  // present, we assume that we are dynamically linking against an unpatched
  // version.
  static std::atomic<is_using_io_uring_fn> check =
      uv__node_patch_is_using_io_uring;
  if (check == nullptr) {
    check = reinterpret_cast<is_using_io_uring_fn>(
        dlsym(RTLD_DEFAULT, "uv__node_patch_is_using_io_uring"));
  }
  return uv_version() >= 0x012d00u && (check == nullptr || (*check)());
#else
  return false;
#endif
}

static bool ThrowIfUvMightBeUsingIoUring(Environment* env, const char* fn) {
  if (UvMightBeUsingIoUring()) {
    node::THROW_ERR_INVALID_STATE(
        env, "%s() disabled: io_uring may be enabled. See CVE-2024-22017.", fn);
    return true;
  }
  return false;
}

static void GetUid(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  CHECK(env->has_run_bootstrapping_code());
  // uid_t is an uint32_t on all supported platforms.
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsUint32() || args[0]->IsString());

  if (ThrowIfUvMightBeUsingIoUring(env, "setgid")) return;

  gid_t gid = gid_by_name(env->isolate(), args[0]);

  if (gid == gid_not_found) {
    // Tells JS to throw ERR_INVALID_CREDENTIAL
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsUint32() || args[0]->IsString());

  if (ThrowIfUvMightBeUsingIoUring(env, "setegid")) return;

  gid_t gid = gid_by_name(env->isolate(), args[0]);

  if (gid == gid_not_found) {
    // Tells JS to throw ERR_INVALID_CREDENTIAL
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsUint32() || args[0]->IsString());

  if (ThrowIfUvMightBeUsingIoUring(env, "setuid")) return;

  uid_t uid = uid_by_name(env->isolate(), args[0]);

  if (uid == uid_not_found) {
    // Tells JS to throw ERR_INVALID_CREDENTIAL
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsUint32() || args[0]->IsString());

  if (ThrowIfUvMightBeUsingIoUring(env, "seteuid")) return;

  uid_t uid = uid_by_name(env->isolate(), args[0]);

  if (uid == uid_not_found) {
    // Tells JS to throw ERR_INVALID_CREDENTIAL
  CHECK_EQ(args.Length(), 1);
  CHECK(args[0]->IsArray());

  if (ThrowIfUvMightBeUsingIoUring(env, "setgroups")) return;

  Local<Array> groups_list = args[0].As<Array>();
  size_t size = groups_list->Length();
  MaybeStackBuffer<gid_t, 64> groups(size);

  CHECK(args[0]->IsUint32() || args[0]->IsString());
  CHECK(args[1]->IsUint32() || args[1]->IsString());

  if (ThrowIfUvMightBeUsingIoUring(env, "initgroups")) return;

  Utf8Value arg0(env->isolate(), args[0]);
  gid_t extra_group;
  bool must_free;
  char* user;