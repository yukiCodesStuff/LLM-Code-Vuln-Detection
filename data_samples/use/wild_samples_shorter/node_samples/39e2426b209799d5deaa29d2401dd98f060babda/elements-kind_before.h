const int kFastElementsKindCount = LAST_FAST_ELEMENTS_KIND -
    FIRST_FAST_ELEMENTS_KIND + 1;

void PrintElementsKind(FILE* out, ElementsKind kind);

ElementsKind GetInitialFastElementsKind();
