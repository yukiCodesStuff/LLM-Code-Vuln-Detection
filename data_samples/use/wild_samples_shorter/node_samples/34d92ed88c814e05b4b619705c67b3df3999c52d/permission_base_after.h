#define WORKER_THREADS_PERMISSIONS(V)                                          \
  V(WorkerThreads, "worker", PermissionsRoot)

#define INSPECTOR_PERMISSIONS(V) V(Inspector, "inspector", PermissionsRoot)

#define PERMISSIONS(V)                                                         \
  FILESYSTEM_PERMISSIONS(V)                                                    \
  CHILD_PROCESS_PERMISSIONS(V)                                                 \
  WORKER_THREADS_PERMISSIONS(V)                                                \
  INSPECTOR_PERMISSIONS(V)

#define V(name, _, __) k##name,
enum class PermissionScope {
  kPermissionsRoot = -1,