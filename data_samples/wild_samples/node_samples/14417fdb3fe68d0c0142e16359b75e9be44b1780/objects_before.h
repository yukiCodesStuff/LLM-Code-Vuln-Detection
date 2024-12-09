class Code: public HeapObject {
 public:
  // Opaque data type for encapsulating code flags like kind, inline
  // cache state, and arguments count.
  // FLAGS_MIN_VALUE and FLAGS_MAX_VALUE are specified to ensure that
  // enumeration type has correct value range (see Issue 830 for more details).
  enum Flags {
    FLAGS_MIN_VALUE = kMinInt,
    FLAGS_MAX_VALUE = kMaxInt
  };

#define CODE_KIND_LIST(V) \
  V(FUNCTION)             \
  V(OPTIMIZED_FUNCTION)   \
  V(STUB)                 \
  V(BUILTIN)              \
  V(LOAD_IC)              \
  V(KEYED_LOAD_IC)        \
  V(CALL_IC)              \
  V(KEYED_CALL_IC)        \
  V(STORE_IC)             \
  V(KEYED_STORE_IC)       \
  V(UNARY_OP_IC)          \
  V(BINARY_OP_IC)         \
  V(COMPARE_IC)           \
  V(TO_BOOLEAN_IC)

  enum Kind {
#define DEFINE_CODE_KIND_ENUM(name) name,
    CODE_KIND_LIST(DEFINE_CODE_KIND_ENUM)
#undef DEFINE_CODE_KIND_ENUM

    // Pseudo-kinds.
    LAST_CODE_KIND = TO_BOOLEAN_IC,
    REGEXP = BUILTIN,
    FIRST_IC_KIND = LOAD_IC,
    LAST_IC_KIND = TO_BOOLEAN_IC
  };

  // No more than 16 kinds. The value is currently encoded in four bits in
  // Flags.
  STATIC_ASSERT(LAST_CODE_KIND < 16);

  // Types of stubs.
  enum StubType {
    NORMAL,
    FIELD,
    CONSTANT_FUNCTION,
    CALLBACKS,
    INTERCEPTOR,
    MAP_TRANSITION,
    NONEXISTENT
  };

  enum {
    NUMBER_OF_KINDS = LAST_IC_KIND + 1
  };

  typedef int ExtraICState;

  static const ExtraICState kNoExtraICState = 0;

#ifdef ENABLE_DISASSEMBLER
  // Printing
  static const char* Kind2String(Kind kind);
  static const char* ICState2String(InlineCacheState state);
  static const char* StubType2String(StubType type);
  static void PrintExtraICState(FILE* out, Kind kind, ExtraICState extra);
  inline void Disassemble(const char* name) {
    Disassemble(name, stdout);
  }
  void Disassemble(const char* name, FILE* out);
#endif  // ENABLE_DISASSEMBLER

  // [instruction_size]: Size of the native instructions
  inline int instruction_size();
  inline void set_instruction_size(int value);

  // [relocation_info]: Code relocation information
  DECL_ACCESSORS(relocation_info, ByteArray)
  void InvalidateRelocation();

  // [handler_table]: Fixed array containing offsets of exception handlers.
  DECL_ACCESSORS(handler_table, FixedArray)

  // [deoptimization_data]: Array containing data for deopt.
  DECL_ACCESSORS(deoptimization_data, FixedArray)

  // [type_feedback_info]: Struct containing type feedback information.
  // Will contain either a TypeFeedbackInfo object, or undefined.
  DECL_ACCESSORS(type_feedback_info, Object)

  // [gc_metadata]: Field used to hold GC related metadata. The contents of this
  // field does not have to be traced during garbage collection since
  // it is only used by the garbage collector itself.
  DECL_ACCESSORS(gc_metadata, Object)

  // [ic_age]: Inline caching age: the value of the Heap::global_ic_age
  // at the moment when this object was created.
  inline void set_ic_age(int count);
  inline int ic_age();

  // Unchecked accessors to be used during GC.
  inline ByteArray* unchecked_relocation_info();
  inline FixedArray* unchecked_deoptimization_data();

  inline int relocation_size();

  // [flags]: Various code flags.
  inline Flags flags();
  inline void set_flags(Flags flags);

  // [flags]: Access to specific code flags.
  inline Kind kind();
  inline InlineCacheState ic_state();  // Only valid for IC stubs.
  inline ExtraICState extra_ic_state();  // Only valid for IC stubs.
  inline StubType type();  // Only valid for monomorphic IC stubs.
  inline int arguments_count();  // Only valid for call IC stubs.

  // Testers for IC stub kinds.
  inline bool is_inline_cache_stub();
  inline bool is_load_stub() { return kind() == LOAD_IC; }
  inline bool is_keyed_load_stub() { return kind() == KEYED_LOAD_IC; }
  inline bool is_store_stub() { return kind() == STORE_IC; }
  inline bool is_keyed_store_stub() { return kind() == KEYED_STORE_IC; }
  inline void CodePrint() {
    CodePrint(stdout);
  }
  void CodePrint(FILE* out);
#endif
  DECLARE_VERIFIER(Code)

  void ClearInlineCaches();
  void ClearTypeFeedbackCells(Heap* heap);

  // Max loop nesting marker used to postpose OSR. We don't take loop
  // nesting that is deeper than 5 levels into account.
  static const int kMaxLoopNestingMarker = 6;

  // Layout description.
  static const int kInstructionSizeOffset = HeapObject::kHeaderSize;
  static const int kRelocationInfoOffset = kInstructionSizeOffset + kIntSize;
  static const int kHandlerTableOffset = kRelocationInfoOffset + kPointerSize;
  static const int kDeoptimizationDataOffset =
      kHandlerTableOffset + kPointerSize;
  static const int kTypeFeedbackInfoOffset =
      kDeoptimizationDataOffset + kPointerSize;
  static const int kGCMetadataOffset = kTypeFeedbackInfoOffset + kPointerSize;
  static const int kICAgeOffset =
      kGCMetadataOffset + kPointerSize;
  static const int kFlagsOffset = kICAgeOffset + kIntSize;
  static const int kKindSpecificFlags1Offset = kFlagsOffset + kIntSize;
  static const int kKindSpecificFlags2Offset =
      kKindSpecificFlags1Offset + kIntSize;

  static const int kHeaderPaddingStart = kKindSpecificFlags2Offset + kIntSize;

  // Add padding to align the instruction start following right after
  // the Code object header.
  static const int kHeaderSize =
      (kHeaderPaddingStart + kCodeAlignmentMask) & ~kCodeAlignmentMask;

  // Byte offsets within kKindSpecificFlags1Offset.
  static const int kOptimizableOffset = kKindSpecificFlags1Offset;
  static const int kCheckTypeOffset = kKindSpecificFlags1Offset;

  static const int kFullCodeFlags = kOptimizableOffset + 1;
  class FullCodeFlagsHasDeoptimizationSupportField:
      public BitField<bool, 0, 1> {};  // NOLINT
  class FullCodeFlagsHasDebugBreakSlotsField: public BitField<bool, 1, 1> {};
  class FullCodeFlagsIsCompiledOptimizable: public BitField<bool, 2, 1> {};

  static const int kAllowOSRAtLoopNestingLevelOffset = kFullCodeFlags + 1;
  static const int kProfilerTicksOffset = kAllowOSRAtLoopNestingLevelOffset + 1;

  // Flags layout.  BitField<type, shift, size>.
  class ICStateField: public BitField<InlineCacheState, 0, 3> {};
  class TypeField: public BitField<StubType, 3, 3> {};
  class CacheHolderField: public BitField<InlineCacheHolderFlag, 6, 1> {};
  class KindField: public BitField<Kind, 7, 4> {};
  class ExtraICStateField: public BitField<ExtraICState, 11, 2> {};
  class IsPregeneratedField: public BitField<bool, 13, 1> {};

  // KindSpecificFlags1 layout (STUB and OPTIMIZED_FUNCTION)
  static const int kStackSlotsFirstBit = 0;
  static const int kStackSlotsBitCount = 24;
  static const int kUnaryOpTypeFirstBit =
      kStackSlotsFirstBit + kStackSlotsBitCount;
  static const int kUnaryOpTypeBitCount = 3;
  static const int kBinaryOpTypeFirstBit =
      kStackSlotsFirstBit + kStackSlotsBitCount;
  static const int kBinaryOpTypeBitCount = 3;
  static const int kBinaryOpResultTypeFirstBit =
      kBinaryOpTypeFirstBit + kBinaryOpTypeBitCount;
  static const int kBinaryOpResultTypeBitCount = 3;
  static const int kCompareStateFirstBit =
      kStackSlotsFirstBit + kStackSlotsBitCount;
  static const int kCompareStateBitCount = 3;
  static const int kCompareOperationFirstBit =
      kCompareStateFirstBit + kCompareStateBitCount;
  static const int kCompareOperationBitCount = 4;
  static const int kToBooleanStateFirstBit =
      kStackSlotsFirstBit + kStackSlotsBitCount;
  static const int kToBooleanStateBitCount = 8;
  static const int kHasFunctionCacheFirstBit =
      kStackSlotsFirstBit + kStackSlotsBitCount;
  static const int kHasFunctionCacheBitCount = 1;

  STATIC_ASSERT(kStackSlotsFirstBit + kStackSlotsBitCount <= 32);
  STATIC_ASSERT(kUnaryOpTypeFirstBit + kUnaryOpTypeBitCount <= 32);
  STATIC_ASSERT(kBinaryOpTypeFirstBit + kBinaryOpTypeBitCount <= 32);
  STATIC_ASSERT(kBinaryOpResultTypeFirstBit +
                kBinaryOpResultTypeBitCount <= 32);
  STATIC_ASSERT(kCompareStateFirstBit + kCompareStateBitCount <= 32);
  STATIC_ASSERT(kCompareOperationFirstBit + kCompareOperationBitCount <= 32);
  STATIC_ASSERT(kToBooleanStateFirstBit + kToBooleanStateBitCount <= 32);
  STATIC_ASSERT(kHasFunctionCacheFirstBit + kHasFunctionCacheBitCount <= 32);

  class StackSlotsField: public BitField<int,
      kStackSlotsFirstBit, kStackSlotsBitCount> {};  // NOLINT
  class UnaryOpTypeField: public BitField<int,
      kUnaryOpTypeFirstBit, kUnaryOpTypeBitCount> {};  // NOLINT
  class BinaryOpTypeField: public BitField<int,
      kBinaryOpTypeFirstBit, kBinaryOpTypeBitCount> {};  // NOLINT
  class BinaryOpResultTypeField: public BitField<int,
      kBinaryOpResultTypeFirstBit, kBinaryOpResultTypeBitCount> {};  // NOLINT
  class CompareStateField: public BitField<int,
      kCompareStateFirstBit, kCompareStateBitCount> {};  // NOLINT
  class CompareOperationField: public BitField<int,
      kCompareOperationFirstBit, kCompareOperationBitCount> {};  // NOLINT
  class ToBooleanStateField: public BitField<int,
      kToBooleanStateFirstBit, kToBooleanStateBitCount> {};  // NOLINT
  class HasFunctionCacheField: public BitField<bool,
      kHasFunctionCacheFirstBit, kHasFunctionCacheBitCount> {};  // NOLINT

  // KindSpecificFlags2 layout (STUB and OPTIMIZED_FUNCTION)
  static const int kStubMajorKeyFirstBit = 0;
  static const int kSafepointTableOffsetFirstBit =
      kStubMajorKeyFirstBit + kStubMajorKeyBits;
  static const int kSafepointTableOffsetBitCount = 26;

  STATIC_ASSERT(kStubMajorKeyFirstBit + kStubMajorKeyBits <= 32);
  STATIC_ASSERT(kSafepointTableOffsetFirstBit +
                kSafepointTableOffsetBitCount <= 32);

  class SafepointTableOffsetField: public BitField<int,
      kSafepointTableOffsetFirstBit,
      kSafepointTableOffsetBitCount> {};  // NOLINT
  class StubMajorKeyField: public BitField<int,
      kStubMajorKeyFirstBit, kStubMajorKeyBits> {};  // NOLINT

  // KindSpecificFlags2 layout (FUNCTION)
  class StackCheckTableOffsetField: public BitField<int, 0, 31> {};

  // Signed field cannot be encoded using the BitField class.
  static const int kArgumentsCountShift = 14;
  static const int kArgumentsCountMask = ~((1 << kArgumentsCountShift) - 1);

  // This constant should be encodable in an ARM instruction.
  static const int kFlagsNotUsedInLookup =
      TypeField::kMask | CacheHolderField::kMask;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Code);
};


// All heap objects have a Map that describes their structure.
//  A Map contains information about:
//  - Size information about the object
//  - How to iterate over an object (for garbage collection)
class Map: public HeapObject {
 public:
  // Instance size.
  // Size in bytes or kVariableSizeSentinel if instances do not have
  // a fixed size.
  inline int instance_size();
  inline void set_instance_size(int value);

  // Count of properties allocated in the object.
  inline int inobject_properties();
  inline void set_inobject_properties(int value);

  // Count of property fields pre-allocated in the object when first allocated.
  inline int pre_allocated_property_fields();
  inline void set_pre_allocated_property_fields(int value);

  // Instance type.
  inline InstanceType instance_type();
  inline void set_instance_type(InstanceType value);

  // Tells how many unused property fields are available in the
  // instance (only used for JSObject in fast mode).
  inline int unused_property_fields();
  inline void set_unused_property_fields(int value);

  // Bit field.
  inline byte bit_field();
  inline void set_bit_field(byte value);

  // Bit field 2.
  inline byte bit_field2();
  inline void set_bit_field2(byte value);

  // Bit field 3.
  inline int bit_field3();
  inline void set_bit_field3(int value);

  class EnumLengthBits:             public BitField<int,   0, 11> {};
  class NumberOfOwnDescriptorsBits: public BitField<int,  11, 11> {};
  class IsShared:                   public BitField<bool, 22,  1> {};
  class FunctionWithPrototype:      public BitField<bool, 23,  1> {};
  class DictionaryMap:              public BitField<bool, 24,  1> {};
  class OwnsDescriptors:            public BitField<bool, 25,  1> {};

  // Tells whether the object in the prototype property will be used
  // for instances created from this function.  If the prototype
  // property is set to a value that is not a JSObject, the prototype
  // property will not be used to create instances of the function.
  // See ECMA-262, 13.2.2.
  inline void set_non_instance_prototype(bool value);
  inline bool has_non_instance_prototype();

  // Tells whether function has special prototype property. If not, prototype
  // property will not be created when accessed (will return undefined),
  // and construction from this function will not be allowed.
  inline void set_function_with_prototype(bool value);
  inline bool function_with_prototype();

  // Tells whether the instance with this map should be ignored by the
  // __proto__ accessor.
  inline void set_is_hidden_prototype() {
    set_bit_field(bit_field() | (1 << kIsHiddenPrototype));
  }

  inline bool is_hidden_prototype() {
    return ((1 << kIsHiddenPrototype) & bit_field()) != 0;
  }

  // Records and queries whether the instance has a named interceptor.
  inline void set_has_named_interceptor() {
    set_bit_field(bit_field() | (1 << kHasNamedInterceptor));
  }

  inline bool has_named_interceptor() {
    return ((1 << kHasNamedInterceptor) & bit_field()) != 0;
  }

  // Records and queries whether the instance has an indexed interceptor.
  inline void set_has_indexed_interceptor() {
    set_bit_field(bit_field() | (1 << kHasIndexedInterceptor));
  }

  inline bool has_indexed_interceptor() {
    return ((1 << kHasIndexedInterceptor) & bit_field()) != 0;
  }

  // Tells whether the instance is undetectable.
  // An undetectable object is a special class of JSObject: 'typeof' operator
  // returns undefined, ToBoolean returns false. Otherwise it behaves like
  // a normal JS object.  It is useful for implementing undetectable
  // document.all in Firefox & Safari.
  // See https://bugzilla.mozilla.org/show_bug.cgi?id=248549.
  inline void set_is_undetectable() {
    set_bit_field(bit_field() | (1 << kIsUndetectable));
  }

  inline bool is_undetectable() {
    return ((1 << kIsUndetectable) & bit_field()) != 0;
  }

  // Tells whether the instance has a call-as-function handler.
  inline void set_has_instance_call_handler() {
    set_bit_field(bit_field() | (1 << kHasInstanceCallHandler));
  }

  inline bool has_instance_call_handler() {
    return ((1 << kHasInstanceCallHandler) & bit_field()) != 0;
  }

  inline void set_is_extensible(bool value);
  inline bool is_extensible();

  inline void set_elements_kind(ElementsKind elements_kind) {
    ASSERT(elements_kind < kElementsKindCount);
    ASSERT(kElementsKindCount <= (1 << kElementsKindBitCount));
    set_bit_field2((bit_field2() & ~kElementsKindMask) |
        (elements_kind << kElementsKindShift));
    ASSERT(this->elements_kind() == elements_kind);
  }

  inline ElementsKind elements_kind() {
    return static_cast<ElementsKind>(
        (bit_field2() & kElementsKindMask) >> kElementsKindShift);
  }

  // Tells whether the instance has fast elements that are only Smis.
  inline bool has_fast_smi_elements() {
    return IsFastSmiElementsKind(elements_kind());
  }

  // Tells whether the instance has fast elements.
  inline bool has_fast_object_elements() {
    return IsFastObjectElementsKind(elements_kind());
  }

  inline bool has_fast_smi_or_object_elements() {
    return IsFastSmiOrObjectElementsKind(elements_kind());
  }

  inline bool has_fast_double_elements() {
    return IsFastDoubleElementsKind(elements_kind());
  }

  inline bool has_non_strict_arguments_elements() {
    return elements_kind() == NON_STRICT_ARGUMENTS_ELEMENTS;
  }

  inline bool has_external_array_elements() {
    return IsExternalArrayElementsKind(elements_kind());
  }

  inline bool has_dictionary_elements() {
    return IsDictionaryElementsKind(elements_kind());
  }

  inline bool has_slow_elements_kind() {
    return elements_kind() == DICTIONARY_ELEMENTS
        || elements_kind() == NON_STRICT_ARGUMENTS_ELEMENTS;
  }

  static bool IsValidElementsTransition(ElementsKind from_kind,
                                        ElementsKind to_kind);

  inline bool HasTransitionArray();
  inline bool HasElementsTransition();
  inline Map* elements_transition_map();
  MUST_USE_RESULT inline MaybeObject* set_elements_transition_map(
      Map* transitioned_map);
  inline void SetTransition(int transition_index, Map* target);
  inline Map* GetTransition(int transition_index);
  MUST_USE_RESULT inline MaybeObject* AddTransition(String* key,
                                                    Map* target,
                                                    SimpleTransitionFlag flag);
  DECL_ACCESSORS(transitions, TransitionArray)
  inline void ClearTransitions(Heap* heap,
                               WriteBarrierMode mode = UPDATE_WRITE_BARRIER);

  // Tells whether the map is attached to SharedFunctionInfo
  // (for inobject slack tracking).
  inline void set_attached_to_shared_function_info(bool value);

  inline bool attached_to_shared_function_info();

  // Tells whether the map is shared between objects that may have different
  // behavior. If true, the map should never be modified, instead a clone
  // should be created and modified.
  inline void set_is_shared(bool value);
  inline bool is_shared();

  // Tells whether the map is used for JSObjects in dictionary mode (ie
  // normalized objects, ie objects for which HasFastProperties returns false).
  // A map can never be used for both dictionary mode and fast mode JSObjects.
  // False by default and for HeapObjects that are not JSObjects.
  inline void set_dictionary_map(bool value);
  inline bool is_dictionary_map();

  // Tells whether the instance needs security checks when accessing its
  // properties.
  inline void set_is_access_check_needed(bool access_check_needed);
  inline bool is_access_check_needed();

  // [prototype]: implicit prototype object.
  DECL_ACCESSORS(prototype, Object)

  // [constructor]: points back to the function responsible for this map.
  DECL_ACCESSORS(constructor, Object)

  inline JSFunction* unchecked_constructor();

  // [instance descriptors]: describes the object.
  DECL_ACCESSORS(instance_descriptors, DescriptorArray)
  inline void InitializeDescriptors(DescriptorArray* descriptors);

