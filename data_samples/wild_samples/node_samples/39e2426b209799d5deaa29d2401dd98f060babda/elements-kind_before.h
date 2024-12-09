enum ElementsKind {
  // The "fast" kind for elements that only contain SMI values. Must be first
  // to make it possible to efficiently check maps for this kind.
  FAST_SMI_ELEMENTS,
  FAST_HOLEY_SMI_ELEMENTS,

  // The "fast" kind for tagged values. Must be second to make it possible to
  // efficiently check maps for this and the FAST_SMI_ONLY_ELEMENTS kind
  // together at once.
  FAST_ELEMENTS,
  FAST_HOLEY_ELEMENTS,

  // The "fast" kind for unwrapped, non-tagged double values.
  FAST_DOUBLE_ELEMENTS,
  FAST_HOLEY_DOUBLE_ELEMENTS,

  // The "slow" kind.
  DICTIONARY_ELEMENTS,
  NON_STRICT_ARGUMENTS_ELEMENTS,
  // The "fast" kind for external arrays
  EXTERNAL_BYTE_ELEMENTS,
  EXTERNAL_UNSIGNED_BYTE_ELEMENTS,
  EXTERNAL_SHORT_ELEMENTS,
  EXTERNAL_UNSIGNED_SHORT_ELEMENTS,
  EXTERNAL_INT_ELEMENTS,
  EXTERNAL_UNSIGNED_INT_ELEMENTS,
  EXTERNAL_FLOAT_ELEMENTS,
  EXTERNAL_DOUBLE_ELEMENTS,
  EXTERNAL_PIXEL_ELEMENTS,

  // Derived constants from ElementsKind
  FIRST_ELEMENTS_KIND = FAST_SMI_ELEMENTS,
  LAST_ELEMENTS_KIND = EXTERNAL_PIXEL_ELEMENTS,
  FIRST_FAST_ELEMENTS_KIND = FAST_SMI_ELEMENTS,
  LAST_FAST_ELEMENTS_KIND = FAST_HOLEY_DOUBLE_ELEMENTS,
  FIRST_EXTERNAL_ARRAY_ELEMENTS_KIND = EXTERNAL_BYTE_ELEMENTS,
  LAST_EXTERNAL_ARRAY_ELEMENTS_KIND = EXTERNAL_PIXEL_ELEMENTS,
  TERMINAL_FAST_ELEMENTS_KIND = FAST_HOLEY_ELEMENTS
};

const int kElementsKindCount = LAST_ELEMENTS_KIND - FIRST_ELEMENTS_KIND + 1;
const int kFastElementsKindCount = LAST_FAST_ELEMENTS_KIND -
    FIRST_FAST_ELEMENTS_KIND + 1;

void PrintElementsKind(FILE* out, ElementsKind kind);

ElementsKind GetInitialFastElementsKind();

ElementsKind GetFastElementsKindFromSequenceIndex(int sequence_index);

int GetSequenceIndexFromFastElementsKind(ElementsKind elements_kind);


inline bool IsDictionaryElementsKind(ElementsKind kind) {
  return kind == DICTIONARY_ELEMENTS;
}