                Number::New(env->isolate(), static_cast<double>(thread_id_.id)))
      .Check();

  inspector_parent_handle_ =
      GetInspectorParentHandle(env, thread_id_, url.c_str(), name.c_str());

  argv_ = std::vector<std::string>{env->argv()[0]};
  // Mark this Worker object as weak until we actually start the thread.
  MakeWeak();