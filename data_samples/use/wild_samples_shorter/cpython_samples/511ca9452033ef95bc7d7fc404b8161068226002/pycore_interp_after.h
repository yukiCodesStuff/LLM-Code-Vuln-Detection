    struct types_state types;
    struct callable_cache callable_cache;

    int int_max_str_digits;

    /* The following fields are here to avoid allocation during init.
       The data is exposed through PyInterpreterState pointer fields.
       These fields should not be accessed directly outside of init.
