#include "node_options-inl.h"
#include "node_process-inl.h"
#include "node_url.h"
#include "permission/permission.h"
#include "timer_wrap-inl.h"
#include "util-inl.h"
#include "v8-inspector.h"
#include "v8-platform.h"

#include "libplatform/libplatform.h"
  if (io_ != nullptr)
    return true;

  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "StartIoThread",
                                    false);
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return false;
  }
std::unique_ptr<InspectorSession> Agent::Connect(
    std::unique_ptr<InspectorSessionDelegate> delegate,
    bool prevent_shutdown) {
  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "Connect",
                                    std::unique_ptr<InspectorSession>{});
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return std::unique_ptr<InspectorSession>{};
  }
std::unique_ptr<InspectorSession> Agent::ConnectToMainThread(
    std::unique_ptr<InspectorSessionDelegate> delegate,
    bool prevent_shutdown) {
  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "ConnectToMainThread",
                                    std::unique_ptr<InspectorSession>{});
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return std::unique_ptr<InspectorSession>{};
  }
}

void Agent::WaitForDisconnect() {
  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "WaitForDisconnect");
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return;
  }

std::unique_ptr<ParentInspectorHandle> Agent::GetParentHandle(
    uint64_t thread_id, const std::string& url, const std::string& name) {
  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "GetParentHandle",
                                    std::unique_ptr<ParentInspectorHandle>{});
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return std::unique_ptr<ParentInspectorHandle>{};
  }
}

void Agent::WaitForConnect() {
  THROW_IF_INSUFFICIENT_PERMISSIONS(
      parent_env_, permission::PermissionScope::kInspector, "WaitForConnect");
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return;
  }
}

std::shared_ptr<WorkerManager> Agent::GetWorkerManager() {
  THROW_IF_INSUFFICIENT_PERMISSIONS(parent_env_,
                                    permission::PermissionScope::kInspector,
                                    "GetWorkerManager",
                                    std::unique_ptr<WorkerManager>{});
  if (!parent_env_->should_create_inspector() && !client_) {
    ThrowUninitializedInspectorError(parent_env_);
    return std::unique_ptr<WorkerManager>{};
  }