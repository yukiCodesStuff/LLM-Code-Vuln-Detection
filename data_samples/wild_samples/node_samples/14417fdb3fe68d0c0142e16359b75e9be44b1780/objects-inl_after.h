                               InlineCacheHolderFlag holder) {
  // Extra IC state is only allowed for call IC stubs or for store IC
  // stubs.
  ASSERT(extra_ic_state == kNoExtraICState ||
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
      | CacheHolderField::encode(holder);
  return static_cast<Flags>(bits);
}


Code::Flags Code::ComputeMonomorphicFlags(Kind kind,
                                          StubType type,
                                          ExtraICState extra_ic_state,
                                          InlineCacheHolderFlag holder,
                                          int argc) {
  return ComputeFlags(kind, MONOMORPHIC, extra_ic_state, type, argc, holder);
}


Code::Kind Code::ExtractKindFromFlags(Flags flags) {
  return KindField::decode(flags);
}


InlineCacheState Code::ExtractICStateFromFlags(Flags flags) {
  return ICStateField::decode(flags);
}


Code::ExtraICState Code::ExtractExtraICStateFromFlags(Flags flags) {
  return ExtraICStateField::decode(flags);
}


Code::StubType Code::ExtractTypeFromFlags(Flags flags) {
  return TypeField::decode(flags);
}


int Code::ExtractArgumentsCountFromFlags(Flags flags) {
  return (flags & kArgumentsCountMask) >> kArgumentsCountShift;
}


InlineCacheHolderFlag Code::ExtractCacheHolderFromFlags(Flags flags) {
  return CacheHolderField::decode(flags);
}


Code::Flags Code::RemoveTypeFromFlags(Flags flags) {
  int bits = flags & ~TypeField::kMask;
  return static_cast<Flags>(bits);
}


Code* Code::GetCodeFromTargetAddress(Address address) {
  HeapObject* code = HeapObject::FromAddress(address - Code::kHeaderSize);
  // GetCodeFromTargetAddress might be called when marking objects during mark
  // sweep. reinterpret_cast is therefore used instead of the more appropriate
  // Code::cast. Code::cast does not work when the object's map is
  // marked.
  Code* result = reinterpret_cast<Code*>(code);
  return result;
}


Object* Code::GetObjectFromEntryAddress(Address location_of_address) {
  return HeapObject::
      FromAddress(Memory::Address_at(location_of_address) - Code::kHeaderSize);
}


Object* Map::prototype() {
  return READ_FIELD(this, kPrototypeOffset);
}


void Map::set_prototype(Object* value, WriteBarrierMode mode) {
  ASSERT(value->IsNull() || value->IsJSReceiver());
  WRITE_FIELD(this, kPrototypeOffset, value);
  CONDITIONAL_WRITE_BARRIER(GetHeap(), this, kPrototypeOffset, value, mode);
}


// If the descriptor is using the empty transition array, install a new empty
// transition array that will have place for an element transition.
static MaybeObject* EnsureHasTransitionArray(Map* map) {
  TransitionArray* transitions;
  MaybeObject* maybe_transitions;
  if (!map->HasTransitionArray()) {
    maybe_transitions = TransitionArray::Allocate(0);
    if (!maybe_transitions->To(&transitions)) return maybe_transitions;
    transitions->set_back_pointer_storage(map->GetBackPointer());
  } else if (!map->transitions()->IsFullTransitionArray()) {
    maybe_transitions = map->transitions()->ExtendToFullTransitionArray();
    if (!maybe_transitions->To(&transitions)) return maybe_transitions;
  } else {
    return map;
  }
  map->set_transitions(transitions);
  return transitions;
}


void Map::InitializeDescriptors(DescriptorArray* descriptors) {
  int len = descriptors->number_of_descriptors();
#ifdef DEBUG
  ASSERT(len <= DescriptorArray::kMaxNumberOfDescriptors);

  bool used_indices[DescriptorArray::kMaxNumberOfDescriptors];
  for (int i = 0; i < len; ++i) used_indices[i] = false;

  // Ensure that all enumeration indexes between 1 and length occur uniquely in
  // the descriptor array.
  for (int i = 0; i < len; ++i) {
    int enum_index = descriptors->GetDetails(i).descriptor_index() -
                     PropertyDetails::kInitialIndex;
    ASSERT(0 <= enum_index && enum_index < len);
    ASSERT(!used_indices[enum_index]);
    used_indices[enum_index] = true;
  }
#endif

  set_instance_descriptors(descriptors);
  SetNumberOfOwnDescriptors(len);
}


ACCESSORS(Map, instance_descriptors, DescriptorArray, kDescriptorsOffset)
SMI_ACCESSORS(Map, bit_field3, kBitField3Offset)


void Map::ClearTransitions(Heap* heap, WriteBarrierMode mode) {
  Object* back_pointer = GetBackPointer();

  if (Heap::ShouldZapGarbage() && HasTransitionArray()) {
    ZapTransitions();
  }

  WRITE_FIELD(this, kTransitionsOrBackPointerOffset, back_pointer);
  CONDITIONAL_WRITE_BARRIER(
      heap, this, kTransitionsOrBackPointerOffset, back_pointer, mode);
}


void Map::AppendDescriptor(Descriptor* desc,
                           const DescriptorArray::WhitenessWitness& witness) {
  DescriptorArray* descriptors = instance_descriptors();
  int number_of_own_descriptors = NumberOfOwnDescriptors();
  ASSERT(descriptors->number_of_descriptors() == number_of_own_descriptors);
  descriptors->Append(desc, witness);
  SetNumberOfOwnDescriptors(number_of_own_descriptors + 1);
}


Object* Map::GetBackPointer() {
  Object* object = READ_FIELD(this, kTransitionsOrBackPointerOffset);
  if (object->IsDescriptorArray()) {
    return TransitionArray::cast(object)->back_pointer_storage();
  } else {
    ASSERT(object->IsMap() || object->IsUndefined());
    return object;
  }
}


bool Map::HasElementsTransition() {
  return HasTransitionArray() && transitions()->HasElementsTransition();
}


bool Map::HasTransitionArray() {
  Object* object = READ_FIELD(this, kTransitionsOrBackPointerOffset);
  return object->IsTransitionArray();
}


Map* Map::elements_transition_map() {
  return transitions()->elements_transition();
}


bool Map::CanHaveMoreTransitions() {
  if (!HasTransitionArray()) return true;
  return FixedArray::SizeFor(transitions()->length() +
                             TransitionArray::kTransitionSize)
      <= Page::kMaxNonCodeHeapObjectSize;
}


MaybeObject* Map::AddTransition(String* key,
                                Map* target,
                                SimpleTransitionFlag flag) {
  if (HasTransitionArray()) return transitions()->CopyInsert(key, target);
  return TransitionArray::NewWith(flag, key, target, GetBackPointer());
}


void Map::SetTransition(int transition_index, Map* target) {
  transitions()->SetTarget(transition_index, target);
}


Map* Map::GetTransition(int transition_index) {
  return transitions()->GetTarget(transition_index);
}


MaybeObject* Map::set_elements_transition_map(Map* transitioned_map) {
  MaybeObject* allow_elements = EnsureHasTransitionArray(this);
  if (allow_elements->IsFailure()) return allow_elements;
  transitions()->set_elements_transition(transitioned_map);
  return this;
}


FixedArray* Map::GetPrototypeTransitions() {
  if (!HasTransitionArray()) return GetHeap()->empty_fixed_array();
  if (!transitions()->HasPrototypeTransitions()) {
    return GetHeap()->empty_fixed_array();
  }
  return transitions()->GetPrototypeTransitions();
}


MaybeObject* Map::SetPrototypeTransitions(FixedArray* proto_transitions) {
  MaybeObject* allow_prototype = EnsureHasTransitionArray(this);
  if (allow_prototype->IsFailure()) return allow_prototype;
#ifdef DEBUG
  if (HasPrototypeTransitions()) {
    ASSERT(GetPrototypeTransitions() != proto_transitions);
    ZapPrototypeTransitions();
  }
#endif
  transitions()->SetPrototypeTransitions(proto_transitions);
  return this;
}


bool Map::HasPrototypeTransitions() {
  return HasTransitionArray() && transitions()->HasPrototypeTransitions();
}


TransitionArray* Map::transitions() {
  ASSERT(HasTransitionArray());
  Object* object = READ_FIELD(this, kTransitionsOrBackPointerOffset);
  return TransitionArray::cast(object);
}


void Map::set_transitions(TransitionArray* transition_array,
                          WriteBarrierMode mode) {
  // In release mode, only run this code if verify_heap is on.
  if (Heap::ShouldZapGarbage() && HasTransitionArray()) {
    CHECK(transitions() != transition_array);
    ZapTransitions();
  }

  WRITE_FIELD(this, kTransitionsOrBackPointerOffset, transition_array);
  CONDITIONAL_WRITE_BARRIER(
      GetHeap(), this, kTransitionsOrBackPointerOffset, transition_array, mode);
}


void Map::init_back_pointer(Object* undefined) {
  ASSERT(undefined->IsUndefined());
  WRITE_FIELD(this, kTransitionsOrBackPointerOffset, undefined);
}


void Map::SetBackPointer(Object* value, WriteBarrierMode mode) {
  ASSERT(instance_type() >= FIRST_JS_RECEIVER_TYPE);
  ASSERT((value->IsUndefined() && GetBackPointer()->IsMap()) ||
         (value->IsMap() && GetBackPointer()->IsUndefined()));
  Object* object = READ_FIELD(this, kTransitionsOrBackPointerOffset);
  if (object->IsTransitionArray()) {
    TransitionArray::cast(object)->set_back_pointer_storage(value);
  } else {
    WRITE_FIELD(this, kTransitionsOrBackPointerOffset, value);
    CONDITIONAL_WRITE_BARRIER(
        GetHeap(), this, kTransitionsOrBackPointerOffset, value, mode);
  }
}


// Can either be Smi (no transitions), normal transition array, or a transition
// array with the header overwritten as a Smi (thus iterating).
TransitionArray* Map::unchecked_transition_array() {
  Object* object = *HeapObject::RawField(this,
                                         Map::kTransitionsOrBackPointerOffset);
  TransitionArray* transition_array = static_cast<TransitionArray*>(object);
  return transition_array;
}


HeapObject* Map::UncheckedPrototypeTransitions() {
  ASSERT(HasTransitionArray());
  ASSERT(unchecked_transition_array()->HasPrototypeTransitions());
  return unchecked_transition_array()->UncheckedPrototypeTransitions();
}


ACCESSORS(Map, code_cache, Object, kCodeCacheOffset)
ACCESSORS(Map, constructor, Object, kConstructorOffset)

ACCESSORS(JSFunction, shared, SharedFunctionInfo, kSharedFunctionInfoOffset)
ACCESSORS(JSFunction, literals_or_bindings, FixedArray, kLiteralsOffset)
ACCESSORS(JSFunction, next_function_link, Object, kNextFunctionLinkOffset)

ACCESSORS(GlobalObject, builtins, JSBuiltinsObject, kBuiltinsOffset)
ACCESSORS(GlobalObject, native_context, Context, kNativeContextOffset)
ACCESSORS(GlobalObject, global_context, Context, kGlobalContextOffset)
ACCESSORS(GlobalObject, global_receiver, JSObject, kGlobalReceiverOffset)

ACCESSORS(JSGlobalProxy, native_context, Object, kNativeContextOffset)

ACCESSORS(AccessorInfo, getter, Object, kGetterOffset)
ACCESSORS(AccessorInfo, setter, Object, kSetterOffset)
ACCESSORS(AccessorInfo, data, Object, kDataOffset)
ACCESSORS(AccessorInfo, name, Object, kNameOffset)
ACCESSORS_TO_SMI(AccessorInfo, flag, kFlagOffset)
ACCESSORS(AccessorInfo, expected_receiver_type, Object,
          kExpectedReceiverTypeOffset)

ACCESSORS(AccessorPair, getter, Object, kGetterOffset)
ACCESSORS(AccessorPair, setter, Object, kSetterOffset)

ACCESSORS(AccessCheckInfo, named_callback, Object, kNamedCallbackOffset)
ACCESSORS(AccessCheckInfo, indexed_callback, Object, kIndexedCallbackOffset)
ACCESSORS(AccessCheckInfo, data, Object, kDataOffset)

ACCESSORS(InterceptorInfo, getter, Object, kGetterOffset)
ACCESSORS(InterceptorInfo, setter, Object, kSetterOffset)
ACCESSORS(InterceptorInfo, query, Object, kQueryOffset)
ACCESSORS(InterceptorInfo, deleter, Object, kDeleterOffset)
ACCESSORS(InterceptorInfo, enumerator, Object, kEnumeratorOffset)
ACCESSORS(InterceptorInfo, data, Object, kDataOffset)

ACCESSORS(CallHandlerInfo, callback, Object, kCallbackOffset)
ACCESSORS(CallHandlerInfo, data, Object, kDataOffset)

ACCESSORS(TemplateInfo, tag, Object, kTagOffset)
ACCESSORS(TemplateInfo, property_list, Object, kPropertyListOffset)

ACCESSORS(FunctionTemplateInfo, serial_number, Object, kSerialNumberOffset)
ACCESSORS(FunctionTemplateInfo, call_code, Object, kCallCodeOffset)
ACCESSORS(FunctionTemplateInfo, property_accessors, Object,
          kPropertyAccessorsOffset)
ACCESSORS(FunctionTemplateInfo, prototype_template, Object,
          kPrototypeTemplateOffset)
ACCESSORS(FunctionTemplateInfo, parent_template, Object, kParentTemplateOffset)
ACCESSORS(FunctionTemplateInfo, named_property_handler, Object,
          kNamedPropertyHandlerOffset)
ACCESSORS(FunctionTemplateInfo, indexed_property_handler, Object,
          kIndexedPropertyHandlerOffset)
ACCESSORS(FunctionTemplateInfo, instance_template, Object,
          kInstanceTemplateOffset)
ACCESSORS(FunctionTemplateInfo, class_name, Object, kClassNameOffset)
ACCESSORS(FunctionTemplateInfo, signature, Object, kSignatureOffset)
ACCESSORS(FunctionTemplateInfo, instance_call_handler, Object,
          kInstanceCallHandlerOffset)
ACCESSORS(FunctionTemplateInfo, access_check_info, Object,
          kAccessCheckInfoOffset)
ACCESSORS_TO_SMI(FunctionTemplateInfo, flag, kFlagOffset)

ACCESSORS(ObjectTemplateInfo, constructor, Object, kConstructorOffset)
ACCESSORS(ObjectTemplateInfo, internal_field_count, Object,
          kInternalFieldCountOffset)

ACCESSORS(SignatureInfo, receiver, Object, kReceiverOffset)
ACCESSORS(SignatureInfo, args, Object, kArgsOffset)

ACCESSORS(TypeSwitchInfo, types, Object, kTypesOffset)

ACCESSORS(Script, source, Object, kSourceOffset)
ACCESSORS(Script, name, Object, kNameOffset)
ACCESSORS(Script, id, Object, kIdOffset)
ACCESSORS_TO_SMI(Script, line_offset, kLineOffsetOffset)
ACCESSORS_TO_SMI(Script, column_offset, kColumnOffsetOffset)
ACCESSORS(Script, data, Object, kDataOffset)
ACCESSORS(Script, context_data, Object, kContextOffset)
ACCESSORS(Script, wrapper, Foreign, kWrapperOffset)
ACCESSORS_TO_SMI(Script, type, kTypeOffset)
ACCESSORS_TO_SMI(Script, compilation_type, kCompilationTypeOffset)
ACCESSORS_TO_SMI(Script, compilation_state, kCompilationStateOffset)
ACCESSORS(Script, line_ends, Object, kLineEndsOffset)
ACCESSORS(Script, eval_from_shared, Object, kEvalFromSharedOffset)
ACCESSORS_TO_SMI(Script, eval_from_instructions_offset,
                 kEvalFrominstructionsOffsetOffset)

#ifdef ENABLE_DEBUGGER_SUPPORT
ACCESSORS(DebugInfo, shared, SharedFunctionInfo, kSharedFunctionInfoIndex)
ACCESSORS(DebugInfo, original_code, Code, kOriginalCodeIndex)
ACCESSORS(DebugInfo, code, Code, kPatchedCodeIndex)
ACCESSORS(DebugInfo, break_points, FixedArray, kBreakPointsStateIndex)

ACCESSORS_TO_SMI(BreakPointInfo, code_position, kCodePositionIndex)
ACCESSORS_TO_SMI(BreakPointInfo, source_position, kSourcePositionIndex)
ACCESSORS_TO_SMI(BreakPointInfo, statement_position, kStatementPositionIndex)
ACCESSORS(BreakPointInfo, break_point_objects, Object, kBreakPointObjectsIndex)
#endif

ACCESSORS(SharedFunctionInfo, name, Object, kNameOffset)
ACCESSORS(SharedFunctionInfo, optimized_code_map, Object,
                 kOptimizedCodeMapOffset)
ACCESSORS(SharedFunctionInfo, construct_stub, Code, kConstructStubOffset)
ACCESSORS(SharedFunctionInfo, initial_map, Object, kInitialMapOffset)
ACCESSORS(SharedFunctionInfo, instance_class_name, Object,
          kInstanceClassNameOffset)
ACCESSORS(SharedFunctionInfo, function_data, Object, kFunctionDataOffset)
ACCESSORS(SharedFunctionInfo, script, Object, kScriptOffset)
ACCESSORS(SharedFunctionInfo, debug_info, Object, kDebugInfoOffset)
ACCESSORS(SharedFunctionInfo, inferred_name, String, kInferredNameOffset)
ACCESSORS(SharedFunctionInfo, this_property_assignments, Object,
          kThisPropertyAssignmentsOffset)
SMI_ACCESSORS(SharedFunctionInfo, ast_node_count, kAstNodeCountOffset)


BOOL_ACCESSORS(FunctionTemplateInfo, flag, hidden_prototype,
               kHiddenPrototypeBit)
BOOL_ACCESSORS(FunctionTemplateInfo, flag, undetectable, kUndetectableBit)
BOOL_ACCESSORS(FunctionTemplateInfo, flag, needs_access_check,
               kNeedsAccessCheckBit)
BOOL_ACCESSORS(FunctionTemplateInfo, flag, read_only_prototype,
               kReadOnlyPrototypeBit)
BOOL_ACCESSORS(SharedFunctionInfo, start_position_and_type, is_expression,
               kIsExpressionBit)
BOOL_ACCESSORS(SharedFunctionInfo, start_position_and_type, is_toplevel,
               kIsTopLevelBit)
BOOL_GETTER(SharedFunctionInfo,
            compiler_hints,
            has_only_simple_this_property_assignments,
            kHasOnlySimpleThisPropertyAssignments)
BOOL_ACCESSORS(SharedFunctionInfo,
               compiler_hints,
               allows_lazy_compilation,
               kAllowLazyCompilation)
BOOL_ACCESSORS(SharedFunctionInfo,
               compiler_hints,
               allows_lazy_compilation_without_context,
               kAllowLazyCompilationWithoutContext)
BOOL_ACCESSORS(SharedFunctionInfo,
               compiler_hints,
               uses_arguments,
               kUsesArguments)
BOOL_ACCESSORS(SharedFunctionInfo,
               compiler_hints,
               has_duplicate_parameters,
               kHasDuplicateParameters)


#if V8_HOST_ARCH_32_BIT
SMI_ACCESSORS(SharedFunctionInfo, length, kLengthOffset)
SMI_ACCESSORS(SharedFunctionInfo, formal_parameter_count,
              kFormalParameterCountOffset)
SMI_ACCESSORS(SharedFunctionInfo, expected_nof_properties,
              kExpectedNofPropertiesOffset)
SMI_ACCESSORS(SharedFunctionInfo, num_literals, kNumLiteralsOffset)
SMI_ACCESSORS(SharedFunctionInfo, start_position_and_type,
              kStartPositionAndTypeOffset)
SMI_ACCESSORS(SharedFunctionInfo, end_position, kEndPositionOffset)
SMI_ACCESSORS(SharedFunctionInfo, function_token_position,
              kFunctionTokenPositionOffset)
SMI_ACCESSORS(SharedFunctionInfo, compiler_hints,
              kCompilerHintsOffset)
SMI_ACCESSORS(SharedFunctionInfo, this_property_assignments_count,
              kThisPropertyAssignmentsCountOffset)
SMI_ACCESSORS(SharedFunctionInfo, opt_count, kOptCountOffset)
SMI_ACCESSORS(SharedFunctionInfo, counters, kCountersOffset)
SMI_ACCESSORS(SharedFunctionInfo,
              stress_deopt_counter,
              kStressDeoptCounterOffset)
#else

#define PSEUDO_SMI_ACCESSORS_LO(holder, name, offset)             \
  STATIC_ASSERT(holder::offset % kPointerSize == 0);              \
  int holder::name() {                                            \
    int value = READ_INT_FIELD(this, offset);                     \
    ASSERT(kHeapObjectTag == 1);                                  \
    ASSERT((value & kHeapObjectTag) == 0);                        \
    return value >> 1;                                            \
  }                                                               \
  void holder::set_##name(int value) {                            \
    ASSERT(kHeapObjectTag == 1);                                  \
    ASSERT((value & 0xC0000000) == 0xC0000000 ||                  \
           (value & 0xC0000000) == 0x000000000);                  \
    WRITE_INT_FIELD(this,                                         \
                    offset,                                       \
                    (value << 1) & ~kHeapObjectTag);              \
  }