  // [stub cache]: contains stubs compiled for this map.
  DECL_ACCESSORS(code_cache, Object)

  // [back pointer]: points back to the parent map from which a transition
  // leads to this map. The field overlaps with prototype transitions and the
  // back pointer will be moved into the prototype transitions array if
  // required.
  inline Object* GetBackPointer();
  inline void SetBackPointer(Object* value,
                             WriteBarrierMode mode = UPDATE_WRITE_BARRIER);
  inline void init_back_pointer(Object* undefined);

  // [prototype transitions]: cache of prototype transitions.
  // Prototype transition is a transition that happens
  // when we change object's prototype to a new one.
  // Cache format:
  //    0: finger - index of the first free cell in the cache
  //    1: back pointer that overlaps with prototype transitions field.
  //    2 + 2 * i: prototype
  //    3 + 2 * i: target map
  inline FixedArray* GetPrototypeTransitions();
  MUST_USE_RESULT inline MaybeObject* SetPrototypeTransitions(
      FixedArray* prototype_transitions);
  inline bool HasPrototypeTransitions();

  inline HeapObject* UncheckedPrototypeTransitions();
  inline TransitionArray* unchecked_transition_array();

  static const int kProtoTransitionHeaderSize = 1;
  static const int kProtoTransitionNumberOfEntriesOffset = 0;
  static const int kProtoTransitionElementsPerEntry = 2;
  static const int kProtoTransitionPrototypeOffset = 0;
  static const int kProtoTransitionMapOffset = 1;

  inline int NumberOfProtoTransitions() {
    FixedArray* cache = GetPrototypeTransitions();
    if (cache->length() == 0) return 0;
    return
        Smi::cast(cache->get(kProtoTransitionNumberOfEntriesOffset))->value();
  }

  inline void SetNumberOfProtoTransitions(int value) {
    FixedArray* cache = GetPrototypeTransitions();
    ASSERT(cache->length() != 0);
    cache->set_unchecked(kProtoTransitionNumberOfEntriesOffset,
                         Smi::FromInt(value));
  }

  // Lookup in the map's instance descriptors and fill out the result
  // with the given holder if the name is found. The holder may be
  // NULL when this function is used from the compiler.
  inline void LookupDescriptor(JSObject* holder,
                               String* name,
                               LookupResult* result);

  inline void LookupTransition(JSObject* holder,
                               String* name,
                               LookupResult* result);

  // The size of transition arrays are limited so they do not end up in large
  // object space. Otherwise ClearNonLiveTransitions would leak memory while
  // applying in-place right trimming.
  inline bool CanHaveMoreTransitions();

  int LastAdded() {
    int number_of_own_descriptors = NumberOfOwnDescriptors();
    ASSERT(number_of_own_descriptors > 0);
    return number_of_own_descriptors - 1;
  }

  int NumberOfOwnDescriptors() {
    return NumberOfOwnDescriptorsBits::decode(bit_field3());
  }

  void SetNumberOfOwnDescriptors(int number) {
    ASSERT(number <= instance_descriptors()->number_of_descriptors());
    set_bit_field3(NumberOfOwnDescriptorsBits::update(bit_field3(), number));
  }

  inline JSGlobalPropertyCell* RetrieveDescriptorsPointer();

  int EnumLength() {
    return EnumLengthBits::decode(bit_field3());
  }

  void SetEnumLength(int length) {
    if (length != kInvalidEnumCache) {
      ASSERT(length >= 0);
      ASSERT(length == 0 || instance_descriptors()->HasEnumCache());
      ASSERT(length <= NumberOfOwnDescriptors());
    }
    set_bit_field3(EnumLengthBits::update(bit_field3(), length));
  }


  inline bool owns_descriptors();
  inline void set_owns_descriptors(bool is_shared);

  MUST_USE_RESULT MaybeObject* RawCopy(int instance_size);
  MUST_USE_RESULT MaybeObject* CopyWithPreallocatedFieldDescriptors();
  MUST_USE_RESULT MaybeObject* CopyDropDescriptors();
  MUST_USE_RESULT MaybeObject* CopyReplaceDescriptors(
      DescriptorArray* descriptors,
      String* name,
      TransitionFlag flag,
      int descriptor_index);
  MUST_USE_RESULT MaybeObject* ShareDescriptor(DescriptorArray* descriptors,
                                               Descriptor* descriptor);
  MUST_USE_RESULT MaybeObject* CopyAddDescriptor(Descriptor* descriptor,
                                                 TransitionFlag flag);
  MUST_USE_RESULT MaybeObject* CopyInsertDescriptor(Descriptor* descriptor,
                                                    TransitionFlag flag);
  MUST_USE_RESULT MaybeObject* CopyReplaceDescriptor(
      DescriptorArray* descriptors,
      Descriptor* descriptor,
      int index,
      TransitionFlag flag);
  MUST_USE_RESULT MaybeObject* CopyAsElementsKind(ElementsKind kind,
                                                  TransitionFlag flag);

  MUST_USE_RESULT MaybeObject* CopyNormalized(PropertyNormalizationMode mode,
                                              NormalizedMapSharingMode sharing);

  inline void AppendDescriptor(Descriptor* desc,
                               const DescriptorArray::WhitenessWitness&);

  // Returns a copy of the map, with all transitions dropped from the
  // instance descriptors.
  MUST_USE_RESULT MaybeObject* Copy();

  // Returns the property index for name (only valid for FAST MODE).
  int PropertyIndexFor(String* name);

  // Returns the next free property index (only valid for FAST MODE).
  int NextFreePropertyIndex();

  // Returns the number of properties described in instance_descriptors
  // filtering out properties with the specified attributes.
  int NumberOfDescribedProperties(DescriptorFlag which = OWN_DESCRIPTORS,
                                  PropertyAttributes filter = NONE);

  // Casting.
  static inline Map* cast(Object* obj);

  // Locate an accessor in the instance descriptor.
  AccessorDescriptor* FindAccessor(String* name);

  // Code cache operations.

  // Clears the code cache.
  inline void ClearCodeCache(Heap* heap);

  // Update code cache.
  static void UpdateCodeCache(Handle<Map> map,
                              Handle<String> name,
                              Handle<Code> code);
  MUST_USE_RESULT MaybeObject* UpdateCodeCache(String* name, Code* code);

  // Extend the descriptor array of the map with the list of descriptors.
  // In case of duplicates, the latest descriptor is used.
  static void AppendCallbackDescriptors(Handle<Map> map,
                                        Handle<Object> descriptors);

  static void EnsureDescriptorSlack(Handle<Map> map, int slack);

  // Returns the found code or undefined if absent.
  Object* FindInCodeCache(String* name, Code::Flags flags);

  // Returns the non-negative index of the code object if it is in the
  // cache and -1 otherwise.
  int IndexInCodeCache(Object* name, Code* code);

  // Removes a code object from the code cache at the given index.
  void RemoveFromCodeCache(String* name, Code* code, int index);

  // Set all map transitions from this map to dead maps to null.  Also clear
  // back pointers in transition targets so that we do not process this map
  // again while following back pointers.
  void ClearNonLiveTransitions(Heap* heap);

  // Computes a hash value for this map, to be used in HashTables and such.
  int Hash();

  // Compares this map to another to see if they describe equivalent objects.
  // If |mode| is set to CLEAR_INOBJECT_PROPERTIES, |other| is treated as if
  // it had exactly zero inobject properties.
  // The "shared" flags of both this map and |other| are ignored.
  bool EquivalentToForNormalization(Map* other, PropertyNormalizationMode mode);

  // Returns the map that this map transitions to if its elements_kind
  // is changed to |elements_kind|, or NULL if no such map is cached yet.
  // |safe_to_add_transitions| is set to false if adding transitions is not
  // allowed.
  Map* LookupElementsTransitionMap(ElementsKind elements_kind);

  // Returns the transitioned map for this map with the most generic
  // elements_kind that's found in |candidates|, or null handle if no match is
  // found at all.
  Handle<Map> FindTransitionedMap(MapHandleList* candidates);
  Map* FindTransitionedMap(MapList* candidates);

  // Zaps the contents of backing data structures. Note that the
  // heap verifier (i.e. VerifyMarkingVisitor) relies on zapping of objects
  // holding weak references when incremental marking is used, because it also
  // iterates over objects that are otherwise unreachable.
  // In general we only want to call these functions in release mode when
  // heap verification is turned on.
  void ZapPrototypeTransitions();
  void ZapTransitions();

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void MapPrint() {
    MapPrint(stdout);
  }
  void MapPrint(FILE* out);
#endif
  DECLARE_VERIFIER(Map)

#ifdef VERIFY_HEAP
  void SharedMapVerify();
#endif

  inline int visitor_id();
  inline void set_visitor_id(int visitor_id);

  typedef void (*TraverseCallback)(Map* map, void* data);

  void TraverseTransitionTree(TraverseCallback callback, void* data);

  // When you set the prototype of an object using the __proto__ accessor you
  // need a new map for the object (the prototype is stored in the map).  In
  // order not to multiply maps unnecessarily we store these as transitions in
  // the original map.  That way we can transition to the same map if the same
  // prototype is set, rather than creating a new map every time.  The
  // transitions are in the form of a map where the keys are prototype objects
  // and the values are the maps the are transitioned to.
  static const int kMaxCachedPrototypeTransitions = 256;

  Map* GetPrototypeTransition(Object* prototype);

  MUST_USE_RESULT MaybeObject* PutPrototypeTransition(Object* prototype,
                                                      Map* map);

  static const int kMaxPreAllocatedPropertyFields = 255;

  // Constant for denoting that the enum cache is not yet initialized.
  static const int kInvalidEnumCache = EnumLengthBits::kMax;

  // Layout description.
  static const int kInstanceSizesOffset = HeapObject::kHeaderSize;
  static const int kInstanceAttributesOffset = kInstanceSizesOffset + kIntSize;
  static const int kPrototypeOffset = kInstanceAttributesOffset + kIntSize;
  static const int kConstructorOffset = kPrototypeOffset + kPointerSize;
  // Storage for the transition array is overloaded to directly contain a back
  // pointer if unused. When the map has transitions, the back pointer is
  // transferred to the transition array and accessed through an extra
  // indirection.
  static const int kTransitionsOrBackPointerOffset =
      kConstructorOffset + kPointerSize;
  static const int kDescriptorsOffset =
      kTransitionsOrBackPointerOffset + kPointerSize;
  static const int kCodeCacheOffset =
      kDescriptorsOffset + kPointerSize;
  static const int kBitField3Offset = kCodeCacheOffset + kPointerSize;
  static const int kSize = kBitField3Offset + kPointerSize;

  // Layout of pointer fields. Heap iteration code relies on them
  // being continuously allocated.
  static const int kPointerFieldsBeginOffset = Map::kPrototypeOffset;
  static const int kPointerFieldsEndOffset = kBitField3Offset + kPointerSize;

  // Byte offsets within kInstanceSizesOffset.
  static const int kInstanceSizeOffset = kInstanceSizesOffset + 0;
  static const int kInObjectPropertiesByte = 1;
  static const int kInObjectPropertiesOffset =
      kInstanceSizesOffset + kInObjectPropertiesByte;
  static const int kPreAllocatedPropertyFieldsByte = 2;
  static const int kPreAllocatedPropertyFieldsOffset =
      kInstanceSizesOffset + kPreAllocatedPropertyFieldsByte;
  static const int kVisitorIdByte = 3;
  static const int kVisitorIdOffset = kInstanceSizesOffset + kVisitorIdByte;

  // Byte offsets within kInstanceAttributesOffset attributes.
  static const int kInstanceTypeOffset = kInstanceAttributesOffset + 0;
  static const int kUnusedPropertyFieldsOffset = kInstanceAttributesOffset + 1;
  static const int kBitFieldOffset = kInstanceAttributesOffset + 2;
  static const int kBitField2Offset = kInstanceAttributesOffset + 3;

  STATIC_CHECK(kInstanceTypeOffset == Internals::kMapInstanceTypeOffset);

  // Bit positions for bit field.
  static const int kUnused = 0;  // To be used for marking recently used maps.
  static const int kHasNonInstancePrototype = 1;
  static const int kIsHiddenPrototype = 2;
  static const int kHasNamedInterceptor = 3;
  static const int kHasIndexedInterceptor = 4;
  static const int kIsUndetectable = 5;
  static const int kHasInstanceCallHandler = 6;
  static const int kIsAccessCheckNeeded = 7;

  // Bit positions for bit field 2
  static const int kIsExtensible = 0;
  static const int kStringWrapperSafeForDefaultValueOf = 1;
  static const int kAttachedToSharedFunctionInfo = 2;
  // No bits can be used after kElementsKindFirstBit, they are all reserved for
  // storing ElementKind.
  static const int kElementsKindShift = 3;
  static const int kElementsKindBitCount = 5;

  // Derived values from bit field 2
  static const int kElementsKindMask = (-1 << kElementsKindShift) &
      ((1 << (kElementsKindShift + kElementsKindBitCount)) - 1);
  static const int8_t kMaximumBitField2FastElementValue = static_cast<int8_t>(
      (FAST_ELEMENTS + 1) << Map::kElementsKindShift) - 1;
  static const int8_t kMaximumBitField2FastSmiElementValue =
      static_cast<int8_t>((FAST_SMI_ELEMENTS + 1) <<
                          Map::kElementsKindShift) - 1;
  static const int8_t kMaximumBitField2FastHoleyElementValue =
      static_cast<int8_t>((FAST_HOLEY_ELEMENTS + 1) <<
                          Map::kElementsKindShift) - 1;
  static const int8_t kMaximumBitField2FastHoleySmiElementValue =
      static_cast<int8_t>((FAST_HOLEY_SMI_ELEMENTS + 1) <<
                          Map::kElementsKindShift) - 1;

  typedef FixedBodyDescriptor<kPointerFieldsBeginOffset,
                              kPointerFieldsEndOffset,
                              kSize> BodyDescriptor;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Map);
};


// An abstract superclass, a marker class really, for simple structure classes.
// It doesn't carry much functionality but allows struct classes to be
// identified in the type system.
class Struct: public HeapObject {
 public:
  inline void InitializeBody(int object_size);
  static inline Struct* cast(Object* that);
};


// Script describes a script which has been added to the VM.
class Script: public Struct {
 public:
  // Script types.
  enum Type {
    TYPE_NATIVE = 0,
    TYPE_EXTENSION = 1,
    TYPE_NORMAL = 2
  };

  // Script compilation types.
  enum CompilationType {
    COMPILATION_TYPE_HOST = 0,
    COMPILATION_TYPE_EVAL = 1
  };

  // Script compilation state.
  enum CompilationState {
    COMPILATION_STATE_INITIAL = 0,
    COMPILATION_STATE_COMPILED = 1
  };

  // [source]: the script source.
  DECL_ACCESSORS(source, Object)

  // [name]: the script name.
  DECL_ACCESSORS(name, Object)

  // [id]: the script id.
  DECL_ACCESSORS(id, Object)

  // [line_offset]: script line offset in resource from where it was extracted.
  DECL_ACCESSORS(line_offset, Smi)

  // [column_offset]: script column offset in resource from where it was
  // extracted.
  DECL_ACCESSORS(column_offset, Smi)

  // [data]: additional data associated with this script.
  DECL_ACCESSORS(data, Object)

  // [context_data]: context data for the context this script was compiled in.
  DECL_ACCESSORS(context_data, Object)

  // [wrapper]: the wrapper cache.
  DECL_ACCESSORS(wrapper, Foreign)

  // [type]: the script type.
  DECL_ACCESSORS(type, Smi)

  // [compilation]: how the the script was compiled.
  DECL_ACCESSORS(compilation_type, Smi)

  // [is_compiled]: determines whether the script has already been compiled.
  DECL_ACCESSORS(compilation_state, Smi)

  // [line_ends]: FixedArray of line ends positions.
  DECL_ACCESSORS(line_ends, Object)

  // [eval_from_shared]: for eval scripts the shared funcion info for the
  // function from which eval was called.
  DECL_ACCESSORS(eval_from_shared, Object)

  // [eval_from_instructions_offset]: the instruction offset in the code for the
  // function from which eval was called where eval was called.
  DECL_ACCESSORS(eval_from_instructions_offset, Smi)

  static inline Script* cast(Object* obj);

  // If script source is an external string, check that the underlying
  // resource is accessible. Otherwise, always return true.
  inline bool HasValidSource();

#ifdef OBJECT_PRINT
  inline void ScriptPrint() {
    ScriptPrint(stdout);
  }
  void ScriptPrint(FILE* out);
#endif
  DECLARE_VERIFIER(Script)

  static const int kSourceOffset = HeapObject::kHeaderSize;
  static const int kNameOffset = kSourceOffset + kPointerSize;
  static const int kLineOffsetOffset = kNameOffset + kPointerSize;
  static const int kColumnOffsetOffset = kLineOffsetOffset + kPointerSize;
  static const int kDataOffset = kColumnOffsetOffset + kPointerSize;
  static const int kContextOffset = kDataOffset + kPointerSize;
  static const int kWrapperOffset = kContextOffset + kPointerSize;
  static const int kTypeOffset = kWrapperOffset + kPointerSize;
  static const int kCompilationTypeOffset = kTypeOffset + kPointerSize;
  static const int kCompilationStateOffset =
      kCompilationTypeOffset + kPointerSize;
  static const int kLineEndsOffset = kCompilationStateOffset + kPointerSize;
  static const int kIdOffset = kLineEndsOffset + kPointerSize;
  static const int kEvalFromSharedOffset = kIdOffset + kPointerSize;
  static const int kEvalFrominstructionsOffsetOffset =
      kEvalFromSharedOffset + kPointerSize;
  static const int kSize = kEvalFrominstructionsOffsetOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Script);
};


// List of builtin functions we want to identify to improve code
// generation.
//
// Each entry has a name of a global object property holding an object
// optionally followed by ".prototype", a name of a builtin function
// on the object (the one the id is set for), and a label.
//
// Installation of ids for the selected builtin functions is handled
// by the bootstrapper.
//
// NOTE: Order is important: math functions should be at the end of
// the list and MathFloor should be the first math function.
#define FUNCTIONS_WITH_ID_LIST(V)                   \
  V(Array.prototype, push, ArrayPush)               \
  V(Array.prototype, pop, ArrayPop)                 \
  V(Function.prototype, apply, FunctionApply)       \
  V(String.prototype, charCodeAt, StringCharCodeAt) \
  V(String.prototype, charAt, StringCharAt)         \
  V(String, fromCharCode, StringFromCharCode)       \
  V(Math, floor, MathFloor)                         \
  V(Math, round, MathRound)                         \
  V(Math, ceil, MathCeil)                           \
  V(Math, abs, MathAbs)                             \
  V(Math, log, MathLog)                             \
  V(Math, sin, MathSin)                             \
  V(Math, cos, MathCos)                             \
  V(Math, tan, MathTan)                             \
  V(Math, asin, MathASin)                           \
  V(Math, acos, MathACos)                           \
  V(Math, atan, MathATan)                           \
  V(Math, exp, MathExp)                             \
  V(Math, sqrt, MathSqrt)                           \
  V(Math, pow, MathPow)                             \
  V(Math, random, MathRandom)                       \
  V(Math, max, MathMax)                             \
  V(Math, min, MathMin)


enum BuiltinFunctionId {
#define DECLARE_FUNCTION_ID(ignored1, ignore2, name)    \
  k##name,
  FUNCTIONS_WITH_ID_LIST(DECLARE_FUNCTION_ID)
#undef DECLARE_FUNCTION_ID
  // Fake id for a special case of Math.pow. Note, it continues the
  // list of math functions.
  kMathPowHalf,
  kFirstMathFunctionId = kMathFloor
};


// SharedFunctionInfo describes the JSFunction information that can be
// shared by multiple instances of the function.
class SharedFunctionInfo: public HeapObject {
 public:
  // [name]: Function name.
  DECL_ACCESSORS(name, Object)

  // [code]: Function code.
  DECL_ACCESSORS(code, Code)

  // [optimized_code_map]: Map from native context to optimized code
  // and a shared literals array or Smi 0 if none.
  DECL_ACCESSORS(optimized_code_map, Object)

