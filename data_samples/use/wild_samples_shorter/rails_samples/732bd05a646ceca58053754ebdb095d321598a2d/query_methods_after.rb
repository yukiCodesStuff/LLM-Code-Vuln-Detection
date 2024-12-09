    #   # SELECT "users"."name" FROM "users" /* selecting */ /* user */ /* names */
    #
    # The SQL block comment delimiters, "/*" and "*/", will be added automatically.
    #
    # Some escaping is performed, however untrusted user input should not be used.
    def annotate(*args)
      check_if_method_has_arguments!(__callee__, args)
      spawn.annotate!(*args)
    end