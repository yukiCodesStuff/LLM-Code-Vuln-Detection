  int err;
  WriteWrap* wrap;
  size_t bytes;
  BaseObjectPtr<AsyncWrap> wrap_obj;
};

using JSMethodFunction = void(const v8::FunctionCallbackInfo<v8::Value>& args);
