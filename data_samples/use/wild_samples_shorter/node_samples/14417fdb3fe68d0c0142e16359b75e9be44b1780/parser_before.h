                       Vector<Handle<String> > args);

 private:
  // Limit on number of function parameters is chosen arbitrarily.
  // Code::Flags uses only the low 17 bits of num-parameters to
  // construct a hashable id, so if more than 2^17 are allowed, this
  // should be checked.
  static const int kMaxNumFunctionParameters = 32766;
  static const int kMaxNumFunctionLocals = 131071;  // 2^17-1

  enum Mode {
    PARSE_LAZILY,