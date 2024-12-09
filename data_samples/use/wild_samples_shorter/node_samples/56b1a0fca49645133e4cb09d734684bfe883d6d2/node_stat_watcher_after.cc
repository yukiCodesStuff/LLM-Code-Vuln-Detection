#include "memory_tracker-inl.h"
#include "node_external_reference.h"
#include "node_file-inl.h"
#include "permission/permission.h"
#include "util-inl.h"

#include <cstring>
#include <cstdlib>

  node::Utf8Value path(args.GetIsolate(), args[0]);
  CHECK_NOT_NULL(*path);
  THROW_IF_INSUFFICIENT_PERMISSIONS(
      wrap->env(),
      permission::PermissionScope::kFileSystemRead,
      path.ToStringView());

  CHECK(args[1]->IsUint32());
  const uint32_t interval = args[1].As<Uint32>()->Value();
