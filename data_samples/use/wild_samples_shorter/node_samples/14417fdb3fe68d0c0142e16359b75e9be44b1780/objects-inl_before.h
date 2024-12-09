         kind == CALL_IC ||
         kind == STORE_IC ||
         kind == KEYED_STORE_IC);
  // Compute the bit mask.
  int bits = KindField::encode(kind)
      | ICStateField::encode(ic_state)
      | TypeField::encode(type)
      | ExtraICStateField::encode(extra_ic_state)
      | (argc << kArgumentsCountShift)