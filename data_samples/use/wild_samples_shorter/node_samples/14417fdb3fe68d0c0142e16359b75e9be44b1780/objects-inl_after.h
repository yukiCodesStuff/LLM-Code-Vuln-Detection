         kind == CALL_IC ||
         kind == STORE_IC ||
         kind == KEYED_STORE_IC);
  ASSERT(argc <= Code::kMaxArguments);
  // Compute the bit mask.
  unsigned int bits = KindField::encode(kind)
      | ICStateField::encode(ic_state)
      | TypeField::encode(type)
      | ExtraICStateField::encode(extra_ic_state)
      | (argc << kArgumentsCountShift)