#define PSEUDO_SMI_ACCESSORS_HI(holder, name, offset)             \
  STATIC_ASSERT(holder::offset % kPointerSize == kIntSize);       \
  INT_ACCESSORS(holder, name, offset)


PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo, length, kLengthOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo,
                        formal_parameter_count,
                        kFormalParameterCountOffset)

PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo,
                        expected_nof_properties,
                        kExpectedNofPropertiesOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo, num_literals, kNumLiteralsOffset)

PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo, end_position, kEndPositionOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo,
                        start_position_and_type,
                        kStartPositionAndTypeOffset)

PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo,
                        function_token_position,
                        kFunctionTokenPositionOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo,
                        compiler_hints,
                        kCompilerHintsOffset)

PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo,
                        this_property_assignments_count,
                        kThisPropertyAssignmentsCountOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo, opt_count, kOptCountOffset)

PSEUDO_SMI_ACCESSORS_LO(SharedFunctionInfo, counters, kCountersOffset)
PSEUDO_SMI_ACCESSORS_HI(SharedFunctionInfo,
                        stress_deopt_counter,
                        kStressDeoptCounterOffset)
#endif


int SharedFunctionInfo::construction_count() {
  return READ_BYTE_FIELD(this, kConstructionCountOffset);
}


void SharedFunctionInfo::set_construction_count(int value) {
  ASSERT(0 <= value && value < 256);
  WRITE_BYTE_FIELD(this, kConstructionCountOffset, static_cast<byte>(value));
}


