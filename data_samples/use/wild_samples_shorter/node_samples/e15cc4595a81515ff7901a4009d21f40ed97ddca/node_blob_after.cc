#include "node_errors.h"
#include "node_external_reference.h"
#include "node_file.h"
#include "permission/permission.h"
#include "util.h"
#include "v8.h"

#include <algorithm>

void BlobFromFilePath(const FunctionCallbackInfo<Value>& args) {
  Environment* env = Environment::GetCurrent(args);
  BufferValue path(env->isolate(), args[0]);
  CHECK_NOT_NULL(*path);
  THROW_IF_INSUFFICIENT_PERMISSIONS(
      env, permission::PermissionScope::kFileSystemRead, path.ToStringView());
  auto entry = DataQueue::CreateFdEntry(env, args[0]);
  if (entry == nullptr) {
    return THROW_ERR_INVALID_ARG_VALUE(env, "Unabled to open file as blob");
  }