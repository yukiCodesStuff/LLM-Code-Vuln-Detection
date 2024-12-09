  // FLAGS_MIN_VALUE and FLAGS_MAX_VALUE are specified to ensure that
  // enumeration type has correct value range (see Issue 830 for more details).
  enum Flags {
    FLAGS_MIN_VALUE = kMinInt,
    FLAGS_MAX_VALUE = kMaxInt
  };

#define CODE_KIND_LIST(V) \
  V(FUNCTION)             \
  // Signed field cannot be encoded using the BitField class.
  static const int kArgumentsCountShift = 14;
  static const int kArgumentsCountMask = ~((1 << kArgumentsCountShift) - 1);

  // This constant should be encodable in an ARM instruction.
  static const int kFlagsNotUsedInLookup =
      TypeField::kMask | CacheHolderField::kMask;