BOOL_ACCESSORS(SharedFunctionInfo,
               compiler_hints,
               live_objects_may_exist,
               kLiveObjectsMayExist)


bool SharedFunctionInfo::IsInobjectSlackTrackingInProgress() {
  return initial_map() != GetHeap()->undefined_value();
}


BOOL_GETTER(SharedFunctionInfo,
            compiler_hints,
            optimization_disabled,
            kOptimizationDisabled)


void SharedFunctionInfo::set_optimization_disabled(bool disable) {
  set_compiler_hints(BooleanBit::set(compiler_hints(),
                                     kOptimizationDisabled,
                                     disable));
  // If disabling optimizations we reflect that in the code object so
  // it will not be counted as optimizable code.
  if ((code()->kind() == Code::FUNCTION) && disable) {
    code()->set_optimizable(false);
  }
}


int SharedFunctionInfo::profiler_ticks() {
  if (code()->kind() != Code::FUNCTION) return 0;
  return code()->profiler_ticks();
}


LanguageMode SharedFunctionInfo::language_mode() {
  int hints = compiler_hints();
  if (BooleanBit::get(hints, kExtendedModeFunction)) {
    ASSERT(BooleanBit::get(hints, kStrictModeFunction));
    return EXTENDED_MODE;
  }
  return BooleanBit::get(hints, kStrictModeFunction)
      ? STRICT_MODE : CLASSIC_MODE;
}


void SharedFunctionInfo::set_language_mode(LanguageMode language_mode) {
  // We only allow language mode transitions that go set the same language mode
  // again or go up in the chain:
  //   CLASSIC_MODE -> STRICT_MODE -> EXTENDED_MODE.
  ASSERT(this->language_mode() == CLASSIC_MODE ||
         this->language_mode() == language_mode ||
         language_mode == EXTENDED_MODE);
  int hints = compiler_hints();
  hints = BooleanBit::set(
      hints, kStrictModeFunction, language_mode != CLASSIC_MODE);
  hints = BooleanBit::set(
      hints, kExtendedModeFunction, language_mode == EXTENDED_MODE);
  set_compiler_hints(hints);
}


bool SharedFunctionInfo::is_classic_mode() {
  return !BooleanBit::get(compiler_hints(), kStrictModeFunction);
}

BOOL_GETTER(SharedFunctionInfo, compiler_hints, is_extended_mode,
            kExtendedModeFunction)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, native, kNative)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints,
               name_should_print_as_anonymous,
               kNameShouldPrintAsAnonymous)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, bound, kBoundFunction)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, is_anonymous, kIsAnonymous)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, is_function, kIsFunction)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, dont_optimize,
               kDontOptimize)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, dont_inline, kDontInline)
BOOL_ACCESSORS(SharedFunctionInfo, compiler_hints, dont_cache, kDontCache)

void SharedFunctionInfo::BeforeVisitingPointers() {
  if (IsInobjectSlackTrackingInProgress()) DetachInitialMap();

  // Flush optimized code map on major GC.
  // Note: we may experiment with rebuilding it or retaining entries
  // which should survive as we iterate through optimized functions
  // anyway.
  set_optimized_code_map(Smi::FromInt(0));
}


ACCESSORS(CodeCache, default_cache, FixedArray, kDefaultCacheOffset)
ACCESSORS(CodeCache, normal_type_cache, Object, kNormalTypeCacheOffset)

ACCESSORS(PolymorphicCodeCache, cache, Object, kCacheOffset)

bool Script::HasValidSource() {
  Object* src = this->source();
  if (!src->IsString()) return true;
  String* src_str = String::cast(src);
  if (!StringShape(src_str).IsExternal()) return true;
  if (src_str->IsAsciiRepresentation()) {
    return ExternalAsciiString::cast(src)->resource() != NULL;
  } else if (src_str->IsTwoByteRepresentation()) {
    return ExternalTwoByteString::cast(src)->resource() != NULL;
  }
  return true;
}


void SharedFunctionInfo::DontAdaptArguments() {
  ASSERT(code()->kind() == Code::BUILTIN);
  set_formal_parameter_count(kDontAdaptArgumentsSentinel);
}


int SharedFunctionInfo::start_position() {
  return start_position_and_type() >> kStartPositionShift;
}


void SharedFunctionInfo::set_start_position(int start_position) {
  set_start_position_and_type((start_position << kStartPositionShift)
    | (start_position_and_type() & ~kStartPositionMask));
}


Code* SharedFunctionInfo::code() {
  return Code::cast(READ_FIELD(this, kCodeOffset));
}


Code* SharedFunctionInfo::unchecked_code() {
  return reinterpret_cast<Code*>(READ_FIELD(this, kCodeOffset));
}


void SharedFunctionInfo::set_code(Code* value, WriteBarrierMode mode) {
  WRITE_FIELD(this, kCodeOffset, value);
  CONDITIONAL_WRITE_BARRIER(value->GetHeap(), this, kCodeOffset, value, mode);
}


ScopeInfo* SharedFunctionInfo::scope_info() {
  return reinterpret_cast<ScopeInfo*>(READ_FIELD(this, kScopeInfoOffset));
}


void SharedFunctionInfo::set_scope_info(ScopeInfo* value,
                                        WriteBarrierMode mode) {
  WRITE_FIELD(this, kScopeInfoOffset, reinterpret_cast<Object*>(value));
  CONDITIONAL_WRITE_BARRIER(GetHeap(),
                            this,
                            kScopeInfoOffset,
                            reinterpret_cast<Object*>(value),
                            mode);
}


bool SharedFunctionInfo::is_compiled() {
  return code() !=
      Isolate::Current()->builtins()->builtin(Builtins::kLazyCompile);
}


bool SharedFunctionInfo::IsApiFunction() {
  return function_data()->IsFunctionTemplateInfo();
}


FunctionTemplateInfo* SharedFunctionInfo::get_api_func_data() {
  ASSERT(IsApiFunction());
  return FunctionTemplateInfo::cast(function_data());
}


bool SharedFunctionInfo::HasBuiltinFunctionId() {
  return function_data()->IsSmi();
}


BuiltinFunctionId SharedFunctionInfo::builtin_function_id() {
  ASSERT(HasBuiltinFunctionId());
  return static_cast<BuiltinFunctionId>(Smi::cast(function_data())->value());
}


int SharedFunctionInfo::code_age() {
  return (compiler_hints() >> kCodeAgeShift) & kCodeAgeMask;
}


void SharedFunctionInfo::set_code_age(int code_age) {
  int hints = compiler_hints() & ~(kCodeAgeMask << kCodeAgeShift);
  set_compiler_hints(hints | ((code_age & kCodeAgeMask) << kCodeAgeShift));
}


int SharedFunctionInfo::ic_age() {
  return ICAgeBits::decode(counters());
}


void SharedFunctionInfo::set_ic_age(int ic_age) {
  set_counters(ICAgeBits::update(counters(), ic_age));
}


int SharedFunctionInfo::deopt_count() {
  return DeoptCountBits::decode(counters());
}


void SharedFunctionInfo::set_deopt_count(int deopt_count) {
  set_counters(DeoptCountBits::update(counters(), deopt_count));
}


void SharedFunctionInfo::increment_deopt_count() {
  int value = counters();
  int deopt_count = DeoptCountBits::decode(value);
  deopt_count = (deopt_count + 1) & DeoptCountBits::kMax;
  set_counters(DeoptCountBits::update(value, deopt_count));
}


int SharedFunctionInfo::opt_reenable_tries() {
  return OptReenableTriesBits::decode(counters());
}


void SharedFunctionInfo::set_opt_reenable_tries(int tries) {
  set_counters(OptReenableTriesBits::update(counters(), tries));
}


bool SharedFunctionInfo::has_deoptimization_support() {
  Code* code = this->code();
  return code->kind() == Code::FUNCTION && code->has_deoptimization_support();
}


void SharedFunctionInfo::TryReenableOptimization() {
  int tries = opt_reenable_tries();
  set_opt_reenable_tries((tries + 1) & OptReenableTriesBits::kMax);
  // We reenable optimization whenever the number of tries is a large
  // enough power of 2.
  if (tries >= 16 && (((tries - 1) & tries) == 0)) {
    set_optimization_disabled(false);
    set_opt_count(0);
    set_deopt_count(0);
    code()->set_optimizable(true);
  }
}


bool JSFunction::IsBuiltin() {
  return context()->global_object()->IsJSBuiltinsObject();
}


bool JSFunction::NeedsArgumentsAdaption() {
  return shared()->formal_parameter_count() !=
      SharedFunctionInfo::kDontAdaptArgumentsSentinel;
}


bool JSFunction::IsOptimized() {
  return code()->kind() == Code::OPTIMIZED_FUNCTION;
}


bool JSFunction::IsOptimizable() {
  return code()->kind() == Code::FUNCTION && code()->optimizable();
}


bool JSFunction::IsMarkedForLazyRecompilation() {
  return code() == GetIsolate()->builtins()->builtin(Builtins::kLazyRecompile);
}


bool JSFunction::IsMarkedForParallelRecompilation() {
  return code() ==
      GetIsolate()->builtins()->builtin(Builtins::kParallelRecompile);
}


bool JSFunction::IsInRecompileQueue() {
  return code() == GetIsolate()->builtins()->builtin(
      Builtins::kInRecompileQueue);
}


Code* JSFunction::code() {
  return Code::cast(unchecked_code());
}


Code* JSFunction::unchecked_code() {
  return reinterpret_cast<Code*>(
      Code::GetObjectFromEntryAddress(FIELD_ADDR(this, kCodeEntryOffset)));
}


void JSFunction::set_code(Code* value) {
  ASSERT(!HEAP->InNewSpace(value));
  Address entry = value->entry();
  WRITE_INTPTR_FIELD(this, kCodeEntryOffset, reinterpret_cast<intptr_t>(entry));
  GetHeap()->incremental_marking()->RecordWriteOfCodeEntry(
      this,
      HeapObject::RawField(this, kCodeEntryOffset),
      value);
}


void JSFunction::ReplaceCode(Code* code) {
  bool was_optimized = IsOptimized();
  bool is_optimized = code->kind() == Code::OPTIMIZED_FUNCTION;

  set_code(code);

  // Add/remove the function from the list of optimized functions for this
  // context based on the state change.
  if (!was_optimized && is_optimized) {
    context()->native_context()->AddOptimizedFunction(this);
  }
  if (was_optimized && !is_optimized) {
    context()->native_context()->RemoveOptimizedFunction(this);
  }
}


Context* JSFunction::context() {
  return Context::cast(READ_FIELD(this, kContextOffset));
}


Object* JSFunction::unchecked_context() {
  return READ_FIELD(this, kContextOffset);
}


SharedFunctionInfo* JSFunction::unchecked_shared() {
  return reinterpret_cast<SharedFunctionInfo*>(
      READ_FIELD(this, kSharedFunctionInfoOffset));
}


void JSFunction::set_context(Object* value) {
  ASSERT(value->IsUndefined() || value->IsContext());
  WRITE_FIELD(this, kContextOffset, value);
  WRITE_BARRIER(GetHeap(), this, kContextOffset, value);
}

ACCESSORS(JSFunction, prototype_or_initial_map, Object,
          kPrototypeOrInitialMapOffset)


Map* JSFunction::initial_map() {
  return Map::cast(prototype_or_initial_map());
}


void JSFunction::set_initial_map(Map* value) {
  set_prototype_or_initial_map(value);
}


bool JSFunction::has_initial_map() {
  return prototype_or_initial_map()->IsMap();
}


bool JSFunction::has_instance_prototype() {
  return has_initial_map() || !prototype_or_initial_map()->IsTheHole();
}


bool JSFunction::has_prototype() {
  return map()->has_non_instance_prototype() || has_instance_prototype();
}


Object* JSFunction::instance_prototype() {
  ASSERT(has_instance_prototype());
  if (has_initial_map()) return initial_map()->prototype();
  // When there is no initial map and the prototype is a JSObject, the
  // initial map field is used for the prototype field.
  return prototype_or_initial_map();
}


Object* JSFunction::prototype() {
  ASSERT(has_prototype());
  // If the function's prototype property has been set to a non-JSObject
  // value, that value is stored in the constructor field of the map.
  if (map()->has_non_instance_prototype()) return map()->constructor();
  return instance_prototype();
}


bool JSFunction::should_have_prototype() {
  return map()->function_with_prototype();
}


bool JSFunction::is_compiled() {
  return code() != GetIsolate()->builtins()->builtin(Builtins::kLazyCompile);
}


FixedArray* JSFunction::literals() {
  ASSERT(!shared()->bound());
  return literals_or_bindings();
}


void JSFunction::set_literals(FixedArray* literals) {
  ASSERT(!shared()->bound());
  set_literals_or_bindings(literals);
}


FixedArray* JSFunction::function_bindings() {
  ASSERT(shared()->bound());
  return literals_or_bindings();
}


void JSFunction::set_function_bindings(FixedArray* bindings) {
  ASSERT(shared()->bound());
  // Bound function literal may be initialized to the empty fixed array
  // before the bindings are set.
  ASSERT(bindings == GetHeap()->empty_fixed_array() ||
         bindings->map() == GetHeap()->fixed_cow_array_map());
  set_literals_or_bindings(bindings);
}


int JSFunction::NumberOfLiterals() {
  ASSERT(!shared()->bound());
  return literals()->length();
}


Object* JSBuiltinsObject::javascript_builtin(Builtins::JavaScript id) {
  ASSERT(id < kJSBuiltinsCount);  // id is unsigned.
  return READ_FIELD(this, OffsetOfFunctionWithId(id));
}


void JSBuiltinsObject::set_javascript_builtin(Builtins::JavaScript id,
                                              Object* value) {
  ASSERT(id < kJSBuiltinsCount);  // id is unsigned.
  WRITE_FIELD(this, OffsetOfFunctionWithId(id), value);
  WRITE_BARRIER(GetHeap(), this, OffsetOfFunctionWithId(id), value);
}


Code* JSBuiltinsObject::javascript_builtin_code(Builtins::JavaScript id) {
  ASSERT(id < kJSBuiltinsCount);  // id is unsigned.
  return Code::cast(READ_FIELD(this, OffsetOfCodeWithId(id)));
}


void JSBuiltinsObject::set_javascript_builtin_code(Builtins::JavaScript id,
                                                   Code* value) {
  ASSERT(id < kJSBuiltinsCount);  // id is unsigned.
  WRITE_FIELD(this, OffsetOfCodeWithId(id), value);
  ASSERT(!HEAP->InNewSpace(value));
}


ACCESSORS(JSProxy, handler, Object, kHandlerOffset)
ACCESSORS(JSProxy, hash, Object, kHashOffset)
ACCESSORS(JSFunctionProxy, call_trap, Object, kCallTrapOffset)
ACCESSORS(JSFunctionProxy, construct_trap, Object, kConstructTrapOffset)


void JSProxy::InitializeBody(int object_size, Object* value) {
  ASSERT(!value->IsHeapObject() || !GetHeap()->InNewSpace(value));
  for (int offset = kHeaderSize; offset < object_size; offset += kPointerSize) {
    WRITE_FIELD(this, offset, value);
  }
}


ACCESSORS(JSSet, table, Object, kTableOffset)
ACCESSORS(JSMap, table, Object, kTableOffset)
ACCESSORS(JSWeakMap, table, Object, kTableOffset)
ACCESSORS(JSWeakMap, next, Object, kNextOffset)


Address Foreign::foreign_address() {
  return AddressFrom<Address>(READ_INTPTR_FIELD(this, kForeignAddressOffset));
}


void Foreign::set_foreign_address(Address value) {
  WRITE_INTPTR_FIELD(this, kForeignAddressOffset, OffsetFrom(value));
}


ACCESSORS(JSModule, context, Object, kContextOffset)
ACCESSORS(JSModule, scope_info, ScopeInfo, kScopeInfoOffset)


JSModule* JSModule::cast(Object* obj) {
  ASSERT(obj->IsJSModule());
  ASSERT(HeapObject::cast(obj)->Size() == JSModule::kSize);
  return reinterpret_cast<JSModule*>(obj);
}


ACCESSORS(JSValue, value, Object, kValueOffset)


JSValue* JSValue::cast(Object* obj) {
  ASSERT(obj->IsJSValue());
  ASSERT(HeapObject::cast(obj)->Size() == JSValue::kSize);
  return reinterpret_cast<JSValue*>(obj);
}


ACCESSORS(JSDate, value, Object, kValueOffset)
ACCESSORS(JSDate, cache_stamp, Object, kCacheStampOffset)
ACCESSORS(JSDate, year, Object, kYearOffset)
ACCESSORS(JSDate, month, Object, kMonthOffset)
ACCESSORS(JSDate, day, Object, kDayOffset)
ACCESSORS(JSDate, weekday, Object, kWeekdayOffset)
ACCESSORS(JSDate, hour, Object, kHourOffset)
ACCESSORS(JSDate, min, Object, kMinOffset)
ACCESSORS(JSDate, sec, Object, kSecOffset)


JSDate* JSDate::cast(Object* obj) {
  ASSERT(obj->IsJSDate());
  ASSERT(HeapObject::cast(obj)->Size() == JSDate::kSize);
  return reinterpret_cast<JSDate*>(obj);
}


ACCESSORS(JSMessageObject, type, String, kTypeOffset)
ACCESSORS(JSMessageObject, arguments, JSArray, kArgumentsOffset)
ACCESSORS(JSMessageObject, script, Object, kScriptOffset)
ACCESSORS(JSMessageObject, stack_trace, Object, kStackTraceOffset)
ACCESSORS(JSMessageObject, stack_frames, Object, kStackFramesOffset)
SMI_ACCESSORS(JSMessageObject, start_position, kStartPositionOffset)
SMI_ACCESSORS(JSMessageObject, end_position, kEndPositionOffset)


JSMessageObject* JSMessageObject::cast(Object* obj) {
  ASSERT(obj->IsJSMessageObject());
  ASSERT(HeapObject::cast(obj)->Size() == JSMessageObject::kSize);
  return reinterpret_cast<JSMessageObject*>(obj);
}


INT_ACCESSORS(Code, instruction_size, kInstructionSizeOffset)
ACCESSORS(Code, relocation_info, ByteArray, kRelocationInfoOffset)
ACCESSORS(Code, handler_table, FixedArray, kHandlerTableOffset)
ACCESSORS(Code, deoptimization_data, FixedArray, kDeoptimizationDataOffset)
ACCESSORS(Code, type_feedback_info, Object, kTypeFeedbackInfoOffset)
ACCESSORS(Code, gc_metadata, Object, kGCMetadataOffset)
INT_ACCESSORS(Code, ic_age, kICAgeOffset)

byte* Code::instruction_start()  {
  return FIELD_ADDR(this, kHeaderSize);
}


byte* Code::instruction_end()  {
  return instruction_start() + instruction_size();
}


int Code::body_size() {
  return RoundUp(instruction_size(), kObjectAlignment);
}


FixedArray* Code::unchecked_deoptimization_data() {
  return reinterpret_cast<FixedArray*>(
      READ_FIELD(this, kDeoptimizationDataOffset));
}


ByteArray* Code::unchecked_relocation_info() {
  return reinterpret_cast<ByteArray*>(READ_FIELD(this, kRelocationInfoOffset));
}


byte* Code::relocation_start() {
  return unchecked_relocation_info()->GetDataStartAddress();
}


int Code::relocation_size() {
  return unchecked_relocation_info()->length();
}


byte* Code::entry() {
  return instruction_start();
}


bool Code::contains(byte* inner_pointer) {
  return (address() <= inner_pointer) && (inner_pointer <= address() + Size());
}


ACCESSORS(JSArray, length, Object, kLengthOffset)


ACCESSORS(JSRegExp, data, Object, kDataOffset)


JSRegExp::Type JSRegExp::TypeTag() {
  Object* data = this->data();
  if (data->IsUndefined()) return JSRegExp::NOT_COMPILED;
  Smi* smi = Smi::cast(FixedArray::cast(data)->get(kTagIndex));
  return static_cast<JSRegExp::Type>(smi->value());
}


JSRegExp::Type JSRegExp::TypeTagUnchecked() {
  Smi* smi = Smi::cast(DataAtUnchecked(kTagIndex));
  return static_cast<JSRegExp::Type>(smi->value());
}


int JSRegExp::CaptureCount() {
  switch (TypeTag()) {
    case ATOM:
      return 0;
    case IRREGEXP:
      return Smi::cast(DataAt(kIrregexpCaptureCountIndex))->value();
    default:
      UNREACHABLE();
      return -1;
  }
}


JSRegExp::Flags JSRegExp::GetFlags() {
  ASSERT(this->data()->IsFixedArray());
  Object* data = this->data();
  Smi* smi = Smi::cast(FixedArray::cast(data)->get(kFlagsIndex));
  return Flags(smi->value());
}


String* JSRegExp::Pattern() {
  ASSERT(this->data()->IsFixedArray());
  Object* data = this->data();
  String* pattern= String::cast(FixedArray::cast(data)->get(kSourceIndex));
  return pattern;
}


Object* JSRegExp::DataAt(int index) {
  ASSERT(TypeTag() != NOT_COMPILED);
  return FixedArray::cast(data())->get(index);
}