  // Returns index i of the entry with the specified context. At position
  // i - 1 is the context, position i the code, and i + 1 the literals array.
  // Returns -1 when no matching entry is found.
  int SearchOptimizedCodeMap(Context* native_context);

  // Installs optimized code from the code map on the given closure. The
  // index has to be consistent with a search result as defined above.
  void InstallFromOptimizedCodeMap(JSFunction* function, int index);

  // Clear optimized code map.
  void ClearOptimizedCodeMap();

  // Add a new entry to the optimized code map.
  static void AddToOptimizedCodeMap(Handle<SharedFunctionInfo> shared,
                                    Handle<Context> native_context,
                                    Handle<Code> code,
                                    Handle<FixedArray> literals);
  static const int kEntryLength = 3;

  // [scope_info]: Scope info.
  DECL_ACCESSORS(scope_info, ScopeInfo)

  // [construct stub]: Code stub for constructing instances of this function.
  DECL_ACCESSORS(construct_stub, Code)

  inline Code* unchecked_code();

  // Returns if this function has been compiled to native code yet.
  inline bool is_compiled();

  // [length]: The function length - usually the number of declared parameters.
  // Use up to 2^30 parameters.
  inline int length();
  inline void set_length(int value);

  // [formal parameter count]: The declared number of parameters.
  inline int formal_parameter_count();
  inline void set_formal_parameter_count(int value);

  // Set the formal parameter count so the function code will be
  // called without using argument adaptor frames.
  inline void DontAdaptArguments();

  // [expected_nof_properties]: Expected number of properties for the function.
  inline int expected_nof_properties();
  inline void set_expected_nof_properties(int value);

  // Inobject slack tracking is the way to reclaim unused inobject space.
  //
  // The instance size is initially determined by adding some slack to
  // expected_nof_properties (to allow for a few extra properties added
  // after the constructor). There is no guarantee that the extra space
  // will not be wasted.
  //
  // Here is the algorithm to reclaim the unused inobject space:
  // - Detect the first constructor call for this SharedFunctionInfo.
  //   When it happens enter the "in progress" state: remember the
  //   constructor's initial_map and install a special construct stub that
  //   counts constructor calls.
  // - While the tracking is in progress create objects filled with
  //   one_pointer_filler_map instead of undefined_value. This way they can be
  //   resized quickly and safely.
  // - Once enough (kGenerousAllocationCount) objects have been created
  //   compute the 'slack' (traverse the map transition tree starting from the
  //   initial_map and find the lowest value of unused_property_fields).
  // - Traverse the transition tree again and decrease the instance size
  //   of every map. Existing objects will resize automatically (they are
  //   filled with one_pointer_filler_map). All further allocations will
  //   use the adjusted instance size.
  // - Decrease expected_nof_properties so that an allocations made from
  //   another context will use the adjusted instance size too.
  // - Exit "in progress" state by clearing the reference to the initial_map
  //   and setting the regular construct stub (generic or inline).
  //
  //  The above is the main event sequence. Some special cases are possible
  //  while the tracking is in progress:
  //
  // - GC occurs.
  //   Check if the initial_map is referenced by any live objects (except this
  //   SharedFunctionInfo). If it is, continue tracking as usual.
  //   If it is not, clear the reference and reset the tracking state. The
  //   tracking will be initiated again on the next constructor call.
  //
  // - The constructor is called from another context.
  //   Immediately complete the tracking, perform all the necessary changes
  //   to maps. This is  necessary because there is no efficient way to track
  //   multiple initial_maps.
  //   Proceed to create an object in the current context (with the adjusted
  //   size).
  //
  // - A different constructor function sharing the same SharedFunctionInfo is
  //   called in the same context. This could be another closure in the same
  //   context, or the first function could have been disposed.
  //   This is handled the same way as the previous case.
  //
  //  Important: inobject slack tracking is not attempted during the snapshot
  //  creation.

  static const int kGenerousAllocationCount = 8;

  // [construction_count]: Counter for constructor calls made during
  // the tracking phase.
  inline int construction_count();
  inline void set_construction_count(int value);

  // [initial_map]: initial map of the first function called as a constructor.
  // Saved for the duration of the tracking phase.
  // This is a weak link (GC resets it to undefined_value if no other live
  // object reference this map).
  DECL_ACCESSORS(initial_map, Object)

  // True if the initial_map is not undefined and the countdown stub is
  // installed.
  inline bool IsInobjectSlackTrackingInProgress();

  // Starts the tracking.
  // Stores the initial map and installs the countdown stub.
  // IsInobjectSlackTrackingInProgress is normally true after this call,
  // except when tracking have not been started (e.g. the map has no unused
  // properties or the snapshot is being built).
  void StartInobjectSlackTracking(Map* map);

  // Completes the tracking.
  // IsInobjectSlackTrackingInProgress is false after this call.
  void CompleteInobjectSlackTracking();

  // Invoked before pointers in SharedFunctionInfo are being marked.
  // Also clears the optimized code map.
  inline void BeforeVisitingPointers();

  // Clears the initial_map before the GC marking phase to ensure the reference
  // is weak. IsInobjectSlackTrackingInProgress is false after this call.
  void DetachInitialMap();

  // Restores the link to the initial map after the GC marking phase.
  // IsInobjectSlackTrackingInProgress is true after this call.
  void AttachInitialMap(Map* map);

  // False if there are definitely no live objects created from this function.
  // True if live objects _may_ exist (existence not guaranteed).
  // May go back from true to false after GC.
  DECL_BOOLEAN_ACCESSORS(live_objects_may_exist)

  // [instance class name]: class name for instances.
  DECL_ACCESSORS(instance_class_name, Object)

  // [function data]: This field holds some additional data for function.
  // Currently it either has FunctionTemplateInfo to make benefit the API
  // or Smi identifying a builtin function.
  // In the long run we don't want all functions to have this field but
  // we can fix that when we have a better model for storing hidden data
  // on objects.
  DECL_ACCESSORS(function_data, Object)

  inline bool IsApiFunction();
  inline FunctionTemplateInfo* get_api_func_data();
  inline bool HasBuiltinFunctionId();
  inline BuiltinFunctionId builtin_function_id();

  // [script info]: Script from which the function originates.
  DECL_ACCESSORS(script, Object)

  // [num_literals]: Number of literals used by this function.
  inline int num_literals();
  inline void set_num_literals(int value);

  // [start_position_and_type]: Field used to store both the source code
  // position, whether or not the function is a function expression,
  // and whether or not the function is a toplevel function. The two
  // least significants bit indicates whether the function is an
  // expression and the rest contains the source code position.
  inline int start_position_and_type();
  inline void set_start_position_and_type(int value);

  // [debug info]: Debug information.
  DECL_ACCESSORS(debug_info, Object)

  // [inferred name]: Name inferred from variable or property
  // assignment of this function. Used to facilitate debugging and
  // profiling of JavaScript code written in OO style, where almost
  // all functions are anonymous but are assigned to object
  // properties.
  DECL_ACCESSORS(inferred_name, String)

  // The function's name if it is non-empty, otherwise the inferred name.
  String* DebugName();

  // Position of the 'function' token in the script source.
  inline int function_token_position();
  inline void set_function_token_position(int function_token_position);

  // Position of this function in the script source.
  inline int start_position();
  inline void set_start_position(int start_position);

  // End position of this function in the script source.
  inline int end_position();
  inline void set_end_position(int end_position);

  // Is this function a function expression in the source code.
  DECL_BOOLEAN_ACCESSORS(is_expression)

  // Is this function a top-level function (scripts, evals).
  DECL_BOOLEAN_ACCESSORS(is_toplevel)

  // Bit field containing various information collected by the compiler to
  // drive optimization.
  inline int compiler_hints();
  inline void set_compiler_hints(int value);

  inline int ast_node_count();
  inline void set_ast_node_count(int count);

  // A counter used to determine when to stress the deoptimizer with a
  // deopt.
  inline int stress_deopt_counter();
  inline void set_stress_deopt_counter(int counter);

  inline int profiler_ticks();

  // Inline cache age is used to infer whether the function survived a context
  // disposal or not. In the former case we reset the opt_count.
  inline int ic_age();
  inline void set_ic_age(int age);

  // Add information on assignments of the form this.x = ...;
  void SetThisPropertyAssignmentsInfo(
      bool has_only_simple_this_property_assignments,
      FixedArray* this_property_assignments);

  // Clear information on assignments of the form this.x = ...;
  void ClearThisPropertyAssignmentsInfo();

  // Indicate that this function only consists of assignments of the form
  // this.x = y; where y is either a constant or refers to an argument.
  inline bool has_only_simple_this_property_assignments();

  // Indicates if this function can be lazy compiled.
  // This is used to determine if we can safely flush code from a function
  // when doing GC if we expect that the function will no longer be used.
  DECL_BOOLEAN_ACCESSORS(allows_lazy_compilation)

  // Indicates if this function can be lazy compiled without a context.
  // This is used to determine if we can force compilation without reaching
  // the function through program execution but through other means (e.g. heap
  // iteration by the debugger).
  DECL_BOOLEAN_ACCESSORS(allows_lazy_compilation_without_context)

  // Indicates how many full GCs this function has survived with assigned
  // code object. Used to determine when it is relatively safe to flush
  // this code object and replace it with lazy compilation stub.
  // Age is reset when GC notices that the code object is referenced
  // from the stack or compilation cache.
  inline int code_age();
  inline void set_code_age(int age);

  // Indicates whether optimizations have been disabled for this
  // shared function info. If a function is repeatedly optimized or if
  // we cannot optimize the function we disable optimization to avoid
  // spending time attempting to optimize it again.
  DECL_BOOLEAN_ACCESSORS(optimization_disabled)

  // Indicates the language mode of the function's code as defined by the
  // current harmony drafts for the next ES language standard. Possible
  // values are:
  // 1. CLASSIC_MODE - Unrestricted syntax and semantics, same as in ES5.
  // 2. STRICT_MODE - Restricted syntax and semantics, same as in ES5.
  // 3. EXTENDED_MODE - Only available under the harmony flag, not part of ES5.
  inline LanguageMode language_mode();
  inline void set_language_mode(LanguageMode language_mode);

  // Indicates whether the language mode of this function is CLASSIC_MODE.
  inline bool is_classic_mode();

  // Indicates whether the language mode of this function is EXTENDED_MODE.
  inline bool is_extended_mode();

  // False if the function definitely does not allocate an arguments object.
  DECL_BOOLEAN_ACCESSORS(uses_arguments)

  // True if the function has any duplicated parameter names.
  DECL_BOOLEAN_ACCESSORS(has_duplicate_parameters)

  // Indicates whether the function is a native function.
  // These needs special treatment in .call and .apply since
  // null passed as the receiver should not be translated to the
  // global object.
  DECL_BOOLEAN_ACCESSORS(native)

  // Indicates that the function was created by the Function function.
  // Though it's anonymous, toString should treat it as if it had the name
  // "anonymous".  We don't set the name itself so that the system does not
  // see a binding for it.
  DECL_BOOLEAN_ACCESSORS(name_should_print_as_anonymous)

  // Indicates whether the function is a bound function created using
  // the bind function.
  DECL_BOOLEAN_ACCESSORS(bound)

  // Indicates that the function is anonymous (the name field can be set
  // through the API, which does not change this flag).
  DECL_BOOLEAN_ACCESSORS(is_anonymous)

  // Is this a function or top-level/eval code.
  DECL_BOOLEAN_ACCESSORS(is_function)

  // Indicates that the function cannot be optimized.
  DECL_BOOLEAN_ACCESSORS(dont_optimize)

  // Indicates that the function cannot be inlined.
  DECL_BOOLEAN_ACCESSORS(dont_inline)

  // Indicates that code for this function cannot be cached.
  DECL_BOOLEAN_ACCESSORS(dont_cache)

  // Indicates whether or not the code in the shared function support
  // deoptimization.
  inline bool has_deoptimization_support();

  // Enable deoptimization support through recompiled code.
  void EnableDeoptimizationSupport(Code* recompiled);

  // Disable (further) attempted optimization of all functions sharing this
  // shared function info.
  void DisableOptimization(const char* reason);

  // Lookup the bailout ID and ASSERT that it exists in the non-optimized
  // code, returns whether it asserted (i.e., always true if assertions are
  // disabled).
  bool VerifyBailoutId(BailoutId id);

  // Check whether a inlined constructor can be generated with the given
  // prototype.
  bool CanGenerateInlineConstructor(Object* prototype);

  // Prevents further attempts to generate inline constructors.
  // To be called if generation failed for any reason.
  void ForbidInlineConstructor();

  // For functions which only contains this property assignments this provides
  // access to the names for the properties assigned.
  DECL_ACCESSORS(this_property_assignments, Object)
  inline int this_property_assignments_count();
  inline void set_this_property_assignments_count(int value);
  String* GetThisPropertyAssignmentName(int index);
  bool IsThisPropertyAssignmentArgument(int index);
  int GetThisPropertyAssignmentArgument(int index);
  Object* GetThisPropertyAssignmentConstant(int index);

  // [source code]: Source code for the function.
  bool HasSourceCode();
  Handle<Object> GetSourceCode();

  // Number of times the function was optimized.
  inline int opt_count();
  inline void set_opt_count(int opt_count);

  // Number of times the function was deoptimized.
  inline void set_deopt_count(int value);
  inline int deopt_count();
  inline void increment_deopt_count();

  // Number of time we tried to re-enable optimization after it
  // was disabled due to high number of deoptimizations.
  inline void set_opt_reenable_tries(int value);
  inline int opt_reenable_tries();

  inline void TryReenableOptimization();

  // Stores deopt_count, opt_reenable_tries and ic_age as bit-fields.
  inline void set_counters(int value);
  inline int counters();

  // Source size of this function.
  int SourceSize();

  // Calculate the instance size.
  int CalculateInstanceSize();

  // Calculate the number of in-object properties.
  int CalculateInObjectProperties();

  // Dispatched behavior.
  // Set max_length to -1 for unlimited length.
  void SourceCodePrint(StringStream* accumulator, int max_length);
#ifdef OBJECT_PRINT
  inline void SharedFunctionInfoPrint() {
    SharedFunctionInfoPrint(stdout);
  }
  void SharedFunctionInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(SharedFunctionInfo)

  void ResetForNewContext(int new_ic_age);

  // Helper to compile the shared code.  Returns true on success, false on
  // failure (e.g., stack overflow during compilation). This is only used by
  // the debugger, it is not possible to compile without a context otherwise.
  static bool CompileLazy(Handle<SharedFunctionInfo> shared,
                          ClearExceptionFlag flag);

  // Casting.
  static inline SharedFunctionInfo* cast(Object* obj);

  // Constants.
  static const int kDontAdaptArgumentsSentinel = -1;

  // Layout description.
  // Pointer fields.
  static const int kNameOffset = HeapObject::kHeaderSize;
  static const int kCodeOffset = kNameOffset + kPointerSize;
  static const int kOptimizedCodeMapOffset = kCodeOffset + kPointerSize;
  static const int kScopeInfoOffset = kOptimizedCodeMapOffset + kPointerSize;
  static const int kConstructStubOffset = kScopeInfoOffset + kPointerSize;
  static const int kInstanceClassNameOffset =
      kConstructStubOffset + kPointerSize;
  static const int kFunctionDataOffset =
      kInstanceClassNameOffset + kPointerSize;
  static const int kScriptOffset = kFunctionDataOffset + kPointerSize;
  static const int kDebugInfoOffset = kScriptOffset + kPointerSize;
  static const int kInferredNameOffset = kDebugInfoOffset + kPointerSize;
  static const int kInitialMapOffset =
      kInferredNameOffset + kPointerSize;
  static const int kThisPropertyAssignmentsOffset =
      kInitialMapOffset + kPointerSize;
  // ast_node_count is a Smi field. It could be grouped with another Smi field
  // into a PSEUDO_SMI_ACCESSORS pair (on x64), if one becomes available.
  static const int kAstNodeCountOffset =
      kThisPropertyAssignmentsOffset + kPointerSize;
#if V8_HOST_ARCH_32_BIT
  // Smi fields.
  static const int kLengthOffset =
      kAstNodeCountOffset + kPointerSize;
  static const int kFormalParameterCountOffset = kLengthOffset + kPointerSize;
  static const int kExpectedNofPropertiesOffset =
      kFormalParameterCountOffset + kPointerSize;
  static const int kNumLiteralsOffset =
      kExpectedNofPropertiesOffset + kPointerSize;
  static const int kStartPositionAndTypeOffset =
      kNumLiteralsOffset + kPointerSize;
  static const int kEndPositionOffset =
      kStartPositionAndTypeOffset + kPointerSize;
  static const int kFunctionTokenPositionOffset =
      kEndPositionOffset + kPointerSize;
  static const int kCompilerHintsOffset =
      kFunctionTokenPositionOffset + kPointerSize;
  static const int kThisPropertyAssignmentsCountOffset =
      kCompilerHintsOffset + kPointerSize;
  static const int kOptCountOffset =
      kThisPropertyAssignmentsCountOffset + kPointerSize;
  static const int kCountersOffset = kOptCountOffset + kPointerSize;
  static const int kStressDeoptCounterOffset = kCountersOffset + kPointerSize;

  // Total size.
  static const int kSize = kStressDeoptCounterOffset + kPointerSize;
#else
  // The only reason to use smi fields instead of int fields
  // is to allow iteration without maps decoding during
  // garbage collections.
  // To avoid wasting space on 64-bit architectures we use
  // the following trick: we group integer fields into pairs
  // First integer in each pair is shifted left by 1.
  // By doing this we guarantee that LSB of each kPointerSize aligned
  // word is not set and thus this word cannot be treated as pointer
  // to HeapObject during old space traversal.
  static const int kLengthOffset =
      kAstNodeCountOffset + kPointerSize;
  static const int kFormalParameterCountOffset =
      kLengthOffset + kIntSize;

  static const int kExpectedNofPropertiesOffset =
      kFormalParameterCountOffset + kIntSize;
  static const int kNumLiteralsOffset =
      kExpectedNofPropertiesOffset + kIntSize;

  static const int kEndPositionOffset =
      kNumLiteralsOffset + kIntSize;
  static const int kStartPositionAndTypeOffset =
      kEndPositionOffset + kIntSize;

  static const int kFunctionTokenPositionOffset =
      kStartPositionAndTypeOffset + kIntSize;
  static const int kCompilerHintsOffset =
      kFunctionTokenPositionOffset + kIntSize;

  static const int kThisPropertyAssignmentsCountOffset =
      kCompilerHintsOffset + kIntSize;
  static const int kOptCountOffset =
      kThisPropertyAssignmentsCountOffset + kIntSize;

  static const int kCountersOffset = kOptCountOffset + kIntSize;
  static const int kStressDeoptCounterOffset = kCountersOffset + kIntSize;

  // Total size.
  static const int kSize = kStressDeoptCounterOffset + kIntSize;

#endif

  // The construction counter for inobject slack tracking is stored in the
  // most significant byte of compiler_hints which is otherwise unused.
  // Its offset depends on the endian-ness of the architecture.
#if __BYTE_ORDER == __LITTLE_ENDIAN
  static const int kConstructionCountOffset = kCompilerHintsOffset + 3;
#elif __BYTE_ORDER == __BIG_ENDIAN
  static const int kConstructionCountOffset = kCompilerHintsOffset + 0;
#else
#error Unknown byte ordering
#endif

  static const int kAlignedSize = POINTER_SIZE_ALIGN(kSize);

  typedef FixedBodyDescriptor<kNameOffset,
                              kThisPropertyAssignmentsOffset + kPointerSize,
                              kSize> BodyDescriptor;

  // Bit positions in start_position_and_type.
  // The source code start position is in the 30 most significant bits of
  // the start_position_and_type field.
  static const int kIsExpressionBit = 0;
  static const int kIsTopLevelBit   = 1;
  static const int kStartPositionShift = 2;
  static const int kStartPositionMask = ~((1 << kStartPositionShift) - 1);

  // Bit positions in compiler_hints.
  static const int kCodeAgeSize = 3;
  static const int kCodeAgeMask = (1 << kCodeAgeSize) - 1;

  enum CompilerHints {
    kHasOnlySimpleThisPropertyAssignments,
    kAllowLazyCompilation,
    kAllowLazyCompilationWithoutContext,
    kLiveObjectsMayExist,
    kCodeAgeShift,
    kOptimizationDisabled = kCodeAgeShift + kCodeAgeSize,
    kStrictModeFunction,
    kExtendedModeFunction,
    kUsesArguments,
    kHasDuplicateParameters,
    kNative,
    kBoundFunction,
    kIsAnonymous,
    kNameShouldPrintAsAnonymous,
    kIsFunction,
    kDontOptimize,
    kDontInline,
    kDontCache,
    kCompilerHintsCount  // Pseudo entry
  };

  class DeoptCountBits: public BitField<int, 0, 4> {};
  class OptReenableTriesBits: public BitField<int, 4, 18> {};
  class ICAgeBits: public BitField<int, 22, 8> {};

 private:
#if V8_HOST_ARCH_32_BIT
  // On 32 bit platforms, compiler hints is a smi.
  static const int kCompilerHintsSmiTagSize = kSmiTagSize;
  static const int kCompilerHintsSize = kPointerSize;
#else
  // On 64 bit platforms, compiler hints is not a smi, see comment above.
  static const int kCompilerHintsSmiTagSize = 0;
  static const int kCompilerHintsSize = kIntSize;
#endif

  STATIC_ASSERT(SharedFunctionInfo::kCompilerHintsCount <=
                SharedFunctionInfo::kCompilerHintsSize * kBitsPerByte);

 public:
  // Constants for optimizing codegen for strict mode function and
  // native tests.
  // Allows to use byte-width instructions.
  static const int kStrictModeBitWithinByte =
      (kStrictModeFunction + kCompilerHintsSmiTagSize) % kBitsPerByte;

  static const int kExtendedModeBitWithinByte =
      (kExtendedModeFunction + kCompilerHintsSmiTagSize) % kBitsPerByte;

  static const int kNativeBitWithinByte =
      (kNative + kCompilerHintsSmiTagSize) % kBitsPerByte;

#if __BYTE_ORDER == __LITTLE_ENDIAN
  static const int kStrictModeByteOffset = kCompilerHintsOffset +
      (kStrictModeFunction + kCompilerHintsSmiTagSize) / kBitsPerByte;
  static const int kExtendedModeByteOffset = kCompilerHintsOffset +
      (kExtendedModeFunction + kCompilerHintsSmiTagSize) / kBitsPerByte;
  static const int kNativeByteOffset = kCompilerHintsOffset +
      (kNative + kCompilerHintsSmiTagSize) / kBitsPerByte;
#elif __BYTE_ORDER == __BIG_ENDIAN
  static const int kStrictModeByteOffset = kCompilerHintsOffset +
      (kCompilerHintsSize - 1) -
      ((kStrictModeFunction + kCompilerHintsSmiTagSize) / kBitsPerByte);
  static const int kExtendedModeByteOffset = kCompilerHintsOffset +
      (kCompilerHintsSize - 1) -
      ((kExtendedModeFunction + kCompilerHintsSmiTagSize) / kBitsPerByte);
  static const int kNativeByteOffset = kCompilerHintsOffset +
      (kCompilerHintsSize - 1) -
      ((kNative + kCompilerHintsSmiTagSize) / kBitsPerByte);
#else
#error Unknown byte ordering
#endif

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SharedFunctionInfo);
};


// Representation for module instance objects.
class JSModule: public JSObject {
 public:
  // [context]: the context holding the module's locals, or undefined if none.
  DECL_ACCESSORS(context, Object)

  // [scope_info]: Scope info.
  DECL_ACCESSORS(scope_info, ScopeInfo)

  // Casting.
  static inline JSModule* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSModulePrint() {
    JSModulePrint(stdout);
  }
  void JSModulePrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSModule)

  // Layout description.
  static const int kContextOffset = JSObject::kHeaderSize;
  static const int kScopeInfoOffset = kContextOffset + kPointerSize;
  static const int kSize = kScopeInfoOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSModule);
};


// JSFunction describes JavaScript functions.
class JSFunction: public JSObject {
 public:
  // [prototype_or_initial_map]:
  DECL_ACCESSORS(prototype_or_initial_map, Object)

  // [shared]: The information about the function that
  // can be shared by instances.
  DECL_ACCESSORS(shared, SharedFunctionInfo)

  inline SharedFunctionInfo* unchecked_shared();

  // [context]: The context for this function.
  inline Context* context();
  inline Object* unchecked_context();
  inline void set_context(Object* context);

  // [code]: The generated code object for this function.  Executed
  // when the function is invoked, e.g. foo() or new foo(). See
  // [[Call]] and [[Construct]] description in ECMA-262, section
  // 8.6.2, page 27.
  inline Code* code();
  inline void set_code(Code* code);
  inline void ReplaceCode(Code* code);

  inline Code* unchecked_code();

  // Tells whether this function is builtin.
  inline bool IsBuiltin();

  // Tells whether or not the function needs arguments adaption.
  inline bool NeedsArgumentsAdaption();

  // Tells whether or not this function has been optimized.
  inline bool IsOptimized();

  // Tells whether or not this function can be optimized.
  inline bool IsOptimizable();

  // Mark this function for lazy recompilation. The function will be
  // recompiled the next time it is executed.
  void MarkForLazyRecompilation();
  void MarkForParallelRecompilation();

  // Helpers to compile this function.  Returns true on success, false on
  // failure (e.g., stack overflow during compilation).
  static bool EnsureCompiled(Handle<JSFunction> function,
                             ClearExceptionFlag flag);
  static bool CompileLazy(Handle<JSFunction> function,
                          ClearExceptionFlag flag);
  static bool CompileOptimized(Handle<JSFunction> function,
                               BailoutId osr_ast_id,
                               ClearExceptionFlag flag);

  // Tells whether or not the function is already marked for lazy
  // recompilation.
  inline bool IsMarkedForLazyRecompilation();
  inline bool IsMarkedForParallelRecompilation();

  // Tells whether or not the function is on the parallel
  // recompilation queue.
  inline bool IsInRecompileQueue();

  // Check whether or not this function is inlineable.
  bool IsInlineable();

  // [literals_or_bindings]: Fixed array holding either
  // the materialized literals or the bindings of a bound function.
  //
  // If the function contains object, regexp or array literals, the
  // literals array prefix contains the object, regexp, and array
  // function to be used when creating these literals.  This is
  // necessary so that we do not dynamically lookup the object, regexp
  // or array functions.  Performing a dynamic lookup, we might end up
  // using the functions from a new context that we should not have
  // access to.
  //
  // On bound functions, the array is a (copy-on-write) fixed-array containing
  // the function that was bound, bound this-value and any bound
  // arguments. Bound functions never contain literals.
  DECL_ACCESSORS(literals_or_bindings, FixedArray)

  inline FixedArray* literals();
  inline void set_literals(FixedArray* literals);

  inline FixedArray* function_bindings();
  inline void set_function_bindings(FixedArray* bindings);

  // The initial map for an object created by this constructor.
  inline Map* initial_map();
  inline void set_initial_map(Map* value);
  inline bool has_initial_map();

  // Get and set the prototype property on a JSFunction. If the
  // function has an initial map the prototype is set on the initial
  // map. Otherwise, the prototype is put in the initial map field
  // until an initial map is needed.
  inline bool has_prototype();
  inline bool has_instance_prototype();
  inline Object* prototype();
  inline Object* instance_prototype();
  MUST_USE_RESULT MaybeObject* SetInstancePrototype(Object* value);
  MUST_USE_RESULT MaybeObject* SetPrototype(Object* value);

  // After prototype is removed, it will not be created when accessed, and
  // [[Construct]] from this function will not be allowed.
  void RemovePrototype();
  inline bool should_have_prototype();

  // Accessor for this function's initial map's [[class]]
  // property. This is primarily used by ECMA native functions.  This
  // method sets the class_name field of this function's initial map
  // to a given value. It creates an initial map if this function does
  // not have one. Note that this method does not copy the initial map
  // if it has one already, but simply replaces it with the new value.
  // Instances created afterwards will have a map whose [[class]] is
  // set to 'value', but there is no guarantees on instances created
  // before.
  void SetInstanceClassName(String* name);

  // Returns if this function has been compiled to native code yet.
  inline bool is_compiled();

  // [next_function_link]: Field for linking functions. This list is treated as
  // a weak list by the GC.
  DECL_ACCESSORS(next_function_link, Object)

  // Prints the name of the function using PrintF.
  inline void PrintName() {
    PrintName(stdout);
  }
  void PrintName(FILE* out);

  // Casting.
  static inline JSFunction* cast(Object* obj);

  // Iterates the objects, including code objects indirectly referenced
  // through pointers to the first instruction in the code object.
  void JSFunctionIterateBody(int object_size, ObjectVisitor* v);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSFunctionPrint() {
    JSFunctionPrint(stdout);
  }
  void JSFunctionPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSFunction)

  // Returns the number of allocated literals.
  inline int NumberOfLiterals();

  // Retrieve the native context from a function's literal array.
  static Context* NativeContextFromLiterals(FixedArray* literals);

  // Layout descriptors. The last property (from kNonWeakFieldsEndOffset to
  // kSize) is weak and has special handling during garbage collection.
  static const int kCodeEntryOffset = JSObject::kHeaderSize;
  static const int kPrototypeOrInitialMapOffset =
      kCodeEntryOffset + kPointerSize;
  static const int kSharedFunctionInfoOffset =
      kPrototypeOrInitialMapOffset + kPointerSize;
  static const int kContextOffset = kSharedFunctionInfoOffset + kPointerSize;
  static const int kLiteralsOffset = kContextOffset + kPointerSize;
  static const int kNonWeakFieldsEndOffset = kLiteralsOffset + kPointerSize;
  static const int kNextFunctionLinkOffset = kNonWeakFieldsEndOffset;
  static const int kSize = kNextFunctionLinkOffset + kPointerSize;

  // Layout of the literals array.
  static const int kLiteralsPrefixSize = 1;
  static const int kLiteralNativeContextIndex = 0;

  // Layout of the bound-function binding array.
  static const int kBoundFunctionIndex = 0;
  static const int kBoundThisIndex = 1;
  static const int kBoundArgumentsStartIndex = 2;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSFunction);
};


// JSGlobalProxy's prototype must be a JSGlobalObject or null,
// and the prototype is hidden. JSGlobalProxy always delegates
// property accesses to its prototype if the prototype is not null.
//
// A JSGlobalProxy can be reinitialized which will preserve its identity.
//
// Accessing a JSGlobalProxy requires security check.

class JSGlobalProxy : public JSObject {
 public:
  // [native_context]: the owner native context of this global proxy object.
  // It is null value if this object is not used by any context.
  DECL_ACCESSORS(native_context, Object)

  // Casting.
  static inline JSGlobalProxy* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSGlobalProxyPrint() {
    JSGlobalProxyPrint(stdout);
  }
  void JSGlobalProxyPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSGlobalProxy)

  // Layout description.
  static const int kNativeContextOffset = JSObject::kHeaderSize;
  static const int kSize = kNativeContextOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSGlobalProxy);
};


// Forward declaration.
class JSBuiltinsObject;

// Common super class for JavaScript global objects and the special
// builtins global objects.
class GlobalObject: public JSObject {
 public:
  // [builtins]: the object holding the runtime routines written in JS.
  DECL_ACCESSORS(builtins, JSBuiltinsObject)

  // [native context]: the natives corresponding to this global object.
  DECL_ACCESSORS(native_context, Context)

  // [global context]: the most recent (i.e. innermost) global context.
  DECL_ACCESSORS(global_context, Context)

  // [global receiver]: the global receiver object of the context
  DECL_ACCESSORS(global_receiver, JSObject)

  // Retrieve the property cell used to store a property.
  JSGlobalPropertyCell* GetPropertyCell(LookupResult* result);

  // This is like GetProperty, but is used when you know the lookup won't fail
  // by throwing an exception.  This is for the debug and builtins global
  // objects, where it is known which properties can be expected to be present
  // on the object.
  Object* GetPropertyNoExceptionThrown(String* key) {
    Object* answer = GetProperty(key)->ToObjectUnchecked();
    return answer;
  }

  // Ensure that the global object has a cell for the given property name.
  static Handle<JSGlobalPropertyCell> EnsurePropertyCell(
      Handle<GlobalObject> global,
      Handle<String> name);
  // TODO(kmillikin): This function can be eliminated once the stub cache is
  // full handlified (and the static helper can be written directly).
  MUST_USE_RESULT MaybeObject* EnsurePropertyCell(String* name);

  // Casting.
  static inline GlobalObject* cast(Object* obj);

  // Layout description.
  static const int kBuiltinsOffset = JSObject::kHeaderSize;
  static const int kNativeContextOffset = kBuiltinsOffset + kPointerSize;
  static const int kGlobalContextOffset = kNativeContextOffset + kPointerSize;
  static const int kGlobalReceiverOffset = kGlobalContextOffset + kPointerSize;
  static const int kHeaderSize = kGlobalReceiverOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(GlobalObject);
};


// JavaScript global object.
class JSGlobalObject: public GlobalObject {
 public:
  // Casting.
  static inline JSGlobalObject* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSGlobalObjectPrint() {
    JSGlobalObjectPrint(stdout);
  }
  void JSGlobalObjectPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSGlobalObject)

  // Layout description.
  static const int kSize = GlobalObject::kHeaderSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSGlobalObject);
};


// Builtins global object which holds the runtime routines written in
// JavaScript.
class JSBuiltinsObject: public GlobalObject {
 public:
  // Accessors for the runtime routines written in JavaScript.
  inline Object* javascript_builtin(Builtins::JavaScript id);
  inline void set_javascript_builtin(Builtins::JavaScript id, Object* value);

  // Accessors for code of the runtime routines written in JavaScript.
  inline Code* javascript_builtin_code(Builtins::JavaScript id);
  inline void set_javascript_builtin_code(Builtins::JavaScript id, Code* value);

  // Casting.
  static inline JSBuiltinsObject* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSBuiltinsObjectPrint() {
    JSBuiltinsObjectPrint(stdout);
  }
  void JSBuiltinsObjectPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSBuiltinsObject)

  // Layout description.  The size of the builtins object includes
  // room for two pointers per runtime routine written in javascript
  // (function and code object).
  static const int kJSBuiltinsCount = Builtins::id_count;
  static const int kJSBuiltinsOffset = GlobalObject::kHeaderSize;
  static const int kJSBuiltinsCodeOffset =
      GlobalObject::kHeaderSize + (kJSBuiltinsCount * kPointerSize);
  static const int kSize =
      kJSBuiltinsCodeOffset + (kJSBuiltinsCount * kPointerSize);

  static int OffsetOfFunctionWithId(Builtins::JavaScript id) {
    return kJSBuiltinsOffset + id * kPointerSize;
  }

  static int OffsetOfCodeWithId(Builtins::JavaScript id) {
    return kJSBuiltinsCodeOffset + id * kPointerSize;
  }

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSBuiltinsObject);
};


// Representation for JS Wrapper objects, String, Number, Boolean, etc.
class JSValue: public JSObject {
 public:
  // [value]: the object being wrapped.
  DECL_ACCESSORS(value, Object)

  // Casting.
  static inline JSValue* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSValuePrint() {
    JSValuePrint(stdout);
  }
  void JSValuePrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSValue)

  // Layout description.
  static const int kValueOffset = JSObject::kHeaderSize;
  static const int kSize = kValueOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSValue);
};


class DateCache;

// Representation for JS date objects.
class JSDate: public JSObject {
 public:
  // If one component is NaN, all of them are, indicating a NaN time value.
  // [value]: the time value.
  DECL_ACCESSORS(value, Object)
  // [year]: caches year. Either undefined, smi, or NaN.
  DECL_ACCESSORS(year, Object)
  // [month]: caches month. Either undefined, smi, or NaN.
  DECL_ACCESSORS(month, Object)
  // [day]: caches day. Either undefined, smi, or NaN.
  DECL_ACCESSORS(day, Object)
  // [weekday]: caches day of week. Either undefined, smi, or NaN.
  DECL_ACCESSORS(weekday, Object)
  // [hour]: caches hours. Either undefined, smi, or NaN.
  DECL_ACCESSORS(hour, Object)
  // [min]: caches minutes. Either undefined, smi, or NaN.
  DECL_ACCESSORS(min, Object)
  // [sec]: caches seconds. Either undefined, smi, or NaN.
  DECL_ACCESSORS(sec, Object)
  // [cache stamp]: sample of the date cache stamp at the
  // moment when local fields were cached.
  DECL_ACCESSORS(cache_stamp, Object)

  // Casting.
  static inline JSDate* cast(Object* obj);

  // Returns the date field with the specified index.
  // See FieldIndex for the list of date fields.
  static Object* GetField(Object* date, Smi* index);

  void SetValue(Object* value, bool is_value_nan);


  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSDatePrint() {
    JSDatePrint(stdout);
  }
  void JSDatePrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSDate)

  // The order is important. It must be kept in sync with date macros
  // in macros.py.
  enum FieldIndex {
    kDateValue,
    kYear,
    kMonth,
    kDay,
    kWeekday,
    kHour,
    kMinute,
    kSecond,
    kFirstUncachedField,
    kMillisecond = kFirstUncachedField,
    kDays,
    kTimeInDay,
    kFirstUTCField,
    kYearUTC = kFirstUTCField,
    kMonthUTC,
    kDayUTC,
    kWeekdayUTC,
    kHourUTC,
    kMinuteUTC,
    kSecondUTC,
    kMillisecondUTC,
    kDaysUTC,
    kTimeInDayUTC,
    kTimezoneOffset
  };

  // Layout description.
  static const int kValueOffset = JSObject::kHeaderSize;
  static const int kYearOffset = kValueOffset + kPointerSize;
  static const int kMonthOffset = kYearOffset + kPointerSize;
  static const int kDayOffset = kMonthOffset + kPointerSize;
  static const int kWeekdayOffset = kDayOffset + kPointerSize;
  static const int kHourOffset = kWeekdayOffset  + kPointerSize;
  static const int kMinOffset = kHourOffset + kPointerSize;
  static const int kSecOffset = kMinOffset + kPointerSize;
  static const int kCacheStampOffset = kSecOffset + kPointerSize;
  static const int kSize = kCacheStampOffset + kPointerSize;

 private:
  inline Object* DoGetField(FieldIndex index);

  Object* GetUTCField(FieldIndex index, double value, DateCache* date_cache);

  // Computes and caches the cacheable fields of the date.
  inline void SetLocalFields(int64_t local_time_ms, DateCache* date_cache);


  DISALLOW_IMPLICIT_CONSTRUCTORS(JSDate);
};


// Representation of message objects used for error reporting through
// the API. The messages are formatted in JavaScript so this object is
// a real JavaScript object. The information used for formatting the
// error messages are not directly accessible from JavaScript to
// prevent leaking information to user code called during error
// formatting.
class JSMessageObject: public JSObject {
 public:
  // [type]: the type of error message.
  DECL_ACCESSORS(type, String)

  // [arguments]: the arguments for formatting the error message.
  DECL_ACCESSORS(arguments, JSArray)

  // [script]: the script from which the error message originated.
  DECL_ACCESSORS(script, Object)

  // [stack_trace]: the stack trace for this error message.
  DECL_ACCESSORS(stack_trace, Object)

  // [stack_frames]: an array of stack frames for this error object.
  DECL_ACCESSORS(stack_frames, Object)

  // [start_position]: the start position in the script for the error message.
  inline int start_position();
  inline void set_start_position(int value);

  // [end_position]: the end position in the script for the error message.
  inline int end_position();
  inline void set_end_position(int value);

