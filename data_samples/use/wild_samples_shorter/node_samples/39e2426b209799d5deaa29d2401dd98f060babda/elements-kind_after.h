const int kFastElementsKindCount = LAST_FAST_ELEMENTS_KIND -
    FIRST_FAST_ELEMENTS_KIND + 1;

int ElementsKindToShiftSize(ElementsKind elements_kind);

void PrintElementsKind(FILE* out, ElementsKind kind);

ElementsKind GetInitialFastElementsKind();
