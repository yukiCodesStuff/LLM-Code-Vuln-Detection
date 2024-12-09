                                v8::Local<v8::Value> input,
                                uv_fs_type type);

// Returns true if OS==Windows and filename ends in .bat or .cmd,
// case insensitive.
inline bool IsWindowsBatchFile(const char* filename);

}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS
