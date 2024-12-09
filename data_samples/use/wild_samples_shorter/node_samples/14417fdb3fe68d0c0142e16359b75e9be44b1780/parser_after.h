                       Vector<Handle<String> > args);

 private:
  static const int kMaxNumFunctionLocals = 131071;  // 2^17-1

  enum Mode {
    PARSE_LAZILY,