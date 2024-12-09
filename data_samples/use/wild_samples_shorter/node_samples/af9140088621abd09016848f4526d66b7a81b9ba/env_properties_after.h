  V(napi_type_tag, "node:napi:type_tag")                                       \
  V(napi_wrapper, "node:napi:wrapper")                                         \
  V(untransferable_object_private_symbol, "node:untransferableObject")         \
  V(exit_info_private_symbol, "node:exit_info_private_symbol")                 \
  V(require_private_symbol, "node:require_private_symbol")

// Symbols are per-isolate primitives but Environment proxies them
// for the sake of convenience.
#define PER_ISOLATE_SYMBOL_PROPERTIES(V)                                       \