Object* JSRegExp::DataAtUnchecked(int index) {
  FixedArray* fa = reinterpret_cast<FixedArray*>(data());
  int offset = FixedArray::kHeaderSize + index * kPointerSize;
  return READ_FIELD(fa, offset);
}


void JSRegExp::SetDataAt(int index, Object* value) {
  ASSERT(TypeTag() != NOT_COMPILED);
  ASSERT(index >= kDataIndex);  // Only implementation data can be set this way.
  FixedArray::cast(data())->set(index, value);
}


void JSRegExp::SetDataAtUnchecked(int index, Object* value, Heap* heap) {
  ASSERT(index >= kDataIndex);  // Only implementation data can be set this way.
  FixedArray* fa = reinterpret_cast<FixedArray*>(data());
  if (value->IsSmi()) {
    fa->set_unchecked(index, Smi::cast(value));
  } else {
    // We only do this during GC, so we don't need to notify the write barrier.
    fa->set_unchecked(heap, index, value, SKIP_WRITE_BARRIER);
  }
}


ElementsKind JSObject::GetElementsKind() {
  ElementsKind kind = map()->elements_kind();
#if DEBUG
  FixedArrayBase* fixed_array =
      reinterpret_cast<FixedArrayBase*>(READ_FIELD(this, kElementsOffset));
  Map* map = fixed_array->map();
  ASSERT((IsFastSmiOrObjectElementsKind(kind) &&
          (map == GetHeap()->fixed_array_map() ||
           map == GetHeap()->fixed_cow_array_map())) ||
         (IsFastDoubleElementsKind(kind) &&
          (fixed_array->IsFixedDoubleArray() ||
           fixed_array == GetHeap()->empty_fixed_array())) ||
         (kind == DICTIONARY_ELEMENTS &&
            fixed_array->IsFixedArray() &&
          fixed_array->IsDictionary()) ||
         (kind > DICTIONARY_ELEMENTS));
  ASSERT((kind != NON_STRICT_ARGUMENTS_ELEMENTS) ||
         (elements()->IsFixedArray() && elements()->length() >= 2));
#endif
  return kind;
}


ElementsAccessor* JSObject::GetElementsAccessor() {
  return ElementsAccessor::ForKind(GetElementsKind());
}


bool JSObject::HasFastObjectElements() {
  return IsFastObjectElementsKind(GetElementsKind());
}


bool JSObject::HasFastSmiElements() {
  return IsFastSmiElementsKind(GetElementsKind());
}


bool JSObject::HasFastSmiOrObjectElements() {
  return IsFastSmiOrObjectElementsKind(GetElementsKind());
}


bool JSObject::HasFastDoubleElements() {
  return IsFastDoubleElementsKind(GetElementsKind());
}


bool JSObject::HasFastHoleyElements() {
  return IsFastHoleyElementsKind(GetElementsKind());
}


bool JSObject::HasDictionaryElements() {
  return GetElementsKind() == DICTIONARY_ELEMENTS;
}


bool JSObject::HasNonStrictArgumentsElements() {
  return GetElementsKind() == NON_STRICT_ARGUMENTS_ELEMENTS;
}


bool JSObject::HasExternalArrayElements() {
  HeapObject* array = elements();
  ASSERT(array != NULL);
  return array->IsExternalArray();
}


#define EXTERNAL_ELEMENTS_CHECK(name, type)          \
bool JSObject::HasExternal##name##Elements() {       \
  HeapObject* array = elements();                    \
  ASSERT(array != NULL);                             \
  if (!array->IsHeapObject())                        \
    return false;                                    \
  return array->map()->instance_type() == type;      \
}


