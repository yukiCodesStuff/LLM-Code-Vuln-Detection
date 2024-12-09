#define WORKER_THREADS_PERMISSIONS(V)                                          \
  V(WorkerThreads, "worker", PermissionsRoot)

#define PERMISSIONS(V)                                                         \
  FILESYSTEM_PERMISSIONS(V)                                                    \
  CHILD_PROCESS_PERMISSIONS(V)                                                 \
  WORKER_THREADS_PERMISSIONS(V)

#define V(name, _, __) k##name,
enum class PermissionScope {
  kPermissionsRoot = -1,