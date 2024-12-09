  if (parent_port == nullptr) {
    // This can happen e.g. because execution is terminating.
    return;
  }

  child_port_data_ = std::make_unique<MessagePortData>(nullptr);
  MessagePort::Entangle(parent_port, child_port_data_.get());

  object()
      ->Set(env->context(), env->message_port_string(), parent_port->object())
      .Check();

  object()->Set(env->context(),
                env->thread_id_string(),
                Number::New(env->isolate(), static_cast<double>(thread_id_.id)))
      .Check();

  // Without this check, to use the permission model with
  // workers (--allow-worker) one would need to pass --allow-inspector as well
  if (env->permission()->is_granted(
          node::permission::PermissionScope::kInspector)) {
    inspector_parent_handle_ =
        GetInspectorParentHandle(env, thread_id_, url.c_str(), name.c_str());
  }