  // Casting.
  static inline JSMessageObject* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSMessageObjectPrint() {
    JSMessageObjectPrint(stdout);
  }
  void JSMessageObjectPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSMessageObject)

  // Layout description.
  static const int kTypeOffset = JSObject::kHeaderSize;
  static const int kArgumentsOffset = kTypeOffset + kPointerSize;
  static const int kScriptOffset = kArgumentsOffset + kPointerSize;
  static const int kStackTraceOffset = kScriptOffset + kPointerSize;
  static const int kStackFramesOffset = kStackTraceOffset + kPointerSize;
  static const int kStartPositionOffset = kStackFramesOffset + kPointerSize;
  static const int kEndPositionOffset = kStartPositionOffset + kPointerSize;
  static const int kSize = kEndPositionOffset + kPointerSize;

  typedef FixedBodyDescriptor<HeapObject::kMapOffset,
                              kStackFramesOffset + kPointerSize,
                              kSize> BodyDescriptor;
};


// Regular expressions
// The regular expression holds a single reference to a FixedArray in
// the kDataOffset field.
// The FixedArray contains the following data:
// - tag : type of regexp implementation (not compiled yet, atom or irregexp)
// - reference to the original source string
// - reference to the original flag string
// If it is an atom regexp
// - a reference to a literal string to search for
// If it is an irregexp regexp:
// - a reference to code for ASCII inputs (bytecode or compiled), or a smi
// used for tracking the last usage (used for code flushing).
// - a reference to code for UC16 inputs (bytecode or compiled), or a smi
// used for tracking the last usage (used for code flushing)..
// - max number of registers used by irregexp implementations.
// - number of capture registers (output values) of the regexp.
class JSRegExp: public JSObject {
 public:
  // Meaning of Type:
  // NOT_COMPILED: Initial value. No data has been stored in the JSRegExp yet.
  // ATOM: A simple string to match against using an indexOf operation.
  // IRREGEXP: Compiled with Irregexp.
  // IRREGEXP_NATIVE: Compiled to native code with Irregexp.
  enum Type { NOT_COMPILED, ATOM, IRREGEXP };
  enum Flag { NONE = 0, GLOBAL = 1, IGNORE_CASE = 2, MULTILINE = 4 };

  class Flags {
   public:
    explicit Flags(uint32_t value) : value_(value) { }
    bool is_global() { return (value_ & GLOBAL) != 0; }
    bool is_ignore_case() { return (value_ & IGNORE_CASE) != 0; }
    bool is_multiline() { return (value_ & MULTILINE) != 0; }
    uint32_t value() { return value_; }
   private:
    uint32_t value_;
  };

  DECL_ACCESSORS(data, Object)

  inline Type TypeTag();
  inline int CaptureCount();
  inline Flags GetFlags();
  inline String* Pattern();
  inline Object* DataAt(int index);
  // Set implementation data after the object has been prepared.
  inline void SetDataAt(int index, Object* value);

  // Used during GC when flushing code or setting age.
  inline Object* DataAtUnchecked(int index);
  inline void SetDataAtUnchecked(int index, Object* value, Heap* heap);
  inline Type TypeTagUnchecked();

  static int code_index(bool is_ascii) {
    if (is_ascii) {
      return kIrregexpASCIICodeIndex;
    } else {
      return kIrregexpUC16CodeIndex;
    }
  }

  static int saved_code_index(bool is_ascii) {
    if (is_ascii) {
      return kIrregexpASCIICodeSavedIndex;
    } else {
      return kIrregexpUC16CodeSavedIndex;
    }
  }

  static inline JSRegExp* cast(Object* obj);

  // Dispatched behavior.
  DECLARE_VERIFIER(JSRegExp)

  static const int kDataOffset = JSObject::kHeaderSize;
  static const int kSize = kDataOffset + kPointerSize;

  // Indices in the data array.
  static const int kTagIndex = 0;
  static const int kSourceIndex = kTagIndex + 1;
  static const int kFlagsIndex = kSourceIndex + 1;
  static const int kDataIndex = kFlagsIndex + 1;
  // The data fields are used in different ways depending on the
  // value of the tag.
  // Atom regexps (literal strings).
  static const int kAtomPatternIndex = kDataIndex;

  static const int kAtomDataSize = kAtomPatternIndex + 1;

  // Irregexp compiled code or bytecode for ASCII. If compilation
  // fails, this fields hold an exception object that should be
  // thrown if the regexp is used again.
  static const int kIrregexpASCIICodeIndex = kDataIndex;
  // Irregexp compiled code or bytecode for UC16.  If compilation
  // fails, this fields hold an exception object that should be
  // thrown if the regexp is used again.
  static const int kIrregexpUC16CodeIndex = kDataIndex + 1;

  // Saved instance of Irregexp compiled code or bytecode for ASCII that
  // is a potential candidate for flushing.
  static const int kIrregexpASCIICodeSavedIndex = kDataIndex + 2;
  // Saved instance of Irregexp compiled code or bytecode for UC16 that is
  // a potential candidate for flushing.
  static const int kIrregexpUC16CodeSavedIndex = kDataIndex + 3;

  // Maximal number of registers used by either ASCII or UC16.
  // Only used to check that there is enough stack space
  static const int kIrregexpMaxRegisterCountIndex = kDataIndex + 4;
  // Number of captures in the compiled regexp.
  static const int kIrregexpCaptureCountIndex = kDataIndex + 5;

  static const int kIrregexpDataSize = kIrregexpCaptureCountIndex + 1;

  // Offsets directly into the data fixed array.
  static const int kDataTagOffset =
      FixedArray::kHeaderSize + kTagIndex * kPointerSize;
  static const int kDataAsciiCodeOffset =
      FixedArray::kHeaderSize + kIrregexpASCIICodeIndex * kPointerSize;
  static const int kDataUC16CodeOffset =
      FixedArray::kHeaderSize + kIrregexpUC16CodeIndex * kPointerSize;
  static const int kIrregexpCaptureCountOffset =
      FixedArray::kHeaderSize + kIrregexpCaptureCountIndex * kPointerSize;

  // In-object fields.
  static const int kSourceFieldIndex = 0;
  static const int kGlobalFieldIndex = 1;
  static const int kIgnoreCaseFieldIndex = 2;
  static const int kMultilineFieldIndex = 3;
  static const int kLastIndexFieldIndex = 4;
  static const int kInObjectFieldCount = 5;

  // The uninitialized value for a regexp code object.
  static const int kUninitializedValue = -1;

  // The compilation error value for the regexp code object. The real error
  // object is in the saved code field.
  static const int kCompilationErrorValue = -2;

  // When we store the sweep generation at which we moved the code from the
  // code index to the saved code index we mask it of to be in the [0:255]
  // range.
  static const int kCodeAgeMask = 0xff;
};


class CompilationCacheShape : public BaseShape<HashTableKey*> {
 public:
  static inline bool IsMatch(HashTableKey* key, Object* value) {
    return key->IsMatch(value);
  }

  static inline uint32_t Hash(HashTableKey* key) {
    return key->Hash();
  }

  static inline uint32_t HashForObject(HashTableKey* key, Object* object) {
    return key->HashForObject(object);
  }

  MUST_USE_RESULT static MaybeObject* AsObject(HashTableKey* key) {
    return key->AsObject();
  }

  static const int kPrefixSize = 0;
  static const int kEntrySize = 2;
};


class CompilationCacheTable: public HashTable<CompilationCacheShape,
                                              HashTableKey*> {
 public:
  // Find cached value for a string key, otherwise return null.
  Object* Lookup(String* src, Context* context);
  Object* LookupEval(String* src,
                     Context* context,
                     LanguageMode language_mode,
                     int scope_position);
  Object* LookupRegExp(String* source, JSRegExp::Flags flags);
  MUST_USE_RESULT MaybeObject* Put(String* src,
                                   Context* context,
                                   Object* value);
  MUST_USE_RESULT MaybeObject* PutEval(String* src,
                                       Context* context,
                                       SharedFunctionInfo* value,
                                       int scope_position);
  MUST_USE_RESULT MaybeObject* PutRegExp(String* src,
                                         JSRegExp::Flags flags,
                                         FixedArray* value);

  // Remove given value from cache.
  void Remove(Object* value);

  static inline CompilationCacheTable* cast(Object* obj);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CompilationCacheTable);
};


class CodeCache: public Struct {
 public:
  DECL_ACCESSORS(default_cache, FixedArray)
  DECL_ACCESSORS(normal_type_cache, Object)

  // Add the code object to the cache.
  MUST_USE_RESULT MaybeObject* Update(String* name, Code* code);

  // Lookup code object in the cache. Returns code object if found and undefined
  // if not.
  Object* Lookup(String* name, Code::Flags flags);

  // Get the internal index of a code object in the cache. Returns -1 if the
  // code object is not in that cache. This index can be used to later call
  // RemoveByIndex. The cache cannot be modified between a call to GetIndex and
  // RemoveByIndex.
  int GetIndex(Object* name, Code* code);

  // Remove an object from the cache with the provided internal index.
  void RemoveByIndex(Object* name, Code* code, int index);

  static inline CodeCache* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void CodeCachePrint() {
    CodeCachePrint(stdout);
  }
  void CodeCachePrint(FILE* out);
#endif
  DECLARE_VERIFIER(CodeCache)

  static const int kDefaultCacheOffset = HeapObject::kHeaderSize;
  static const int kNormalTypeCacheOffset =
      kDefaultCacheOffset + kPointerSize;
  static const int kSize = kNormalTypeCacheOffset + kPointerSize;

 private:
  MUST_USE_RESULT MaybeObject* UpdateDefaultCache(String* name, Code* code);
  MUST_USE_RESULT MaybeObject* UpdateNormalTypeCache(String* name, Code* code);
  Object* LookupDefaultCache(String* name, Code::Flags flags);
  Object* LookupNormalTypeCache(String* name, Code::Flags flags);

  // Code cache layout of the default cache. Elements are alternating name and
  // code objects for non normal load/store/call IC's.
  static const int kCodeCacheEntrySize = 2;
  static const int kCodeCacheEntryNameOffset = 0;
  static const int kCodeCacheEntryCodeOffset = 1;

  DISALLOW_IMPLICIT_CONSTRUCTORS(CodeCache);
};


class CodeCacheHashTableShape : public BaseShape<HashTableKey*> {
 public:
  static inline bool IsMatch(HashTableKey* key, Object* value) {
    return key->IsMatch(value);
  }

  static inline uint32_t Hash(HashTableKey* key) {
    return key->Hash();
  }

  static inline uint32_t HashForObject(HashTableKey* key, Object* object) {
    return key->HashForObject(object);
  }

  MUST_USE_RESULT static MaybeObject* AsObject(HashTableKey* key) {
    return key->AsObject();
  }

  static const int kPrefixSize = 0;
  static const int kEntrySize = 2;
};


class CodeCacheHashTable: public HashTable<CodeCacheHashTableShape,
                                           HashTableKey*> {
 public:
  Object* Lookup(String* name, Code::Flags flags);
  MUST_USE_RESULT MaybeObject* Put(String* name, Code* code);

  int GetIndex(String* name, Code::Flags flags);
  void RemoveByIndex(int index);

  static inline CodeCacheHashTable* cast(Object* obj);

  // Initial size of the fixed array backing the hash table.
  static const int kInitialSize = 64;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CodeCacheHashTable);
};


class PolymorphicCodeCache: public Struct {
 public:
  DECL_ACCESSORS(cache, Object)

  static void Update(Handle<PolymorphicCodeCache> cache,
                     MapHandleList* maps,
                     Code::Flags flags,
                     Handle<Code> code);

  MUST_USE_RESULT MaybeObject* Update(MapHandleList* maps,
                                      Code::Flags flags,
                                      Code* code);

  // Returns an undefined value if the entry is not found.
  Handle<Object> Lookup(MapHandleList* maps, Code::Flags flags);

  static inline PolymorphicCodeCache* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void PolymorphicCodeCachePrint() {
    PolymorphicCodeCachePrint(stdout);
  }
  void PolymorphicCodeCachePrint(FILE* out);
#endif
  DECLARE_VERIFIER(PolymorphicCodeCache)

  static const int kCacheOffset = HeapObject::kHeaderSize;
  static const int kSize = kCacheOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(PolymorphicCodeCache);
};


class PolymorphicCodeCacheHashTable
    : public HashTable<CodeCacheHashTableShape, HashTableKey*> {
 public:
  Object* Lookup(MapHandleList* maps, int code_kind);

  MUST_USE_RESULT MaybeObject* Put(MapHandleList* maps,
                                   int code_kind,
                                   Code* code);

  static inline PolymorphicCodeCacheHashTable* cast(Object* obj);

  static const int kInitialSize = 64;
 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(PolymorphicCodeCacheHashTable);
};


class TypeFeedbackInfo: public Struct {
 public:
  inline int ic_total_count();
  inline void set_ic_total_count(int count);

  inline int ic_with_type_info_count();
  inline void change_ic_with_type_info_count(int count);

  inline void initialize_storage();

  inline void change_own_type_change_checksum();
  inline int own_type_change_checksum();

  inline void set_inlined_type_change_checksum(int checksum);
  inline bool matches_inlined_type_change_checksum(int checksum);

  DECL_ACCESSORS(type_feedback_cells, TypeFeedbackCells)

  static inline TypeFeedbackInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void TypeFeedbackInfoPrint() {
    TypeFeedbackInfoPrint(stdout);
  }
  void TypeFeedbackInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(TypeFeedbackInfo)

  static const int kStorage1Offset = HeapObject::kHeaderSize;
  static const int kStorage2Offset = kStorage1Offset + kPointerSize;
  static const int kTypeFeedbackCellsOffset = kStorage2Offset + kPointerSize;
  static const int kSize = kTypeFeedbackCellsOffset + kPointerSize;

 private:
  static const int kTypeChangeChecksumBits = 7;

  class ICTotalCountField: public BitField<int, 0,
      kSmiValueSize - kTypeChangeChecksumBits> {};  // NOLINT
  class OwnTypeChangeChecksum: public BitField<int,
      kSmiValueSize - kTypeChangeChecksumBits,
      kTypeChangeChecksumBits> {};  // NOLINT
  class ICsWithTypeInfoCountField: public BitField<int, 0,
      kSmiValueSize - kTypeChangeChecksumBits> {};  // NOLINT
  class InlinedTypeChangeChecksum: public BitField<int,
      kSmiValueSize - kTypeChangeChecksumBits,
      kTypeChangeChecksumBits> {};  // NOLINT

  DISALLOW_IMPLICIT_CONSTRUCTORS(TypeFeedbackInfo);
};


// Representation of a slow alias as part of a non-strict arguments objects.
// For fast aliases (if HasNonStrictArgumentsElements()):
// - the parameter map contains an index into the context
// - all attributes of the element have default values
// For slow aliases (if HasDictionaryArgumentsElements()):
// - the parameter map contains no fast alias mapping (i.e. the hole)
// - this struct (in the slow backing store) contains an index into the context
// - all attributes are available as part if the property details
class AliasedArgumentsEntry: public Struct {
 public:
  inline int aliased_context_slot();
  inline void set_aliased_context_slot(int count);

  static inline AliasedArgumentsEntry* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void AliasedArgumentsEntryPrint() {
    AliasedArgumentsEntryPrint(stdout);
  }
  void AliasedArgumentsEntryPrint(FILE* out);
#endif
  DECLARE_VERIFIER(AliasedArgumentsEntry)

  static const int kAliasedContextSlot = HeapObject::kHeaderSize;
  static const int kSize = kAliasedContextSlot + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(AliasedArgumentsEntry);
};


enum AllowNullsFlag {ALLOW_NULLS, DISALLOW_NULLS};
enum RobustnessFlag {ROBUST_STRING_TRAVERSAL, FAST_STRING_TRAVERSAL};


class StringHasher {
 public:
  explicit inline StringHasher(int length, uint32_t seed);

  // Returns true if the hash of this string can be computed without
  // looking at the contents.
  inline bool has_trivial_hash();

  // Add a character to the hash and update the array index calculation.
  inline void AddCharacter(uint32_t c);

  // Adds a character to the hash but does not update the array index
  // calculation.  This can only be called when it has been verified
  // that the input is not an array index.
  inline void AddCharacterNoIndex(uint32_t c);

  // Add a character above 0xffff as a surrogate pair.  These can get into
  // the hasher through the routines that take a UTF-8 string and make a symbol.
  void AddSurrogatePair(uc32 c);
  void AddSurrogatePairNoIndex(uc32 c);

  // Returns the value to store in the hash field of a string with
  // the given length and contents.
  uint32_t GetHashField();

  // Returns true if the characters seen so far make up a legal array
  // index.
  bool is_array_index() { return is_array_index_; }

  // Calculated hash value for a string consisting of 1 to
  // String::kMaxArrayIndexSize digits with no leading zeros (except "0").
  // value is represented decimal value.
  static uint32_t MakeArrayIndexHash(uint32_t value, int length);

  // No string is allowed to have a hash of zero.  That value is reserved
  // for internal properties.  If the hash calculation yields zero then we
  // use 27 instead.
  static const int kZeroHash = 27;

 private:
  uint32_t array_index() {
    ASSERT(is_array_index());
    return array_index_;
  }

  inline uint32_t GetHash();

  // Reusable parts of the hashing algorithm.
  INLINE(static uint32_t AddCharacterCore(uint32_t running_hash, uint32_t c));
  INLINE(static uint32_t GetHashCore(uint32_t running_hash));

  int length_;
  uint32_t raw_running_hash_;
  uint32_t array_index_;
  bool is_array_index_;
  bool is_first_char_;
  friend class TwoCharHashTableKey;

  template <bool seq_ascii> friend class JsonParser;
};


class IncrementalAsciiStringHasher {
 public:
  explicit inline IncrementalAsciiStringHasher(uint32_t seed, char first_char);
  inline void AddCharacter(uc32 c);
  inline uint32_t GetHash();

 private:
  int length_;
  uint32_t raw_running_hash_;
  uint32_t array_index_;
  bool is_array_index_;
  char first_char_;
};


// Calculates string hash.
template <typename schar>
inline uint32_t HashSequentialString(const schar* chars,
                                     int length,
                                     uint32_t seed);


// The characteristics of a string are stored in its map.  Retrieving these
// few bits of information is moderately expensive, involving two memory
// loads where the second is dependent on the first.  To improve efficiency
// the shape of the string is given its own class so that it can be retrieved
// once and used for several string operations.  A StringShape is small enough
// to be passed by value and is immutable, but be aware that flattening a
// string can potentially alter its shape.  Also be aware that a GC caused by
// something else can alter the shape of a string due to ConsString
// shortcutting.  Keeping these restrictions in mind has proven to be error-
// prone and so we no longer put StringShapes in variables unless there is a
// concrete performance benefit at that particular point in the code.
class StringShape BASE_EMBEDDED {
 public:
  inline explicit StringShape(String* s);
  inline explicit StringShape(Map* s);
  inline explicit StringShape(InstanceType t);
  inline bool IsSequential();
  inline bool IsExternal();
  inline bool IsCons();
  inline bool IsSliced();
  inline bool IsIndirect();
  inline bool IsExternalAscii();
  inline bool IsExternalTwoByte();
  inline bool IsSequentialAscii();
  inline bool IsSequentialTwoByte();
  inline bool IsSymbol();
  inline StringRepresentationTag representation_tag();
  inline uint32_t encoding_tag();
  inline uint32_t full_representation_tag();
  inline uint32_t size_tag();
#ifdef DEBUG
  inline uint32_t type() { return type_; }
  inline void invalidate() { valid_ = false; }
  inline bool valid() { return valid_; }
#else
  inline void invalidate() { }
#endif

 private:
  uint32_t type_;
#ifdef DEBUG
  inline void set_valid() { valid_ = true; }
  bool valid_;
#else
  inline void set_valid() { }
#endif
};


// The String abstract class captures JavaScript string values:
//
// Ecma-262:
//  4.3.16 String Value
//    A string value is a member of the type String and is a finite
//    ordered sequence of zero or more 16-bit unsigned integer values.
//
// All string values have a length field.
class String: public HeapObject {
 public:
  // Representation of the flat content of a String.
  // A non-flat string doesn't have flat content.
  // A flat string has content that's encoded as a sequence of either
  // ASCII chars or two-byte UC16.
  // Returned by String::GetFlatContent().
  class FlatContent {
   public:
    // Returns true if the string is flat and this structure contains content.
    bool IsFlat() { return state_ != NON_FLAT; }
    // Returns true if the structure contains ASCII content.
    bool IsAscii() { return state_ == ASCII; }
    // Returns true if the structure contains two-byte content.
    bool IsTwoByte() { return state_ == TWO_BYTE; }

