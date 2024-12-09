namespace internal {


void PrintElementsKind(FILE* out, ElementsKind kind) {
  ElementsAccessor* accessor = ElementsAccessor::ForKind(kind);
  PrintF(out, "%s", accessor->name());
}