  } else {
    return;
  }

  if (!constant->HasInteger32Value()) return;
  int32_t value = constant->Integer32Value() * sign;
  // We limit offset values to 30 bits because we want to avoid the risk of
  // overflows when the offset is added to the object header size.
  if (value >= 1 << array_operation->MaxIndexOffsetBits() || value < 0) return;
  array_operation->SetKey(subexpression);
  if (index->HasNoUses()) {
    index->DeleteAndReplaceWith(NULL);
  }