    // Return the ASCII content of the string. Only use if IsAscii() returns
    // true.
    Vector<const char> ToAsciiVector() {
      ASSERT_EQ(ASCII, state_);
      return Vector<const char>::cast(buffer_);
    }
    // Return the two-byte content of the string. Only use if IsTwoByte()
    // returns true.
    Vector<const uc16> ToUC16Vector() {
      ASSERT_EQ(TWO_BYTE, state_);
      return Vector<const uc16>::cast(buffer_);
    }

   private:
    enum State { NON_FLAT, ASCII, TWO_BYTE };

    // Constructors only used by String::GetFlatContent().
    explicit FlatContent(Vector<const char> chars)
        : buffer_(Vector<const byte>::cast(chars)),
          state_(ASCII) { }
    explicit FlatContent(Vector<const uc16> chars)
        : buffer_(Vector<const byte>::cast(chars)),
          state_(TWO_BYTE) { }
    FlatContent() : buffer_(), state_(NON_FLAT) { }

    Vector<const byte> buffer_;
    State state_;

    friend class String;
  };

  // Get and set the length of the string.
  inline int length();
  inline void set_length(int value);

  // Get and set the hash field of the string.
  inline uint32_t hash_field();
  inline void set_hash_field(uint32_t value);

  // Returns whether this string has only ASCII chars, i.e. all of them can
  // be ASCII encoded.  This might be the case even if the string is
  // two-byte.  Such strings may appear when the embedder prefers
  // two-byte external representations even for ASCII data.
  inline bool IsAsciiRepresentation();
  inline bool IsTwoByteRepresentation();

  // Cons and slices have an encoding flag that may not represent the actual
  // encoding of the underlying string.  This is taken into account here.
  // Requires: this->IsFlat()
  inline bool IsAsciiRepresentationUnderneath();
  inline bool IsTwoByteRepresentationUnderneath();

  // NOTE: this should be considered only a hint.  False negatives are
  // possible.
  inline bool HasOnlyAsciiChars();

  // Get and set individual two byte chars in the string.
  inline void Set(int index, uint16_t value);
  // Get individual two byte char in the string.  Repeated calls
  // to this method are not efficient unless the string is flat.
  INLINE(uint16_t Get(int index));

  // Try to flatten the string.  Checks first inline to see if it is
  // necessary.  Does nothing if the string is not a cons string.
  // Flattening allocates a sequential string with the same data as
  // the given string and mutates the cons string to a degenerate
  // form, where the first component is the new sequential string and
  // the second component is the empty string.  If allocation fails,
  // this function returns a failure.  If flattening succeeds, this
  // function returns the sequential string that is now the first
  // component of the cons string.
  //
  // Degenerate cons strings are handled specially by the garbage
  // collector (see IsShortcutCandidate).
  //
  // Use FlattenString from Handles.cc to flatten even in case an
  // allocation failure happens.
  inline MaybeObject* TryFlatten(PretenureFlag pretenure = NOT_TENURED);

  // Convenience function.  Has exactly the same behavior as
  // TryFlatten(), except in the case of failure returns the original
  // string.
  inline String* TryFlattenGetString(PretenureFlag pretenure = NOT_TENURED);

  // Tries to return the content of a flat string as a structure holding either
  // a flat vector of char or of uc16.
  // If the string isn't flat, and therefore doesn't have flat content, the
  // returned structure will report so, and can't provide a vector of either
  // kind.
  FlatContent GetFlatContent();

  // Returns the parent of a sliced string or first part of a flat cons string.
  // Requires: StringShape(this).IsIndirect() && this->IsFlat()
  inline String* GetUnderlying();

  // Mark the string as an undetectable object. It only applies to
  // ASCII and two byte string types.
  bool MarkAsUndetectable();

  // Return a substring.
  MUST_USE_RESULT MaybeObject* SubString(int from,
                                         int to,
                                         PretenureFlag pretenure = NOT_TENURED);

  // String equality operations.
  inline bool Equals(String* other);
  bool IsEqualTo(Vector<const char> str);
  bool IsAsciiEqualTo(Vector<const char> str);
  bool IsTwoByteEqualTo(Vector<const uc16> str);

  // Return a UTF8 representation of the string.  The string is null
  // terminated but may optionally contain nulls.  Length is returned
  // in length_output if length_output is not a null pointer  The string
  // should be nearly flat, otherwise the performance of this method may
  // be very slow (quadratic in the length).  Setting robustness_flag to
  // ROBUST_STRING_TRAVERSAL invokes behaviour that is robust  This means it
  // handles unexpected data without causing assert failures and it does not
  // do any heap allocations.  This is useful when printing stack traces.
  SmartArrayPointer<char> ToCString(AllowNullsFlag allow_nulls,
                                    RobustnessFlag robustness_flag,
                                    int offset,
                                    int length,
                                    int* length_output = 0);
  SmartArrayPointer<char> ToCString(
      AllowNullsFlag allow_nulls = DISALLOW_NULLS,
      RobustnessFlag robustness_flag = FAST_STRING_TRAVERSAL,
      int* length_output = 0);

  // Return a 16 bit Unicode representation of the string.
  // The string should be nearly flat, otherwise the performance of
  // of this method may be very bad.  Setting robustness_flag to
  // ROBUST_STRING_TRAVERSAL invokes behaviour that is robust  This means it
  // handles unexpected data without causing assert failures and it does not
  // do any heap allocations.  This is useful when printing stack traces.
  SmartArrayPointer<uc16> ToWideCString(
      RobustnessFlag robustness_flag = FAST_STRING_TRAVERSAL);

  // Tells whether the hash code has been computed.
  inline bool HasHashCode();

  // Returns a hash value used for the property table
  inline uint32_t Hash();

  static uint32_t ComputeHashField(unibrow::CharacterStream* buffer,
                                   int length,
                                   uint32_t seed);

  static bool ComputeArrayIndex(unibrow::CharacterStream* buffer,
                                uint32_t* index,
                                int length);

  // Externalization.
  bool MakeExternal(v8::String::ExternalStringResource* resource);
  bool MakeExternal(v8::String::ExternalAsciiStringResource* resource);

  // Conversion.
  inline bool AsArrayIndex(uint32_t* index);

  // Casting.
  static inline String* cast(Object* obj);

  void PrintOn(FILE* out);

  // For use during stack traces.  Performs rudimentary sanity check.
  bool LooksValid();

  // Dispatched behavior.
  void StringShortPrint(StringStream* accumulator);
#ifdef OBJECT_PRINT
  inline void StringPrint() {
    StringPrint(stdout);
  }
  void StringPrint(FILE* out);

  char* ToAsciiArray();
#endif
  DECLARE_VERIFIER(String)

  inline bool IsFlat();

  // Layout description.
  static const int kLengthOffset = HeapObject::kHeaderSize;
  static const int kHashFieldOffset = kLengthOffset + kPointerSize;
  static const int kSize = kHashFieldOffset + kPointerSize;

  // Maximum number of characters to consider when trying to convert a string
  // value into an array index.
  static const int kMaxArrayIndexSize = 10;

  // Max ASCII char code.
  static const int kMaxAsciiCharCode = unibrow::Utf8::kMaxOneByteChar;
  static const unsigned kMaxAsciiCharCodeU = unibrow::Utf8::kMaxOneByteChar;
  static const int kMaxUtf16CodeUnit = 0xffff;

  // Mask constant for checking if a string has a computed hash code
  // and if it is an array index.  The least significant bit indicates
  // whether a hash code has been computed.  If the hash code has been
  // computed the 2nd bit tells whether the string can be used as an
  // array index.
  static const int kHashNotComputedMask = 1;
  static const int kIsNotArrayIndexMask = 1 << 1;
  static const int kNofHashBitFields = 2;

  // Shift constant retrieving hash code from hash field.
  static const int kHashShift = kNofHashBitFields;

  // Only these bits are relevant in the hash, since the top two are shifted
  // out.
  static const uint32_t kHashBitMask = 0xffffffffu >> kHashShift;

  // Array index strings this short can keep their index in the hash
  // field.
  static const int kMaxCachedArrayIndexLength = 7;

  // For strings which are array indexes the hash value has the string length
  // mixed into the hash, mainly to avoid a hash value of zero which would be
  // the case for the string '0'. 24 bits are used for the array index value.
  static const int kArrayIndexValueBits = 24;
  static const int kArrayIndexLengthBits =
      kBitsPerInt - kArrayIndexValueBits - kNofHashBitFields;

  STATIC_CHECK((kArrayIndexLengthBits > 0));
  STATIC_CHECK(kMaxArrayIndexSize < (1 << kArrayIndexLengthBits));

  static const int kArrayIndexHashLengthShift =
      kArrayIndexValueBits + kNofHashBitFields;

  static const int kArrayIndexHashMask = (1 << kArrayIndexHashLengthShift) - 1;

  static const int kArrayIndexValueMask =
      ((1 << kArrayIndexValueBits) - 1) << kHashShift;

  // Check that kMaxCachedArrayIndexLength + 1 is a power of two so we
  // could use a mask to test if the length of string is less than or equal to
  // kMaxCachedArrayIndexLength.
  STATIC_CHECK(IS_POWER_OF_TWO(kMaxCachedArrayIndexLength + 1));

  static const int kContainsCachedArrayIndexMask =
      (~kMaxCachedArrayIndexLength << kArrayIndexHashLengthShift) |
      kIsNotArrayIndexMask;

  // Value of empty hash field indicating that the hash is not computed.
  static const int kEmptyHashField =
      kIsNotArrayIndexMask | kHashNotComputedMask;

  // Value of hash field containing computed hash equal to zero.
  static const int kEmptyStringHash = kIsNotArrayIndexMask;

  // Maximal string length.
  static const int kMaxLength = (1 << (32 - 2)) - 1;

  // Max length for computing hash. For strings longer than this limit the
  // string length is used as the hash value.
  static const int kMaxHashCalcLength = 16383;

  // Limit for truncation in short printing.
  static const int kMaxShortPrintLength = 1024;

  // Support for regular expressions.
  const uc16* GetTwoByteData();
  const uc16* GetTwoByteData(unsigned start);

  // Support for StringInputBuffer
  static const unibrow::byte* ReadBlock(String* input,
                                        unibrow::byte* util_buffer,
                                        unsigned capacity,
                                        unsigned* remaining,
                                        unsigned* offset);
  static const unibrow::byte* ReadBlock(String** input,
                                        unibrow::byte* util_buffer,
                                        unsigned capacity,
                                        unsigned* remaining,
                                        unsigned* offset);

  // Helper function for flattening strings.
  template <typename sinkchar>
  static void WriteToFlat(String* source,
                          sinkchar* sink,
                          int from,
                          int to);

  // The return value may point to the first aligned word containing the
  // first non-ascii character, rather than directly to the non-ascii character.
  // If the return value is >= the passed length, the entire string was ASCII.
  static inline int NonAsciiStart(const char* chars, int length) {
    const char* start = chars;
    const char* limit = chars + length;
#ifdef V8_HOST_CAN_READ_UNALIGNED
    ASSERT(kMaxAsciiCharCode == 0x7F);
    const uintptr_t non_ascii_mask = kUintptrAllBitsSet / 0xFF * 0x80;
    while (chars + sizeof(uintptr_t) <= limit) {
      if (*reinterpret_cast<const uintptr_t*>(chars) & non_ascii_mask) {
        return static_cast<int>(chars - start);
      }
      chars += sizeof(uintptr_t);
    }
#endif
    while (chars < limit) {
      if (static_cast<uint8_t>(*chars) > kMaxAsciiCharCodeU) {
        return static_cast<int>(chars - start);
      }
      ++chars;
    }
    return static_cast<int>(chars - start);
  }

  static inline bool IsAscii(const char* chars, int length) {
    return NonAsciiStart(chars, length) >= length;
  }

  static inline int NonAsciiStart(const uc16* chars, int length) {
    const uc16* limit = chars + length;
    const uc16* start = chars;
    while (chars < limit) {
      if (*chars > kMaxAsciiCharCodeU) return static_cast<int>(chars - start);
      ++chars;
    }
    return static_cast<int>(chars - start);
  }

  static inline bool IsAscii(const uc16* chars, int length) {
    return NonAsciiStart(chars, length) >= length;
  }

 protected:
  class ReadBlockBuffer {
   public:
    ReadBlockBuffer(unibrow::byte* util_buffer_,
                    unsigned cursor_,
                    unsigned capacity_,
                    unsigned remaining_) :
      util_buffer(util_buffer_),
      cursor(cursor_),
      capacity(capacity_),
      remaining(remaining_) {
    }
    unibrow::byte* util_buffer;
    unsigned       cursor;
    unsigned       capacity;
    unsigned       remaining;
  };

  static inline const unibrow::byte* ReadBlock(String* input,
                                               ReadBlockBuffer* buffer,
                                               unsigned* offset,
                                               unsigned max_chars);
  static void ReadBlockIntoBuffer(String* input,
                                  ReadBlockBuffer* buffer,
                                  unsigned* offset_ptr,
                                  unsigned max_chars);

 private:
  // Try to flatten the top level ConsString that is hiding behind this
  // string.  This is a no-op unless the string is a ConsString.  Flatten
  // mutates the ConsString and might return a failure.
  MUST_USE_RESULT MaybeObject* SlowTryFlatten(PretenureFlag pretenure);

  static inline bool IsHashFieldComputed(uint32_t field);

  // Slow case of String::Equals.  This implementation works on any strings
  // but it is most efficient on strings that are almost flat.
  bool SlowEquals(String* other);

  // Slow case of AsArrayIndex.
  bool SlowAsArrayIndex(uint32_t* index);

  // Compute and set the hash code.
  uint32_t ComputeAndSetHash();

  DISALLOW_IMPLICIT_CONSTRUCTORS(String);
};


// The SeqString abstract class captures sequential string values.
class SeqString: public String {
 public:
  // Casting.
  static inline SeqString* cast(Object* obj);

  // Layout description.
  static const int kHeaderSize = String::kSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SeqString);
};


// The AsciiString class captures sequential ASCII string objects.
// Each character in the AsciiString is an ASCII character.
class SeqAsciiString: public SeqString {
 public:
  static const bool kHasAsciiEncoding = true;

  // Dispatched behavior.
  inline uint16_t SeqAsciiStringGet(int index);
  inline void SeqAsciiStringSet(int index, uint16_t value);

  // Get the address of the characters in this string.
  inline Address GetCharsAddress();

  inline char* GetChars();

  // Casting
  static inline SeqAsciiString* cast(Object* obj);

  // Garbage collection support.  This method is called by the
  // garbage collector to compute the actual size of an AsciiString
  // instance.
  inline int SeqAsciiStringSize(InstanceType instance_type);

  // Computes the size for an AsciiString instance of a given length.
  static int SizeFor(int length) {
    return OBJECT_POINTER_ALIGN(kHeaderSize + length * kCharSize);
  }

  // Maximal memory usage for a single sequential ASCII string.
  static const int kMaxSize = 512 * MB - 1;
  // Maximal length of a single sequential ASCII string.
  // Q.v. String::kMaxLength which is the maximal size of concatenated strings.
  static const int kMaxLength = (kMaxSize - kHeaderSize);

  // Support for StringInputBuffer.
  inline void SeqAsciiStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                                unsigned* offset,
                                                unsigned chars);
  inline const unibrow::byte* SeqAsciiStringReadBlock(unsigned* remaining,
                                                      unsigned* offset,
                                                      unsigned chars);

  DECLARE_VERIFIER(SeqAsciiString)

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SeqAsciiString);
};


// The TwoByteString class captures sequential unicode string objects.
// Each character in the TwoByteString is a two-byte uint16_t.
class SeqTwoByteString: public SeqString {
 public:
  static const bool kHasAsciiEncoding = false;

  // Dispatched behavior.
  inline uint16_t SeqTwoByteStringGet(int index);
  inline void SeqTwoByteStringSet(int index, uint16_t value);

  // Get the address of the characters in this string.
  inline Address GetCharsAddress();

  inline uc16* GetChars();

  // For regexp code.
  const uint16_t* SeqTwoByteStringGetData(unsigned start);

  // Casting
  static inline SeqTwoByteString* cast(Object* obj);

  // Garbage collection support.  This method is called by the
  // garbage collector to compute the actual size of a TwoByteString
  // instance.
  inline int SeqTwoByteStringSize(InstanceType instance_type);

  // Computes the size for a TwoByteString instance of a given length.
  static int SizeFor(int length) {
    return OBJECT_POINTER_ALIGN(kHeaderSize + length * kShortSize);
  }

  // Maximal memory usage for a single sequential two-byte string.
  static const int kMaxSize = 512 * MB - 1;
  // Maximal length of a single sequential two-byte string.
  // Q.v. String::kMaxLength which is the maximal size of concatenated strings.
  static const int kMaxLength = (kMaxSize - kHeaderSize) / sizeof(uint16_t);

  // Support for StringInputBuffer.
  inline void SeqTwoByteStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                                  unsigned* offset_ptr,
                                                  unsigned chars);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SeqTwoByteString);
};


// The ConsString class describes string values built by using the
// addition operator on strings.  A ConsString is a pair where the
// first and second components are pointers to other string values.
// One or both components of a ConsString can be pointers to other
// ConsStrings, creating a binary tree of ConsStrings where the leaves
// are non-ConsString string values.  The string value represented by
// a ConsString can be obtained by concatenating the leaf string
// values in a left-to-right depth-first traversal of the tree.
class ConsString: public String {
 public:
  // First string of the cons cell.
  inline String* first();
  // Doesn't check that the result is a string, even in debug mode.  This is
  // useful during GC where the mark bits confuse the checks.
  inline Object* unchecked_first();
  inline void set_first(String* first,
                        WriteBarrierMode mode = UPDATE_WRITE_BARRIER);

  // Second string of the cons cell.
  inline String* second();
  // Doesn't check that the result is a string, even in debug mode.  This is
  // useful during GC where the mark bits confuse the checks.
  inline Object* unchecked_second();
  inline void set_second(String* second,
                         WriteBarrierMode mode = UPDATE_WRITE_BARRIER);

  // Dispatched behavior.
  uint16_t ConsStringGet(int index);

  // Casting.
  static inline ConsString* cast(Object* obj);

  // Layout description.
  static const int kFirstOffset = POINTER_SIZE_ALIGN(String::kSize);
  static const int kSecondOffset = kFirstOffset + kPointerSize;
  static const int kSize = kSecondOffset + kPointerSize;

  // Support for StringInputBuffer.
  inline const unibrow::byte* ConsStringReadBlock(ReadBlockBuffer* buffer,
                                                  unsigned* offset_ptr,
                                                  unsigned chars);
  inline void ConsStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                            unsigned* offset_ptr,
                                            unsigned chars);

  // Minimum length for a cons string.
  static const int kMinLength = 13;

  typedef FixedBodyDescriptor<kFirstOffset, kSecondOffset + kPointerSize, kSize>
          BodyDescriptor;

  DECLARE_VERIFIER(ConsString)

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ConsString);
};


// The Sliced String class describes strings that are substrings of another
// sequential string.  The motivation is to save time and memory when creating
// a substring.  A Sliced String is described as a pointer to the parent,
// the offset from the start of the parent string and the length.  Using
// a Sliced String therefore requires unpacking of the parent string and
// adding the offset to the start address.  A substring of a Sliced String
// are not nested since the double indirection is simplified when creating
// such a substring.
// Currently missing features are:
//  - handling externalized parent strings
//  - external strings as parent
//  - truncating sliced string to enable otherwise unneeded parent to be GC'ed.
class SlicedString: public String {
 public:
  inline String* parent();
  inline void set_parent(String* parent,
                         WriteBarrierMode mode = UPDATE_WRITE_BARRIER);
  inline int offset();
  inline void set_offset(int offset);

  // Dispatched behavior.
  uint16_t SlicedStringGet(int index);

  // Casting.
  static inline SlicedString* cast(Object* obj);

  // Layout description.
  static const int kParentOffset = POINTER_SIZE_ALIGN(String::kSize);
  static const int kOffsetOffset = kParentOffset + kPointerSize;
  static const int kSize = kOffsetOffset + kPointerSize;

