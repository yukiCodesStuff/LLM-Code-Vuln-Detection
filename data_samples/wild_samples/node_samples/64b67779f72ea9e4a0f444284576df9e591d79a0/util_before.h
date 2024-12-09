class RAIIIsolate {
 public:
  explicit RAIIIsolate(const SnapshotData* data = nullptr);
  ~RAIIIsolate();

  v8::Isolate* get() const { return isolate_.get(); }

 private:
  RAIIIsolateWithoutEntering isolate_;
  v8::Isolate::Scope isolate_scope_;
};

std::string DetermineSpecificErrorType(Environment* env,
                                       v8::Local<v8::Value> input);

v8::Maybe<int32_t> GetValidatedFd(Environment* env, v8::Local<v8::Value> input);
v8::Maybe<int> GetValidFileMode(Environment* env,
                                v8::Local<v8::Value> input,
                                uv_fs_type type);

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS

#endif  // SRC_UTIL_H_