  virtual ~Parser() {
    delete reusable_preparser_;
    reusable_preparser_ = NULL;
  }

  // Returns NULL if parsing failed.
  FunctionLiteral* ParseProgram();
  FunctionLiteral* ParseLazy();

  void ReportMessageAt(Scanner::Location loc,
                       const char* message,
                       Vector<const char*> args);
  void ReportMessageAt(Scanner::Location loc,
                       const char* message,
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
    PARSE_EAGERLY
  };

  enum VariableDeclarationContext {
    kModuleElement,
    kBlockElement,
    kStatement,
    kForStatement
  };

  // If a list of variable declarations includes any initializers.
  enum VariableDeclarationProperties {
    kHasInitializers,
    kHasNoInitializers
  };

  class BlockState;

  class FunctionState BASE_EMBEDDED {
   public:
    FunctionState(Parser* parser,
                  Scope* scope,
                  Isolate* isolate);
    ~FunctionState();

    int NextMaterializedLiteralIndex() {
      return next_materialized_literal_index_++;
    }
    int materialized_literal_count() {
      return next_materialized_literal_index_ - JSFunction::kLiteralsPrefixSize;
    }

    int NextHandlerIndex() { return next_handler_index_++; }
    int handler_count() { return next_handler_index_; }

    void SetThisPropertyAssignmentInfo(
        bool only_simple_this_property_assignments,
        Handle<FixedArray> this_property_assignments) {
      only_simple_this_property_assignments_ =
          only_simple_this_property_assignments;
      this_property_assignments_ = this_property_assignments;
    }
    bool only_simple_this_property_assignments() {
      return only_simple_this_property_assignments_;
    }
    Handle<FixedArray> this_property_assignments() {
      return this_property_assignments_;
    }

    void AddProperty() { expected_property_count_++; }
    int expected_property_count() { return expected_property_count_; }