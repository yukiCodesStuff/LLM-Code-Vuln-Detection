  while (!done) {
    Expression* argument = ParseAssignmentExpression(true, CHECK_OK);
    result->Add(argument, zone());
    if (result->length() > Code::kMaxArguments) {
      ReportMessageAt(scanner().location(), "too_many_arguments",
                      Vector<const char*>::empty());
      *ok = false;
      return NULL;

      top_scope_->DeclareParameter(param_name, VAR);
      num_parameters++;
      if (num_parameters > Code::kMaxArguments) {
        ReportMessageAt(scanner().location(), "too_many_parameters",
                        Vector<const char*>::empty());
        *ok = false;
        return NULL;