  // Support for StringInputBuffer
  inline const unibrow::byte* SlicedStringReadBlock(ReadBlockBuffer* buffer,
                                                    unsigned* offset_ptr,
                                                    unsigned chars);
  inline void SlicedStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                              unsigned* offset_ptr,
                                              unsigned chars);
  // Minimum length for a sliced string.
  static const int kMinLength = 13;

  typedef FixedBodyDescriptor<kParentOffset,
                              kOffsetOffset + kPointerSize, kSize>
          BodyDescriptor;

  DECLARE_VERIFIER(SlicedString)

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SlicedString);
};


// The ExternalString class describes string values that are backed by
// a string resource that lies outside the V8 heap.  ExternalStrings
// consist of the length field common to all strings, a pointer to the
// external resource.  It is important to ensure (externally) that the
// resource is not deallocated while the ExternalString is live in the
// V8 heap.
//
// The API expects that all ExternalStrings are created through the
// API.  Therefore, ExternalStrings should not be used internally.
class ExternalString: public String {
 public:
  // Casting
  static inline ExternalString* cast(Object* obj);

  // Layout description.
  static const int kResourceOffset = POINTER_SIZE_ALIGN(String::kSize);
  static const int kShortSize = kResourceOffset + kPointerSize;
  static const int kResourceDataOffset = kResourceOffset + kPointerSize;
  static const int kSize = kResourceDataOffset + kPointerSize;

  // Return whether external string is short (data pointer is not cached).
  inline bool is_short();

  STATIC_CHECK(kResourceOffset == Internals::kStringResourceOffset);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ExternalString);
};


// The ExternalAsciiString class is an external string backed by an
// ASCII string.
class ExternalAsciiString: public ExternalString {
 public:
  static const bool kHasAsciiEncoding = true;

  typedef v8::String::ExternalAsciiStringResource Resource;

  // The underlying resource.
  inline const Resource* resource();
  inline void set_resource(const Resource* buffer);

  // Update the pointer cache to the external character array.
  // The cached pointer is always valid, as the external character array does =
  // not move during lifetime.  Deserialization is the only exception, after
  // which the pointer cache has to be refreshed.
  inline void update_data_cache();

  inline const char* GetChars();

  // Dispatched behavior.
  inline uint16_t ExternalAsciiStringGet(int index);

  // Casting.
  static inline ExternalAsciiString* cast(Object* obj);

  // Garbage collection support.
  inline void ExternalAsciiStringIterateBody(ObjectVisitor* v);

  template<typename StaticVisitor>
  inline void ExternalAsciiStringIterateBody();

  // Support for StringInputBuffer.
  const unibrow::byte* ExternalAsciiStringReadBlock(unsigned* remaining,
                                                    unsigned* offset,
                                                    unsigned chars);
  inline void ExternalAsciiStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                                     unsigned* offset,
                                                     unsigned chars);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ExternalAsciiString);
};


// The ExternalTwoByteString class is an external string backed by a UTF-16
// encoded string.
class ExternalTwoByteString: public ExternalString {
 public:
  static const bool kHasAsciiEncoding = false;

  typedef v8::String::ExternalStringResource Resource;

  // The underlying string resource.
  inline const Resource* resource();
  inline void set_resource(const Resource* buffer);

  // Update the pointer cache to the external character array.
  // The cached pointer is always valid, as the external character array does =
  // not move during lifetime.  Deserialization is the only exception, after
  // which the pointer cache has to be refreshed.
  inline void update_data_cache();

  inline const uint16_t* GetChars();

  // Dispatched behavior.
  inline uint16_t ExternalTwoByteStringGet(int index);

  // For regexp code.
  inline const uint16_t* ExternalTwoByteStringGetData(unsigned start);

  // Casting.
  static inline ExternalTwoByteString* cast(Object* obj);

  // Garbage collection support.
  inline void ExternalTwoByteStringIterateBody(ObjectVisitor* v);

  template<typename StaticVisitor>
  inline void ExternalTwoByteStringIterateBody();


  // Support for StringInputBuffer.
  void ExternalTwoByteStringReadBlockIntoBuffer(ReadBlockBuffer* buffer,
                                                unsigned* offset_ptr,
                                                unsigned chars);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(ExternalTwoByteString);
};


// Utility superclass for stack-allocated objects that must be updated
// on gc.  It provides two ways for the gc to update instances, either
// iterating or updating after gc.
class Relocatable BASE_EMBEDDED {
 public:
  explicit inline Relocatable(Isolate* isolate);
  inline virtual ~Relocatable();
  virtual void IterateInstance(ObjectVisitor* v) { }
  virtual void PostGarbageCollection() { }

  static void PostGarbageCollectionProcessing();
  static int ArchiveSpacePerThread();
  static char* ArchiveState(Isolate* isolate, char* to);
  static char* RestoreState(Isolate* isolate, char* from);
  static void Iterate(ObjectVisitor* v);
  static void Iterate(ObjectVisitor* v, Relocatable* top);
  static char* Iterate(ObjectVisitor* v, char* t);
 private:
  Isolate* isolate_;
  Relocatable* prev_;
};


// A flat string reader provides random access to the contents of a
// string independent of the character width of the string.  The handle
// must be valid as long as the reader is being used.
class FlatStringReader : public Relocatable {
 public:
  FlatStringReader(Isolate* isolate, Handle<String> str);
  FlatStringReader(Isolate* isolate, Vector<const char> input);
  void PostGarbageCollection();
  inline uc32 Get(int index);
  int length() { return length_; }
 private:
  String** str_;
  bool is_ascii_;
  int length_;
  const void* start_;
};


// Note that StringInputBuffers are not valid across a GC!  To fix this
// it would have to store a String Handle instead of a String* and
// AsciiStringReadBlock would have to be modified to use memcpy.
//
// StringInputBuffer is able to traverse any string regardless of how
// deeply nested a sequence of ConsStrings it is made of.  However,
// performance will be better if deep strings are flattened before they
// are traversed.  Since flattening requires memory allocation this is
// not always desirable, however (esp. in debugging situations).
class StringInputBuffer: public unibrow::InputBuffer<String, String*, 1024> {
 public:
  virtual void Seek(unsigned pos);
  inline StringInputBuffer(): unibrow::InputBuffer<String, String*, 1024>() {}
  explicit inline StringInputBuffer(String* backing):
      unibrow::InputBuffer<String, String*, 1024>(backing) {}
};


class SafeStringInputBuffer
  : public unibrow::InputBuffer<String, String**, 256> {
 public:
  virtual void Seek(unsigned pos);
  inline SafeStringInputBuffer()
      : unibrow::InputBuffer<String, String**, 256>() {}
  explicit inline SafeStringInputBuffer(String** backing)
      : unibrow::InputBuffer<String, String**, 256>(backing) {}
};


template <typename T>
class VectorIterator {
 public:
  VectorIterator(T* d, int l) : data_(Vector<const T>(d, l)), index_(0) { }
  explicit VectorIterator(Vector<const T> data) : data_(data), index_(0) { }
  T GetNext() { return data_[index_++]; }
  bool has_more() { return index_ < data_.length(); }
 private:
  Vector<const T> data_;
  int index_;
};


// The Oddball describes objects null, undefined, true, and false.
class Oddball: public HeapObject {
 public:
  // [to_string]: Cached to_string computed at startup.
  DECL_ACCESSORS(to_string, String)

  // [to_number]: Cached to_number computed at startup.
  DECL_ACCESSORS(to_number, Object)

  inline byte kind();
  inline void set_kind(byte kind);

  // Casting.
  static inline Oddball* cast(Object* obj);

  // Dispatched behavior.
  DECLARE_VERIFIER(Oddball)

  // Initialize the fields.
  MUST_USE_RESULT MaybeObject* Initialize(const char* to_string,
                                          Object* to_number,
                                          byte kind);

  // Layout description.
  static const int kToStringOffset = HeapObject::kHeaderSize;
  static const int kToNumberOffset = kToStringOffset + kPointerSize;
  static const int kKindOffset = kToNumberOffset + kPointerSize;
  static const int kSize = kKindOffset + kPointerSize;

  static const byte kFalse = 0;
  static const byte kTrue = 1;
  static const byte kNotBooleanMask = ~1;
  static const byte kTheHole = 2;
  static const byte kNull = 3;
  static const byte kArgumentMarker = 4;
  static const byte kUndefined = 5;
  static const byte kOther = 6;

  typedef FixedBodyDescriptor<kToStringOffset,
                              kToNumberOffset + kPointerSize,
                              kSize> BodyDescriptor;

  STATIC_CHECK(kKindOffset == Internals::kOddballKindOffset);
  STATIC_CHECK(kNull == Internals::kNullOddballKind);
  STATIC_CHECK(kUndefined == Internals::kUndefinedOddballKind);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Oddball);
};


class JSGlobalPropertyCell: public HeapObject {
 public:
  // [value]: value of the global property.
  DECL_ACCESSORS(value, Object)

  // Casting.
  static inline JSGlobalPropertyCell* cast(Object* obj);

  static inline JSGlobalPropertyCell* FromValueAddress(Address value) {
    return cast(FromAddress(value - kValueOffset));
  }

  inline Address ValueAddress() {
    return address() + kValueOffset;
  }

  DECLARE_VERIFIER(JSGlobalPropertyCell)

#ifdef OBJECT_PRINT
  inline void JSGlobalPropertyCellPrint() {
    JSGlobalPropertyCellPrint(stdout);
  }
  void JSGlobalPropertyCellPrint(FILE* out);
#endif

  // Layout description.
  static const int kValueOffset = HeapObject::kHeaderSize;
  static const int kSize = kValueOffset + kPointerSize;

  typedef FixedBodyDescriptor<kValueOffset,
                              kValueOffset + kPointerSize,
                              kSize> BodyDescriptor;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSGlobalPropertyCell);
};


// The JSProxy describes EcmaScript Harmony proxies
class JSProxy: public JSReceiver {
 public:
  // [handler]: The handler property.
  DECL_ACCESSORS(handler, Object)

  // [hash]: The hash code property (undefined if not initialized yet).
  DECL_ACCESSORS(hash, Object)

  // Casting.
  static inline JSProxy* cast(Object* obj);

  bool HasPropertyWithHandler(String* name);
  bool HasElementWithHandler(uint32_t index);

  MUST_USE_RESULT MaybeObject* GetPropertyWithHandler(
      Object* receiver,
      String* name);
  MUST_USE_RESULT MaybeObject* GetElementWithHandler(
      Object* receiver,
      uint32_t index);

  MUST_USE_RESULT MaybeObject* SetPropertyWithHandler(
      JSReceiver* receiver,
      String* name,
      Object* value,
      PropertyAttributes attributes,
      StrictModeFlag strict_mode);
  MUST_USE_RESULT MaybeObject* SetElementWithHandler(
      JSReceiver* receiver,
      uint32_t index,
      Object* value,
      StrictModeFlag strict_mode);

  // If the handler defines an accessor property with a setter, invoke it.
  // If it defines an accessor property without a setter, or a data property
  // that is read-only, throw. In all these cases set '*done' to true,
  // otherwise set it to false.
  MUST_USE_RESULT MaybeObject* SetPropertyViaPrototypesWithHandler(
      JSReceiver* receiver,
      String* name,
      Object* value,
      PropertyAttributes attributes,
      StrictModeFlag strict_mode,
      bool* done);

  MUST_USE_RESULT MaybeObject* DeletePropertyWithHandler(
      String* name,
      DeleteMode mode);
  MUST_USE_RESULT MaybeObject* DeleteElementWithHandler(
      uint32_t index,
      DeleteMode mode);

  MUST_USE_RESULT PropertyAttributes GetPropertyAttributeWithHandler(
      JSReceiver* receiver,
      String* name);
  MUST_USE_RESULT PropertyAttributes GetElementAttributeWithHandler(
      JSReceiver* receiver,
      uint32_t index);

  MUST_USE_RESULT MaybeObject* GetIdentityHash(CreationFlag flag);

  // Turn this into an (empty) JSObject.
  void Fix();

  // Initializes the body after the handler slot.
  inline void InitializeBody(int object_size, Object* value);

  // Invoke a trap by name. If the trap does not exist on this's handler,
  // but derived_trap is non-NULL, invoke that instead.  May cause GC.
  Handle<Object> CallTrap(const char* name,
                          Handle<Object> derived_trap,
                          int argc,
                          Handle<Object> args[]);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSProxyPrint() {
    JSProxyPrint(stdout);
  }
  void JSProxyPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSProxy)

  // Layout description. We add padding so that a proxy has the same
  // size as a virgin JSObject. This is essential for becoming a JSObject
  // upon freeze.
  static const int kHandlerOffset = HeapObject::kHeaderSize;
  static const int kHashOffset = kHandlerOffset + kPointerSize;
  static const int kPaddingOffset = kHashOffset + kPointerSize;
  static const int kSize = JSObject::kHeaderSize;
  static const int kHeaderSize = kPaddingOffset;
  static const int kPaddingSize = kSize - kPaddingOffset;

  STATIC_CHECK(kPaddingSize >= 0);

  typedef FixedBodyDescriptor<kHandlerOffset,
                              kPaddingOffset,
                              kSize> BodyDescriptor;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSProxy);
};


class JSFunctionProxy: public JSProxy {
 public:
  // [call_trap]: The call trap.
  DECL_ACCESSORS(call_trap, Object)

  // [construct_trap]: The construct trap.
  DECL_ACCESSORS(construct_trap, Object)

  // Casting.
  static inline JSFunctionProxy* cast(Object* obj);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSFunctionProxyPrint() {
    JSFunctionProxyPrint(stdout);
  }
  void JSFunctionProxyPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSFunctionProxy)

  // Layout description.
  static const int kCallTrapOffset = JSProxy::kPaddingOffset;
  static const int kConstructTrapOffset = kCallTrapOffset + kPointerSize;
  static const int kPaddingOffset = kConstructTrapOffset + kPointerSize;
  static const int kSize = JSFunction::kSize;
  static const int kPaddingSize = kSize - kPaddingOffset;

  STATIC_CHECK(kPaddingSize >= 0);

  typedef FixedBodyDescriptor<kHandlerOffset,
                              kConstructTrapOffset + kPointerSize,
                              kSize> BodyDescriptor;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSFunctionProxy);
};


// The JSSet describes EcmaScript Harmony sets
class JSSet: public JSObject {
 public:
  // [set]: the backing hash set containing keys.
  DECL_ACCESSORS(table, Object)

  // Casting.
  static inline JSSet* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void JSSetPrint() {
    JSSetPrint(stdout);
  }
  void JSSetPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSSet)

  static const int kTableOffset = JSObject::kHeaderSize;
  static const int kSize = kTableOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSSet);
};


// The JSMap describes EcmaScript Harmony maps
class JSMap: public JSObject {
 public:
  // [table]: the backing hash table mapping keys to values.
  DECL_ACCESSORS(table, Object)

  // Casting.
  static inline JSMap* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void JSMapPrint() {
    JSMapPrint(stdout);
  }
  void JSMapPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSMap)

  static const int kTableOffset = JSObject::kHeaderSize;
  static const int kSize = kTableOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSMap);
};


// The JSWeakMap describes EcmaScript Harmony weak maps
class JSWeakMap: public JSObject {
 public:
  // [table]: the backing hash table mapping keys to values.
  DECL_ACCESSORS(table, Object)

  // [next]: linked list of encountered weak maps during GC.
  DECL_ACCESSORS(next, Object)

  // Casting.
  static inline JSWeakMap* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void JSWeakMapPrint() {
    JSWeakMapPrint(stdout);
  }
  void JSWeakMapPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSWeakMap)

  static const int kTableOffset = JSObject::kHeaderSize;
  static const int kNextOffset = kTableOffset + kPointerSize;
  static const int kSize = kNextOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSWeakMap);
};


// Foreign describes objects pointing from JavaScript to C structures.
// Since they cannot contain references to JS HeapObjects they can be
// placed in old_data_space.
class Foreign: public HeapObject {
 public:
  // [address]: field containing the address.
  inline Address foreign_address();
  inline void set_foreign_address(Address value);

  // Casting.
  static inline Foreign* cast(Object* obj);

  // Dispatched behavior.
  inline void ForeignIterateBody(ObjectVisitor* v);

  template<typename StaticVisitor>
  inline void ForeignIterateBody();

#ifdef OBJECT_PRINT
  inline void ForeignPrint() {
    ForeignPrint(stdout);
  }
  void ForeignPrint(FILE* out);
#endif
  DECLARE_VERIFIER(Foreign)

  // Layout description.

  static const int kForeignAddressOffset = HeapObject::kHeaderSize;
  static const int kSize = kForeignAddressOffset + kPointerSize;

  STATIC_CHECK(kForeignAddressOffset == Internals::kForeignAddressOffset);

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(Foreign);
};


// The JSArray describes JavaScript Arrays
//  Such an array can be in one of two modes:
//    - fast, backing storage is a FixedArray and length <= elements.length();
//       Please note: push and pop can be used to grow and shrink the array.
//    - slow, backing storage is a HashTable with numbers as keys.
class JSArray: public JSObject {
 public:
  // [length]: The length property.
  DECL_ACCESSORS(length, Object)

  // Overload the length setter to skip write barrier when the length
  // is set to a smi. This matches the set function on FixedArray.
  inline void set_length(Smi* length);

  MUST_USE_RESULT MaybeObject* JSArrayUpdateLengthFromIndex(uint32_t index,
                                                            Object* value);

  // Initialize the array with the given capacity. The function may
  // fail due to out-of-memory situations, but only if the requested
  // capacity is non-zero.
  MUST_USE_RESULT MaybeObject* Initialize(int capacity);

  // Initializes the array to a certain length.
  inline bool AllowsSetElementsLength();
  MUST_USE_RESULT MaybeObject* SetElementsLength(Object* length);

  // Set the content of the array to the content of storage.
  MUST_USE_RESULT inline MaybeObject* SetContent(FixedArrayBase* storage);

  // Casting.
  static inline JSArray* cast(Object* obj);

  // Uses handles.  Ensures that the fixed array backing the JSArray has at
  // least the stated size.
  inline void EnsureSize(int minimum_size_of_backing_fixed_array);

  // Dispatched behavior.
#ifdef OBJECT_PRINT
  inline void JSArrayPrint() {
    JSArrayPrint(stdout);
  }
  void JSArrayPrint(FILE* out);
#endif
  DECLARE_VERIFIER(JSArray)

  // Number of element slots to pre-allocate for an empty array.
  static const int kPreallocatedArrayElements = 4;

  // Layout description.
  static const int kLengthOffset = JSObject::kHeaderSize;
  static const int kSize = kLengthOffset + kPointerSize;

 private:
  // Expand the fixed array backing of a fast-case JSArray to at least
  // the requested size.
  void Expand(int minimum_size_of_backing_fixed_array);

  DISALLOW_IMPLICIT_CONSTRUCTORS(JSArray);
};


// JSRegExpResult is just a JSArray with a specific initial map.
// This initial map adds in-object properties for "index" and "input"
// properties, as assigned by RegExp.prototype.exec, which allows
// faster creation of RegExp exec results.
// This class just holds constants used when creating the result.
// After creation the result must be treated as a JSArray in all regards.
class JSRegExpResult: public JSArray {
 public:
  // Offsets of object fields.
  static const int kIndexOffset = JSArray::kSize;
  static const int kInputOffset = kIndexOffset + kPointerSize;
  static const int kSize = kInputOffset + kPointerSize;
  // Indices of in-object properties.
  static const int kIndexIndex = 0;
  static const int kInputIndex = 1;
 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(JSRegExpResult);
};


// An accessor must have a getter, but can have no setter.
//
// When setting a property, V8 searches accessors in prototypes.
// If an accessor was found and it does not have a setter,
// the request is ignored.
//
// If the accessor in the prototype has the READ_ONLY property attribute, then
// a new value is added to the local object when the property is set.
// This shadows the accessor in the prototype.
class AccessorInfo: public Struct {
 public:
  DECL_ACCESSORS(getter, Object)
  DECL_ACCESSORS(setter, Object)
  DECL_ACCESSORS(data, Object)
  DECL_ACCESSORS(name, Object)
  DECL_ACCESSORS(flag, Smi)
  DECL_ACCESSORS(expected_receiver_type, Object)