EXTERNAL_ELEMENTS_CHECK(Byte, EXTERNAL_BYTE_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(UnsignedByte, EXTERNAL_UNSIGNED_BYTE_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(Short, EXTERNAL_SHORT_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(UnsignedShort,
                        EXTERNAL_UNSIGNED_SHORT_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(Int, EXTERNAL_INT_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(UnsignedInt,
                        EXTERNAL_UNSIGNED_INT_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(Float,
                        EXTERNAL_FLOAT_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(Double,
                        EXTERNAL_DOUBLE_ARRAY_TYPE)
EXTERNAL_ELEMENTS_CHECK(Pixel, EXTERNAL_PIXEL_ARRAY_TYPE)


bool JSObject::HasNamedInterceptor() {
  return map()->has_named_interceptor();
}


bool JSObject::HasIndexedInterceptor() {
  return map()->has_indexed_interceptor();
}


MaybeObject* JSObject::EnsureWritableFastElements() {
  ASSERT(HasFastSmiOrObjectElements());
  FixedArray* elems = FixedArray::cast(elements());
  Isolate* isolate = GetIsolate();
  if (elems->map() != isolate->heap()->fixed_cow_array_map()) return elems;
  Object* writable_elems;
  { MaybeObject* maybe_writable_elems = isolate->heap()->CopyFixedArrayWithMap(
      elems, isolate->heap()->fixed_array_map());
    if (!maybe_writable_elems->ToObject(&writable_elems)) {
      return maybe_writable_elems;
    }
  }
  set_elements(FixedArray::cast(writable_elems));
  isolate->counters()->cow_arrays_converted()->Increment();
  return writable_elems;
}


StringDictionary* JSObject::property_dictionary() {
  ASSERT(!HasFastProperties());
  return StringDictionary::cast(properties());
}


SeededNumberDictionary* JSObject::element_dictionary() {
  ASSERT(HasDictionaryElements());
  return SeededNumberDictionary::cast(elements());
}


bool String::IsHashFieldComputed(uint32_t field) {
  return (field & kHashNotComputedMask) == 0;
}


bool String::HasHashCode() {
  return IsHashFieldComputed(hash_field());
}


uint32_t String::Hash() {
  // Fast case: has hash code already been computed?
  uint32_t field = hash_field();
  if (IsHashFieldComputed(field)) return field >> kHashShift;
  // Slow case: compute hash code and set it.
  return ComputeAndSetHash();
}


StringHasher::StringHasher(int length, uint32_t seed)
  : length_(length),
    raw_running_hash_(seed),
    array_index_(0),
    is_array_index_(0 < length_ && length_ <= String::kMaxArrayIndexSize),
    is_first_char_(true) {
  ASSERT(FLAG_randomize_hashes || raw_running_hash_ == 0);
}


bool StringHasher::has_trivial_hash() {
  return length_ > String::kMaxHashCalcLength;
}


uint32_t StringHasher::AddCharacterCore(uint32_t running_hash, uint32_t c) {
  running_hash += c;
  running_hash += (running_hash << 10);
  running_hash ^= (running_hash >> 6);
  return running_hash;
}


uint32_t StringHasher::GetHashCore(uint32_t running_hash) {
  running_hash += (running_hash << 3);
  running_hash ^= (running_hash >> 11);
  running_hash += (running_hash << 15);
  if ((running_hash & String::kHashBitMask) == 0) {
    return kZeroHash;
  }
  return running_hash;
}


void StringHasher::AddCharacter(uint32_t c) {
  if (c > unibrow::Utf16::kMaxNonSurrogateCharCode) {
    AddSurrogatePair(c);  // Not inlined.
    return;
  }
  // Use the Jenkins one-at-a-time hash function to update the hash
  // for the given character.
  raw_running_hash_ = AddCharacterCore(raw_running_hash_, c);
  // Incremental array index computation.
  if (is_array_index_) {
    if (c < '0' || c > '9') {
      is_array_index_ = false;
    } else {
      int d = c - '0';
      if (is_first_char_) {
        is_first_char_ = false;
        if (c == '0' && length_ > 1) {
          is_array_index_ = false;
          return;
        }
      }
      if (array_index_ > 429496729U - ((d + 2) >> 3)) {
        is_array_index_ = false;
      } else {
        array_index_ = array_index_ * 10 + d;
      }
    }
  }
}


void StringHasher::AddCharacterNoIndex(uint32_t c) {
  ASSERT(!is_array_index());
  if (c > unibrow::Utf16::kMaxNonSurrogateCharCode) {
    AddSurrogatePairNoIndex(c);  // Not inlined.
    return;
  }
  raw_running_hash_ = AddCharacterCore(raw_running_hash_, c);
}


uint32_t StringHasher::GetHash() {
  // Get the calculated raw hash value and do some more bit ops to distribute
  // the hash further. Ensure that we never return zero as the hash value.
  return GetHashCore(raw_running_hash_);
}


template <typename schar>
uint32_t HashSequentialString(const schar* chars, int length, uint32_t seed) {
  StringHasher hasher(length, seed);
  if (!hasher.has_trivial_hash()) {
    int i;
    for (i = 0; hasher.is_array_index() && (i < length); i++) {
      hasher.AddCharacter(chars[i]);
    }
    for (; i < length; i++) {
      hasher.AddCharacterNoIndex(chars[i]);
    }
  }
  return hasher.GetHashField();
}


bool String::AsArrayIndex(uint32_t* index) {
  uint32_t field = hash_field();
  if (IsHashFieldComputed(field) && (field & kIsNotArrayIndexMask)) {
    return false;
  }
  return SlowAsArrayIndex(index);
}


Object* JSReceiver::GetPrototype() {
  return map()->prototype();
}


Object* JSReceiver::GetConstructor() {
  return map()->constructor();
}


bool JSReceiver::HasProperty(String* name) {
  if (IsJSProxy()) {
    return JSProxy::cast(this)->HasPropertyWithHandler(name);
  }
  return GetPropertyAttribute(name) != ABSENT;
}


bool JSReceiver::HasLocalProperty(String* name) {
  if (IsJSProxy()) {
    return JSProxy::cast(this)->HasPropertyWithHandler(name);
  }
  return GetLocalPropertyAttribute(name) != ABSENT;
}


PropertyAttributes JSReceiver::GetPropertyAttribute(String* key) {
  return GetPropertyAttributeWithReceiver(this, key);
}

// TODO(504): this may be useful in other places too where JSGlobalProxy
// is used.
Object* JSObject::BypassGlobalProxy() {
  if (IsJSGlobalProxy()) {
    Object* proto = GetPrototype();
    if (proto->IsNull()) return GetHeap()->undefined_value();
    ASSERT(proto->IsJSGlobalObject());
    return proto;
  }
  return this;
}


MaybeObject* JSReceiver::GetIdentityHash(CreationFlag flag) {
  return IsJSProxy()
      ? JSProxy::cast(this)->GetIdentityHash(flag)
      : JSObject::cast(this)->GetIdentityHash(flag);
}


bool JSReceiver::HasElement(uint32_t index) {
  if (IsJSProxy()) {
    return JSProxy::cast(this)->HasElementWithHandler(index);
  }
  return JSObject::cast(this)->HasElementWithReceiver(this, index);
}


bool AccessorInfo::all_can_read() {
  return BooleanBit::get(flag(), kAllCanReadBit);
}


void AccessorInfo::set_all_can_read(bool value) {
  set_flag(BooleanBit::set(flag(), kAllCanReadBit, value));
}


bool AccessorInfo::all_can_write() {
  return BooleanBit::get(flag(), kAllCanWriteBit);
}


void AccessorInfo::set_all_can_write(bool value) {
  set_flag(BooleanBit::set(flag(), kAllCanWriteBit, value));
}


bool AccessorInfo::prohibits_overwriting() {
  return BooleanBit::get(flag(), kProhibitsOverwritingBit);
}


void AccessorInfo::set_prohibits_overwriting(bool value) {
  set_flag(BooleanBit::set(flag(), kProhibitsOverwritingBit, value));
}


PropertyAttributes AccessorInfo::property_attributes() {
  return AttributesField::decode(static_cast<uint32_t>(flag()->value()));
}


void AccessorInfo::set_property_attributes(PropertyAttributes attributes) {
  set_flag(Smi::FromInt(AttributesField::update(flag()->value(), attributes)));
}


bool AccessorInfo::IsCompatibleReceiver(Object* receiver) {
  Object* function_template = expected_receiver_type();
  if (!function_template->IsFunctionTemplateInfo()) return true;
  return receiver->IsInstanceOf(FunctionTemplateInfo::cast(function_template));
}


template<typename Shape, typename Key>
void Dictionary<Shape, Key>::SetEntry(int entry,
                                      Object* key,
                                      Object* value) {
  SetEntry(entry, key, value, PropertyDetails(Smi::FromInt(0)));
}


template<typename Shape, typename Key>
void Dictionary<Shape, Key>::SetEntry(int entry,
                                      Object* key,
                                      Object* value,
                                      PropertyDetails details) {
  ASSERT(!key->IsString() ||
         details.IsDeleted() ||
         details.dictionary_index() > 0);
  int index = HashTable<Shape, Key>::EntryToIndex(entry);
  AssertNoAllocation no_gc;
  WriteBarrierMode mode = FixedArray::GetWriteBarrierMode(no_gc);
  FixedArray::set(index, key, mode);
  FixedArray::set(index+1, value, mode);
  FixedArray::set(index+2, details.AsSmi());
}


bool NumberDictionaryShape::IsMatch(uint32_t key, Object* other) {
  ASSERT(other->IsNumber());
  return key == static_cast<uint32_t>(other->Number());
}


uint32_t UnseededNumberDictionaryShape::Hash(uint32_t key) {
  return ComputeIntegerHash(key, 0);
}


uint32_t UnseededNumberDictionaryShape::HashForObject(uint32_t key,
                                                      Object* other) {
  ASSERT(other->IsNumber());
  return ComputeIntegerHash(static_cast<uint32_t>(other->Number()), 0);
}

uint32_t SeededNumberDictionaryShape::SeededHash(uint32_t key, uint32_t seed) {
  return ComputeIntegerHash(key, seed);
}

uint32_t SeededNumberDictionaryShape::SeededHashForObject(uint32_t key,
                                                          uint32_t seed,
                                                          Object* other) {
  ASSERT(other->IsNumber());
  return ComputeIntegerHash(static_cast<uint32_t>(other->Number()), seed);
}

MaybeObject* NumberDictionaryShape::AsObject(uint32_t key) {
  return Isolate::Current()->heap()->NumberFromUint32(key);
}


bool StringDictionaryShape::IsMatch(String* key, Object* other) {
  // We know that all entries in a hash table had their hash keys created.
  // Use that knowledge to have fast failure.
  if (key->Hash() != String::cast(other)->Hash()) return false;
  return key->Equals(String::cast(other));
}


uint32_t StringDictionaryShape::Hash(String* key) {
  return key->Hash();
}


uint32_t StringDictionaryShape::HashForObject(String* key, Object* other) {
  return String::cast(other)->Hash();
}


MaybeObject* StringDictionaryShape::AsObject(String* key) {
  return key;
}


template <int entrysize>
bool ObjectHashTableShape<entrysize>::IsMatch(Object* key, Object* other) {
  return key->SameValue(other);
}


template <int entrysize>
uint32_t ObjectHashTableShape<entrysize>::Hash(Object* key) {
  MaybeObject* maybe_hash = key->GetHash(OMIT_CREATION);
  return Smi::cast(maybe_hash->ToObjectChecked())->value();
}


template <int entrysize>
uint32_t ObjectHashTableShape<entrysize>::HashForObject(Object* key,
                                                        Object* other) {
  MaybeObject* maybe_hash = other->GetHash(OMIT_CREATION);
  return Smi::cast(maybe_hash->ToObjectChecked())->value();
}


template <int entrysize>
MaybeObject* ObjectHashTableShape<entrysize>::AsObject(Object* key) {
  return key;
}


void Map::ClearCodeCache(Heap* heap) {
  // No write barrier is needed since empty_fixed_array is not in new space.
  // Please note this function is used during marking:
  //  - MarkCompactCollector::MarkUnmarkedObject
  //  - IncrementalMarking::Step
  ASSERT(!heap->InNewSpace(heap->raw_unchecked_empty_fixed_array()));
  WRITE_FIELD(this, kCodeCacheOffset, heap->raw_unchecked_empty_fixed_array());
}


void JSArray::EnsureSize(int required_size) {
  ASSERT(HasFastSmiOrObjectElements());
  FixedArray* elts = FixedArray::cast(elements());
  const int kArraySizeThatFitsComfortablyInNewSpace = 128;
  if (elts->length() < required_size) {
    // Doubling in size would be overkill, but leave some slack to avoid
    // constantly growing.
    Expand(required_size + (required_size >> 3));
    // It's a performance benefit to keep a frequently used array in new-space.
  } else if (!GetHeap()->new_space()->Contains(elts) &&
             required_size < kArraySizeThatFitsComfortablyInNewSpace) {
    // Expand will allocate a new backing store in new space even if the size
    // we asked for isn't larger than what we had before.
    Expand(required_size);
  }
}


void JSArray::set_length(Smi* length) {
  // Don't need a write barrier for a Smi.
  set_length(static_cast<Object*>(length), SKIP_WRITE_BARRIER);
}


bool JSArray::AllowsSetElementsLength() {
  bool result = elements()->IsFixedArray() || elements()->IsFixedDoubleArray();
  ASSERT(result == !HasExternalArrayElements());
  return result;
}


MaybeObject* JSArray::SetContent(FixedArrayBase* storage) {
  MaybeObject* maybe_result = EnsureCanContainElements(
      storage, storage->length(), ALLOW_COPIED_DOUBLE_ELEMENTS);
  if (maybe_result->IsFailure()) return maybe_result;
  ASSERT((storage->map() == GetHeap()->fixed_double_array_map() &&
          IsFastDoubleElementsKind(GetElementsKind())) ||
         ((storage->map() != GetHeap()->fixed_double_array_map()) &&
          (IsFastObjectElementsKind(GetElementsKind()) ||
           (IsFastSmiElementsKind(GetElementsKind()) &&
            FixedArray::cast(storage)->ContainsOnlySmisOrHoles()))));
  set_elements(storage);
  set_length(Smi::FromInt(storage->length()));
  return this;
}


MaybeObject* FixedArray::Copy() {
  if (length() == 0) return this;
  return GetHeap()->CopyFixedArray(this);
}


MaybeObject* FixedDoubleArray::Copy() {
  if (length() == 0) return this;
  return GetHeap()->CopyFixedDoubleArray(this);
}


void TypeFeedbackCells::SetAstId(int index, TypeFeedbackId id) {
  set(1 + index * 2, Smi::FromInt(id.ToInt()));
}


TypeFeedbackId TypeFeedbackCells::AstId(int index) {
  return TypeFeedbackId(Smi::cast(get(1 + index * 2))->value());
}


void TypeFeedbackCells::SetCell(int index, JSGlobalPropertyCell* cell) {
  set(index * 2, cell);
}


JSGlobalPropertyCell* TypeFeedbackCells::Cell(int index) {
  return JSGlobalPropertyCell::cast(get(index * 2));
}


Handle<Object> TypeFeedbackCells::UninitializedSentinel(Isolate* isolate) {
  return isolate->factory()->the_hole_value();
}


Handle<Object> TypeFeedbackCells::MegamorphicSentinel(Isolate* isolate) {
  return isolate->factory()->undefined_value();
}


Object* TypeFeedbackCells::RawUninitializedSentinel(Heap* heap) {
  return heap->raw_unchecked_the_hole_value();
}


int TypeFeedbackInfo::ic_total_count() {
  int current = Smi::cast(READ_FIELD(this, kStorage1Offset))->value();
  return ICTotalCountField::decode(current);
}


void TypeFeedbackInfo::set_ic_total_count(int count) {
  int value = Smi::cast(READ_FIELD(this, kStorage1Offset))->value();
  value = ICTotalCountField::update(value,
                                    ICTotalCountField::decode(count));
  WRITE_FIELD(this, kStorage1Offset, Smi::FromInt(value));
}


int TypeFeedbackInfo::ic_with_type_info_count() {
  int current = Smi::cast(READ_FIELD(this, kStorage2Offset))->value();
  return ICsWithTypeInfoCountField::decode(current);
}


void TypeFeedbackInfo::change_ic_with_type_info_count(int delta) {
  int value = Smi::cast(READ_FIELD(this, kStorage2Offset))->value();
  int new_count = ICsWithTypeInfoCountField::decode(value) + delta;
  // We can get negative count here when the type-feedback info is
  // shared between two code objects. The can only happen when
  // the debugger made a shallow copy of code object (see Heap::CopyCode).
  // Since we do not optimize when the debugger is active, we can skip
  // this counter update.
  if (new_count >= 0) {
    new_count &= ICsWithTypeInfoCountField::kMask;
    value = ICsWithTypeInfoCountField::update(value, new_count);
    WRITE_FIELD(this, kStorage2Offset, Smi::FromInt(value));
  }
}


void TypeFeedbackInfo::initialize_storage() {
  WRITE_FIELD(this, kStorage1Offset, Smi::FromInt(0));
  WRITE_FIELD(this, kStorage2Offset, Smi::FromInt(0));
}


void TypeFeedbackInfo::change_own_type_change_checksum() {
  int value = Smi::cast(READ_FIELD(this, kStorage1Offset))->value();
  int checksum = OwnTypeChangeChecksum::decode(value);
  checksum = (checksum + 1) % (1 << kTypeChangeChecksumBits);
  value = OwnTypeChangeChecksum::update(value, checksum);
  // Ensure packed bit field is in Smi range.
  if (value > Smi::kMaxValue) value |= Smi::kMinValue;
  if (value < Smi::kMinValue) value &= ~Smi::kMinValue;
  WRITE_FIELD(this, kStorage1Offset, Smi::FromInt(value));
}


void TypeFeedbackInfo::set_inlined_type_change_checksum(int checksum) {
  int value = Smi::cast(READ_FIELD(this, kStorage2Offset))->value();
  int mask = (1 << kTypeChangeChecksumBits) - 1;
  value = InlinedTypeChangeChecksum::update(value, checksum & mask);
  // Ensure packed bit field is in Smi range.
  if (value > Smi::kMaxValue) value |= Smi::kMinValue;
  if (value < Smi::kMinValue) value &= ~Smi::kMinValue;
  WRITE_FIELD(this, kStorage2Offset, Smi::FromInt(value));
}


int TypeFeedbackInfo::own_type_change_checksum() {
  int value = Smi::cast(READ_FIELD(this, kStorage1Offset))->value();
  return OwnTypeChangeChecksum::decode(value);
}


bool TypeFeedbackInfo::matches_inlined_type_change_checksum(int checksum) {
  int value = Smi::cast(READ_FIELD(this, kStorage2Offset))->value();
  int mask = (1 << kTypeChangeChecksumBits) - 1;
  return InlinedTypeChangeChecksum::decode(value) == (checksum & mask);
}


ACCESSORS(TypeFeedbackInfo, type_feedback_cells, TypeFeedbackCells,
          kTypeFeedbackCellsOffset)


SMI_ACCESSORS(AliasedArgumentsEntry, aliased_context_slot, kAliasedContextSlot)


Relocatable::Relocatable(Isolate* isolate) {
  ASSERT(isolate == Isolate::Current());
  isolate_ = isolate;
  prev_ = isolate->relocatable_top();
  isolate->set_relocatable_top(this);
}


Relocatable::~Relocatable() {
  ASSERT(isolate_ == Isolate::Current());
  ASSERT_EQ(isolate_->relocatable_top(), this);
  isolate_->set_relocatable_top(prev_);
}


int JSObject::BodyDescriptor::SizeOf(Map* map, HeapObject* object) {
  return map->instance_size();
}


void Foreign::ForeignIterateBody(ObjectVisitor* v) {
  v->VisitExternalReference(
      reinterpret_cast<Address*>(FIELD_ADDR(this, kForeignAddressOffset)));
}


template<typename StaticVisitor>
void Foreign::ForeignIterateBody() {
  StaticVisitor::VisitExternalReference(
      reinterpret_cast<Address*>(FIELD_ADDR(this, kForeignAddressOffset)));
}


void ExternalAsciiString::ExternalAsciiStringIterateBody(ObjectVisitor* v) {
  typedef v8::String::ExternalAsciiStringResource Resource;
  v->VisitExternalAsciiString(
      reinterpret_cast<Resource**>(FIELD_ADDR(this, kResourceOffset)));
}


template<typename StaticVisitor>
void ExternalAsciiString::ExternalAsciiStringIterateBody() {
  typedef v8::String::ExternalAsciiStringResource Resource;
  StaticVisitor::VisitExternalAsciiString(
      reinterpret_cast<Resource**>(FIELD_ADDR(this, kResourceOffset)));
}


void ExternalTwoByteString::ExternalTwoByteStringIterateBody(ObjectVisitor* v) {
  typedef v8::String::ExternalStringResource Resource;
  v->VisitExternalTwoByteString(
      reinterpret_cast<Resource**>(FIELD_ADDR(this, kResourceOffset)));
}


template<typename StaticVisitor>
void ExternalTwoByteString::ExternalTwoByteStringIterateBody() {
  typedef v8::String::ExternalStringResource Resource;
  StaticVisitor::VisitExternalTwoByteString(
      reinterpret_cast<Resource**>(FIELD_ADDR(this, kResourceOffset)));
}


template<int start_offset, int end_offset, int size>
void FixedBodyDescriptor<start_offset, end_offset, size>::IterateBody(
    HeapObject* obj,
    ObjectVisitor* v) {
    v->VisitPointers(HeapObject::RawField(obj, start_offset),
                     HeapObject::RawField(obj, end_offset));
}


template<int start_offset>
void FlexibleBodyDescriptor<start_offset>::IterateBody(HeapObject* obj,
                                                       int object_size,
                                                       ObjectVisitor* v) {
  v->VisitPointers(HeapObject::RawField(obj, start_offset),
                   HeapObject::RawField(obj, object_size));
}


#undef TYPE_CHECKER
#undef CAST_ACCESSOR
#undef INT_ACCESSORS
#undef ACCESSORS
#undef ACCESSORS_TO_SMI
#undef SMI_ACCESSORS
#undef BOOL_GETTER
#undef BOOL_ACCESSORS
#undef FIELD_ADDR
#undef READ_FIELD
#undef WRITE_FIELD
#undef WRITE_BARRIER
#undef CONDITIONAL_WRITE_BARRIER
#undef READ_DOUBLE_FIELD
#undef WRITE_DOUBLE_FIELD
#undef READ_INT_FIELD
#undef WRITE_INT_FIELD
#undef READ_INTPTR_FIELD
#undef WRITE_INTPTR_FIELD
#undef READ_UINT32_FIELD
#undef WRITE_UINT32_FIELD
#undef READ_SHORT_FIELD
#undef WRITE_SHORT_FIELD
#undef READ_BYTE_FIELD
#undef WRITE_BYTE_FIELD


} }  // namespace v8::internal

#endif  // V8_OBJECTS_INL_H_