  inline bool all_can_read();
  inline void set_all_can_read(bool value);

  inline bool all_can_write();
  inline void set_all_can_write(bool value);

  inline bool prohibits_overwriting();
  inline void set_prohibits_overwriting(bool value);

  inline PropertyAttributes property_attributes();
  inline void set_property_attributes(PropertyAttributes attributes);

  // Checks whether the given receiver is compatible with this accessor.
  inline bool IsCompatibleReceiver(Object* receiver);

  static inline AccessorInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void AccessorInfoPrint() {
    AccessorInfoPrint(stdout);
  }
  void AccessorInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(AccessorInfo)

  static const int kGetterOffset = HeapObject::kHeaderSize;
  static const int kSetterOffset = kGetterOffset + kPointerSize;
  static const int kDataOffset = kSetterOffset + kPointerSize;
  static const int kNameOffset = kDataOffset + kPointerSize;
  static const int kFlagOffset = kNameOffset + kPointerSize;
  static const int kExpectedReceiverTypeOffset = kFlagOffset + kPointerSize;
  static const int kSize = kExpectedReceiverTypeOffset + kPointerSize;

 private:
  // Bit positions in flag.
  static const int kAllCanReadBit = 0;
  static const int kAllCanWriteBit = 1;
  static const int kProhibitsOverwritingBit = 2;
  class AttributesField: public BitField<PropertyAttributes, 3, 3> {};

  DISALLOW_IMPLICIT_CONSTRUCTORS(AccessorInfo);
};


// Support for JavaScript accessors: A pair of a getter and a setter. Each
// accessor can either be
//   * a pointer to a JavaScript function or proxy: a real accessor
//   * undefined: considered an accessor by the spec, too, strangely enough
//   * the hole: an accessor which has not been set
//   * a pointer to a map: a transition used to ensure map sharing
class AccessorPair: public Struct {
 public:
  DECL_ACCESSORS(getter, Object)
  DECL_ACCESSORS(setter, Object)

  static inline AccessorPair* cast(Object* obj);

  MUST_USE_RESULT MaybeObject* Copy();

  Object* get(AccessorComponent component) {
    return component == ACCESSOR_GETTER ? getter() : setter();
  }

  void set(AccessorComponent component, Object* value) {
    if (component == ACCESSOR_GETTER) {
      set_getter(value);
    } else {
      set_setter(value);
    }
  }

  // Note: Returns undefined instead in case of a hole.
  Object* GetComponent(AccessorComponent component);

  // Set both components, skipping arguments which are a JavaScript null.
  void SetComponents(Object* getter, Object* setter) {
    if (!getter->IsNull()) set_getter(getter);
    if (!setter->IsNull()) set_setter(setter);
  }

  bool ContainsAccessor() {
    return IsJSAccessor(getter()) || IsJSAccessor(setter());
  }

#ifdef OBJECT_PRINT
  void AccessorPairPrint(FILE* out = stdout);
#endif
  DECLARE_VERIFIER(AccessorPair)

  static const int kGetterOffset = HeapObject::kHeaderSize;
  static const int kSetterOffset = kGetterOffset + kPointerSize;
  static const int kSize = kSetterOffset + kPointerSize;

 private:
  // Strangely enough, in addition to functions and harmony proxies, the spec
  // requires us to consider undefined as a kind of accessor, too:
  //    var obj = {};
  //    Object.defineProperty(obj, "foo", {get: undefined});
  //    assertTrue("foo" in obj);
  bool IsJSAccessor(Object* obj) {
    return obj->IsSpecFunction() || obj->IsUndefined();
  }

  DISALLOW_IMPLICIT_CONSTRUCTORS(AccessorPair);
};


class AccessCheckInfo: public Struct {
 public:
  DECL_ACCESSORS(named_callback, Object)
  DECL_ACCESSORS(indexed_callback, Object)
  DECL_ACCESSORS(data, Object)

  static inline AccessCheckInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void AccessCheckInfoPrint() {
    AccessCheckInfoPrint(stdout);
  }
  void AccessCheckInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(AccessCheckInfo)

  static const int kNamedCallbackOffset   = HeapObject::kHeaderSize;
  static const int kIndexedCallbackOffset = kNamedCallbackOffset + kPointerSize;
  static const int kDataOffset = kIndexedCallbackOffset + kPointerSize;
  static const int kSize = kDataOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(AccessCheckInfo);
};


class InterceptorInfo: public Struct {
 public:
  DECL_ACCESSORS(getter, Object)
  DECL_ACCESSORS(setter, Object)
  DECL_ACCESSORS(query, Object)
  DECL_ACCESSORS(deleter, Object)
  DECL_ACCESSORS(enumerator, Object)
  DECL_ACCESSORS(data, Object)

  static inline InterceptorInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void InterceptorInfoPrint() {
    InterceptorInfoPrint(stdout);
  }
  void InterceptorInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(InterceptorInfo)

  static const int kGetterOffset = HeapObject::kHeaderSize;
  static const int kSetterOffset = kGetterOffset + kPointerSize;
  static const int kQueryOffset = kSetterOffset + kPointerSize;
  static const int kDeleterOffset = kQueryOffset + kPointerSize;
  static const int kEnumeratorOffset = kDeleterOffset + kPointerSize;
  static const int kDataOffset = kEnumeratorOffset + kPointerSize;
  static const int kSize = kDataOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(InterceptorInfo);
};


class CallHandlerInfo: public Struct {
 public:
  DECL_ACCESSORS(callback, Object)
  DECL_ACCESSORS(data, Object)

  static inline CallHandlerInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void CallHandlerInfoPrint() {
    CallHandlerInfoPrint(stdout);
  }
  void CallHandlerInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(CallHandlerInfo)

  static const int kCallbackOffset = HeapObject::kHeaderSize;
  static const int kDataOffset = kCallbackOffset + kPointerSize;
  static const int kSize = kDataOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(CallHandlerInfo);
};


class TemplateInfo: public Struct {
 public:
  DECL_ACCESSORS(tag, Object)
  DECL_ACCESSORS(property_list, Object)

  DECLARE_VERIFIER(TemplateInfo)

  static const int kTagOffset          = HeapObject::kHeaderSize;
  static const int kPropertyListOffset = kTagOffset + kPointerSize;
  static const int kHeaderSize         = kPropertyListOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(TemplateInfo);
};


class FunctionTemplateInfo: public TemplateInfo {
 public:
  DECL_ACCESSORS(serial_number, Object)
  DECL_ACCESSORS(call_code, Object)
  DECL_ACCESSORS(property_accessors, Object)
  DECL_ACCESSORS(prototype_template, Object)
  DECL_ACCESSORS(parent_template, Object)
  DECL_ACCESSORS(named_property_handler, Object)
  DECL_ACCESSORS(indexed_property_handler, Object)
  DECL_ACCESSORS(instance_template, Object)
  DECL_ACCESSORS(class_name, Object)
  DECL_ACCESSORS(signature, Object)
  DECL_ACCESSORS(instance_call_handler, Object)
  DECL_ACCESSORS(access_check_info, Object)
  DECL_ACCESSORS(flag, Smi)

  // Following properties use flag bits.
  DECL_BOOLEAN_ACCESSORS(hidden_prototype)
  DECL_BOOLEAN_ACCESSORS(undetectable)
  // If the bit is set, object instances created by this function
  // requires access check.
  DECL_BOOLEAN_ACCESSORS(needs_access_check)
  DECL_BOOLEAN_ACCESSORS(read_only_prototype)

  static inline FunctionTemplateInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void FunctionTemplateInfoPrint() {
    FunctionTemplateInfoPrint(stdout);
  }
  void FunctionTemplateInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(FunctionTemplateInfo)

  static const int kSerialNumberOffset = TemplateInfo::kHeaderSize;
  static const int kCallCodeOffset = kSerialNumberOffset + kPointerSize;
  static const int kPropertyAccessorsOffset = kCallCodeOffset + kPointerSize;
  static const int kPrototypeTemplateOffset =
      kPropertyAccessorsOffset + kPointerSize;
  static const int kParentTemplateOffset =
      kPrototypeTemplateOffset + kPointerSize;
  static const int kNamedPropertyHandlerOffset =
      kParentTemplateOffset + kPointerSize;
  static const int kIndexedPropertyHandlerOffset =
      kNamedPropertyHandlerOffset + kPointerSize;
  static const int kInstanceTemplateOffset =
      kIndexedPropertyHandlerOffset + kPointerSize;
  static const int kClassNameOffset = kInstanceTemplateOffset + kPointerSize;
  static const int kSignatureOffset = kClassNameOffset + kPointerSize;
  static const int kInstanceCallHandlerOffset = kSignatureOffset + kPointerSize;
  static const int kAccessCheckInfoOffset =
      kInstanceCallHandlerOffset + kPointerSize;
  static const int kFlagOffset = kAccessCheckInfoOffset + kPointerSize;
  static const int kSize = kFlagOffset + kPointerSize;

 private:
  // Bit position in the flag, from least significant bit position.
  static const int kHiddenPrototypeBit   = 0;
  static const int kUndetectableBit      = 1;
  static const int kNeedsAccessCheckBit  = 2;
  static const int kReadOnlyPrototypeBit = 3;

  DISALLOW_IMPLICIT_CONSTRUCTORS(FunctionTemplateInfo);
};


class ObjectTemplateInfo: public TemplateInfo {
 public:
  DECL_ACCESSORS(constructor, Object)
  DECL_ACCESSORS(internal_field_count, Object)

  static inline ObjectTemplateInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void ObjectTemplateInfoPrint() {
    ObjectTemplateInfoPrint(stdout);
  }
  void ObjectTemplateInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(ObjectTemplateInfo)

  static const int kConstructorOffset = TemplateInfo::kHeaderSize;
  static const int kInternalFieldCountOffset =
      kConstructorOffset + kPointerSize;
  static const int kSize = kInternalFieldCountOffset + kPointerSize;
};


class SignatureInfo: public Struct {
 public:
  DECL_ACCESSORS(receiver, Object)
  DECL_ACCESSORS(args, Object)

  static inline SignatureInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void SignatureInfoPrint() {
    SignatureInfoPrint(stdout);
  }
  void SignatureInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(SignatureInfo)

  static const int kReceiverOffset = Struct::kHeaderSize;
  static const int kArgsOffset     = kReceiverOffset + kPointerSize;
  static const int kSize           = kArgsOffset + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(SignatureInfo);
};


class TypeSwitchInfo: public Struct {
 public:
  DECL_ACCESSORS(types, Object)

  static inline TypeSwitchInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void TypeSwitchInfoPrint() {
    TypeSwitchInfoPrint(stdout);
  }
  void TypeSwitchInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(TypeSwitchInfo)

  static const int kTypesOffset = Struct::kHeaderSize;
  static const int kSize        = kTypesOffset + kPointerSize;
};


#ifdef ENABLE_DEBUGGER_SUPPORT
// The DebugInfo class holds additional information for a function being
// debugged.
class DebugInfo: public Struct {
 public:
  // The shared function info for the source being debugged.
  DECL_ACCESSORS(shared, SharedFunctionInfo)
  // Code object for the original code.
  DECL_ACCESSORS(original_code, Code)
  // Code object for the patched code. This code object is the code object
  // currently active for the function.
  DECL_ACCESSORS(code, Code)
  // Fixed array holding status information for each active break point.
  DECL_ACCESSORS(break_points, FixedArray)

  // Check if there is a break point at a code position.
  bool HasBreakPoint(int code_position);
  // Get the break point info object for a code position.
  Object* GetBreakPointInfo(int code_position);
  // Clear a break point.
  static void ClearBreakPoint(Handle<DebugInfo> debug_info,
                              int code_position,
                              Handle<Object> break_point_object);
  // Set a break point.
  static void SetBreakPoint(Handle<DebugInfo> debug_info, int code_position,
                            int source_position, int statement_position,
                            Handle<Object> break_point_object);
  // Get the break point objects for a code position.
  Object* GetBreakPointObjects(int code_position);
  // Find the break point info holding this break point object.
  static Object* FindBreakPointInfo(Handle<DebugInfo> debug_info,
                                    Handle<Object> break_point_object);
  // Get the number of break points for this function.
  int GetBreakPointCount();

  static inline DebugInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void DebugInfoPrint() {
    DebugInfoPrint(stdout);
  }
  void DebugInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(DebugInfo)

  static const int kSharedFunctionInfoIndex = Struct::kHeaderSize;
  static const int kOriginalCodeIndex = kSharedFunctionInfoIndex + kPointerSize;
  static const int kPatchedCodeIndex = kOriginalCodeIndex + kPointerSize;
  static const int kActiveBreakPointsCountIndex =
      kPatchedCodeIndex + kPointerSize;
  static const int kBreakPointsStateIndex =
      kActiveBreakPointsCountIndex + kPointerSize;
  static const int kSize = kBreakPointsStateIndex + kPointerSize;

 private:
  static const int kNoBreakPointInfo = -1;

  // Lookup the index in the break_points array for a code position.
  int GetBreakPointInfoIndex(int code_position);

  DISALLOW_IMPLICIT_CONSTRUCTORS(DebugInfo);
};


// The BreakPointInfo class holds information for break points set in a
// function. The DebugInfo object holds a BreakPointInfo object for each code
// position with one or more break points.
class BreakPointInfo: public Struct {
 public:
  // The position in the code for the break point.
  DECL_ACCESSORS(code_position, Smi)
  // The position in the source for the break position.
  DECL_ACCESSORS(source_position, Smi)
  // The position in the source for the last statement before this break
  // position.
  DECL_ACCESSORS(statement_position, Smi)
  // List of related JavaScript break points.
  DECL_ACCESSORS(break_point_objects, Object)

  // Removes a break point.
  static void ClearBreakPoint(Handle<BreakPointInfo> info,
                              Handle<Object> break_point_object);
  // Set a break point.
  static void SetBreakPoint(Handle<BreakPointInfo> info,
                            Handle<Object> break_point_object);
  // Check if break point info has this break point object.
  static bool HasBreakPointObject(Handle<BreakPointInfo> info,
                                  Handle<Object> break_point_object);
  // Get the number of break points for this code position.
  int GetBreakPointCount();

  static inline BreakPointInfo* cast(Object* obj);

#ifdef OBJECT_PRINT
  inline void BreakPointInfoPrint() {
    BreakPointInfoPrint(stdout);
  }
  void BreakPointInfoPrint(FILE* out);
#endif
  DECLARE_VERIFIER(BreakPointInfo)

  static const int kCodePositionIndex = Struct::kHeaderSize;
  static const int kSourcePositionIndex = kCodePositionIndex + kPointerSize;
  static const int kStatementPositionIndex =
      kSourcePositionIndex + kPointerSize;
  static const int kBreakPointObjectsIndex =
      kStatementPositionIndex + kPointerSize;
  static const int kSize = kBreakPointObjectsIndex + kPointerSize;

 private:
  DISALLOW_IMPLICIT_CONSTRUCTORS(BreakPointInfo);
};
#endif  // ENABLE_DEBUGGER_SUPPORT


#undef DECL_BOOLEAN_ACCESSORS
#undef DECL_ACCESSORS
#undef DECLARE_VERIFIER

#define VISITOR_SYNCHRONIZATION_TAGS_LIST(V)                            \
  V(kSymbolTable, "symbol_table", "(Symbols)")                          \
  V(kExternalStringsTable, "external_strings_table", "(External strings)") \
  V(kStrongRootList, "strong_root_list", "(Strong roots)")              \
  V(kSymbol, "symbol", "(Symbol)")                                      \
  V(kBootstrapper, "bootstrapper", "(Bootstrapper)")                    \
  V(kTop, "top", "(Isolate)")                                           \
  V(kRelocatable, "relocatable", "(Relocatable)")                       \
  V(kDebug, "debug", "(Debugger)")                                      \
  V(kCompilationCache, "compilationcache", "(Compilation cache)")       \
  V(kHandleScope, "handlescope", "(Handle scope)")                      \
  V(kBuiltins, "builtins", "(Builtins)")                                \
  V(kGlobalHandles, "globalhandles", "(Global handles)")                \
  V(kThreadManager, "threadmanager", "(Thread manager)")                \
  V(kExtensions, "Extensions", "(Extensions)")

class VisitorSynchronization : public AllStatic {
 public:
#define DECLARE_ENUM(enum_item, ignore1, ignore2) enum_item,
  enum SyncTag {
    VISITOR_SYNCHRONIZATION_TAGS_LIST(DECLARE_ENUM)
    kNumberOfSyncTags
  };
#undef DECLARE_ENUM

  static const char* const kTags[kNumberOfSyncTags];
  static const char* const kTagNames[kNumberOfSyncTags];
};

// Abstract base class for visiting, and optionally modifying, the
// pointers contained in Objects. Used in GC and serialization/deserialization.
class ObjectVisitor BASE_EMBEDDED {
 public:
  virtual ~ObjectVisitor() {}

  // Visits a contiguous arrays of pointers in the half-open range
  // [start, end). Any or all of the values may be modified on return.
  virtual void VisitPointers(Object** start, Object** end) = 0;

  // To allow lazy clearing of inline caches the visitor has
  // a rich interface for iterating over Code objects..

  // Visits a code target in the instruction stream.
  virtual void VisitCodeTarget(RelocInfo* rinfo);

  // Visits a code entry in a JS function.
  virtual void VisitCodeEntry(Address entry_address);

  // Visits a global property cell reference in the instruction stream.
  virtual void VisitGlobalPropertyCell(RelocInfo* rinfo);

  // Visits a runtime entry in the instruction stream.
  virtual void VisitRuntimeEntry(RelocInfo* rinfo) {}

  // Visits the resource of an ASCII or two-byte string.
  virtual void VisitExternalAsciiString(
      v8::String::ExternalAsciiStringResource** resource) {}
  virtual void VisitExternalTwoByteString(
      v8::String::ExternalStringResource** resource) {}

  // Visits a debug call target in the instruction stream.
  virtual void VisitDebugTarget(RelocInfo* rinfo);

  // Handy shorthand for visiting a single pointer.
  virtual void VisitPointer(Object** p) { VisitPointers(p, p + 1); }

  // Visit pointer embedded into a code object.
  virtual void VisitEmbeddedPointer(RelocInfo* rinfo);

  // Visits a contiguous arrays of external references (references to the C++
  // heap) in the half-open range [start, end). Any or all of the values
  // may be modified on return.
  virtual void VisitExternalReferences(Address* start, Address* end) {}

  virtual void VisitExternalReference(RelocInfo* rinfo);

  inline void VisitExternalReference(Address* p) {
    VisitExternalReferences(p, p + 1);
  }

  // Visits a handle that has an embedder-assigned class ID.
  virtual void VisitEmbedderReference(Object** p, uint16_t class_id) {}

  // Intended for serialization/deserialization checking: insert, or
  // check for the presence of, a tag at this position in the stream.
  // Also used for marking up GC roots in heap snapshots.
  virtual void Synchronize(VisitorSynchronization::SyncTag tag) {}
};


class StructBodyDescriptor : public
  FlexibleBodyDescriptor<HeapObject::kHeaderSize> {
 public:
  static inline int SizeOf(Map* map, HeapObject* object) {
    return map->instance_size();
  }
};


// BooleanBit is a helper class for setting and getting a bit in an
// integer or Smi.
class BooleanBit : public AllStatic {
 public:
  static inline bool get(Smi* smi, int bit_position) {
    return get(smi->value(), bit_position);
  }

  static inline bool get(int value, int bit_position) {
    return (value & (1 << bit_position)) != 0;
  }

  static inline Smi* set(Smi* smi, int bit_position, bool v) {
    return Smi::FromInt(set(smi->value(), bit_position, v));
  }

  static inline int set(int value, int bit_position, bool v) {
    if (v) {
      value |= (1 << bit_position);
    } else {
      value &= ~(1 << bit_position);
    }
    return value;
  }
};

} }  // namespace v8::internal

#endif  // V8_OBJECTS_H_