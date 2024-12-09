class ArrayInstructionInterface {
 public:
  virtual HValue* GetKey() = 0;
  virtual void SetKey(HValue* key) = 0;
  virtual void SetIndexOffset(uint32_t index_offset) = 0;
  virtual bool IsDehoisted() = 0;
  virtual void SetDehoisted(bool is_dehoisted) = 0;
  virtual ~ArrayInstructionInterface() { };
};

class HLoadKeyedFastElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HLoadKeyedFastElement(HValue* obj,
                        HValue* key,
                        HValue* dependency,
                        ElementsKind elements_kind = FAST_ELEMENTS)
      : bit_field_(0) {
    ASSERT(IsFastSmiOrObjectElementsKind(elements_kind));
    bit_field_ = ElementsKindField::encode(elements_kind);
    if (IsFastSmiElementsKind(elements_kind) &&
        IsFastPackedElementsKind(elements_kind)) {
      set_type(HType::Smi());
    }
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, dependency);
    set_representation(Representation::Tagged());
    SetGVNFlag(kDependsOnArrayElements);
    SetFlag(kUseGVN);
  }

  HValue* object() { return OperandAt(0); }
  void SetIndexOffset(uint32_t index_offset) {
    bit_field_ = IndexOffsetField::update(bit_field_, index_offset);
  }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return IsDehoistedField::decode(bit_field_); }
  void SetDehoisted(bool is_dehoisted) {
    bit_field_ = IsDehoistedField::update(bit_field_, is_dehoisted);
  }
  ElementsKind elements_kind() const {
    return ElementsKindField::decode(bit_field_);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::Tagged();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  virtual void PrintDataTo(StringStream* stream);

  bool RequiresHoleCheck() const;

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedFastElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedFastElement()) return false;
    HLoadKeyedFastElement* other_load = HLoadKeyedFastElement::cast(other);
    if (IsDehoisted() && index_offset() != other_load->index_offset())
      return false;
    return elements_kind() == other_load->elements_kind();
  }

 private:
  virtual bool IsDeletable() const { return !RequiresHoleCheck(); }

  class ElementsKindField:  public BitField<ElementsKind, 0, 4> {};
  class IndexOffsetField:   public BitField<uint32_t, 4, 27> {};
  class IsDehoistedField:   public BitField<bool, 31, 1> {};
  uint32_t bit_field_;
};


enum HoleCheckMode { PERFORM_HOLE_CHECK, OMIT_HOLE_CHECK };


class HLoadKeyedFastDoubleElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HLoadKeyedFastDoubleElement(
    HValue* elements,
    HValue* key,
    HValue* dependency,
    HoleCheckMode hole_check_mode = PERFORM_HOLE_CHECK)
      : index_offset_(0),
        is_dehoisted_(false),
        hole_check_mode_(hole_check_mode) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, dependency);
    set_representation(Representation::Double());
    SetGVNFlag(kDependsOnDoubleArrayElements);
    SetFlag(kUseGVN);
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* dependency() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::Tagged();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  bool RequiresHoleCheck() const {
    return hole_check_mode_ == PERFORM_HOLE_CHECK;
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedFastDoubleElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedFastDoubleElement()) return false;
    HLoadKeyedFastDoubleElement* other_load =
        HLoadKeyedFastDoubleElement::cast(other);
    return hole_check_mode_ == other_load->hole_check_mode_;
  }

 private:
  virtual bool IsDeletable() const { return !RequiresHoleCheck(); }

  uint32_t index_offset_;
  bool is_dehoisted_;
  HoleCheckMode hole_check_mode_;
};


class HLoadKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HLoadKeyedSpecializedArrayElement(HValue* external_elements,
                                    HValue* key,
                                    HValue* dependency,
                                    ElementsKind elements_kind)
      :  elements_kind_(elements_kind),
         index_offset_(0),
         is_dehoisted_(false) {
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, dependency);
    if (elements_kind == EXTERNAL_FLOAT_ELEMENTS ||
        elements_kind == EXTERNAL_DOUBLE_ELEMENTS) {
      set_representation(Representation::Double());
    } else {
      set_representation(Representation::Integer32());
    }
    SetGVNFlag(kDependsOnSpecializedArrayElements);
    // Native code could change the specialized array.
    SetGVNFlag(kDependsOnCalls);
    SetFlag(kUseGVN);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::External();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* dependency() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  virtual Range* InferRange(Zone* zone);

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedSpecializedArrayElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedSpecializedArrayElement()) return false;
    HLoadKeyedSpecializedArrayElement* cast_other =
        HLoadKeyedSpecializedArrayElement::cast(other);
    return elements_kind_ == cast_other->elements_kind();
  }

 private:
  virtual bool IsDeletable() const { return true; }

  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HLoadKeyedGeneric: public HTemplateInstruction<3> {
 public:
  HLoadKeyedGeneric(HValue* context, HValue* obj, HValue* key) {
    set_representation(Representation::Tagged());
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HValue* Canonicalize();

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedGeneric)
};


class HStoreNamedField: public HTemplateInstruction<2> {
 public:
  HStoreNamedField(HValue* obj,
                   Handle<String> name,
                   HValue* val,
                   bool in_object,
                   int offset)
      : name_(name),
        is_in_object_(in_object),
        offset_(offset),
        new_space_dominator_(NULL) {
    SetOperandAt(0, obj);
    SetOperandAt(1, val);
    SetFlag(kTrackSideEffectDominators);
    SetGVNFlag(kDependsOnNewSpacePromotion);
    if (is_in_object_) {
      SetGVNFlag(kChangesInobjectFields);
    } else {
      SetGVNFlag(kChangesBackingStoreFields);
    }
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedField)

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual void SetSideEffectDominator(GVNFlag side_effect, HValue* dominator) {
    ASSERT(side_effect == kChangesNewSpacePromotion);
    new_space_dominator_ = dominator;
  }
  virtual void PrintDataTo(StringStream* stream);

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  Handle<String> name() const { return name_; }
  bool is_in_object() const { return is_in_object_; }
  int offset() const { return offset_; }
  Handle<Map> transition() const { return transition_; }
  void set_transition(Handle<Map> map) { transition_ = map; }
  HValue* new_space_dominator() const { return new_space_dominator_; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value()) &&
        ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

  bool NeedsWriteBarrierForMap() {
    return ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

 private:
  Handle<String> name_;
  bool is_in_object_;
  int offset_;
  Handle<Map> transition_;
  HValue* new_space_dominator_;
};


class HStoreNamedGeneric: public HTemplateInstruction<3> {
 public:
  HStoreNamedGeneric(HValue* context,
                     HValue* object,
                     Handle<String> name,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : name_(name),
        strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, value);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }
  Handle<String> name() { return name_; }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedGeneric)

 private:
  Handle<String> name_;
  StrictModeFlag strict_mode_flag_;
};


class HStoreKeyedFastElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastElement(HValue* obj, HValue* key, HValue* val,
                         ElementsKind elements_kind = FAST_ELEMENTS)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetGVNFlag(kChangesArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    return index == 1
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  bool value_is_smi() {
    return IsFastSmiElementsKind(elements_kind_);
  }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    if (value_is_smi()) {
      return false;
    } else {
      return StoringValueNeedsWriteBarrier(value());
    }
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedFastDoubleElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastDoubleElement(HValue* elements,
                               HValue* key,
                               HValue* val)
      : index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetFlag(kDeoptimizeOnUndefined);
    SetGVNFlag(kChangesDoubleArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 1) {
      return Representation::Integer32();
    } else if (index == 2) {
      return Representation::Double();
    } else {
      return Representation::Tagged();
    }
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value());
  }

  bool NeedsCanonicalization();

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastDoubleElement)

 private:
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedSpecializedArrayElement(HValue* external_elements,
                                     HValue* key,
                                     HValue* val,
                                     ElementsKind elements_kind)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetGVNFlag(kChangesSpecializedArrayElements);
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 0) {
      return Representation::External();
    } else {
      bool float_or_double_elements =
          elements_kind() == EXTERNAL_FLOAT_ELEMENTS ||
          elements_kind() == EXTERNAL_DOUBLE_ELEMENTS;
      if (index == 2 && float_or_double_elements) {
        return Representation::Double();
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_
        hole_check_mode_(hole_check_mode) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, dependency);
    set_representation(Representation::Double());
    SetGVNFlag(kDependsOnDoubleArrayElements);
    SetFlag(kUseGVN);
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* dependency() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::Tagged();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  bool RequiresHoleCheck() const {
    return hole_check_mode_ == PERFORM_HOLE_CHECK;
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedFastDoubleElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedFastDoubleElement()) return false;
    HLoadKeyedFastDoubleElement* other_load =
        HLoadKeyedFastDoubleElement::cast(other);
    return hole_check_mode_ == other_load->hole_check_mode_;
  }

 private:
  virtual bool IsDeletable() const { return !RequiresHoleCheck(); }

  uint32_t index_offset_;
  bool is_dehoisted_;
  HoleCheckMode hole_check_mode_;
};


class HLoadKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HLoadKeyedSpecializedArrayElement(HValue* external_elements,
                                    HValue* key,
                                    HValue* dependency,
                                    ElementsKind elements_kind)
      :  elements_kind_(elements_kind),
         index_offset_(0),
         is_dehoisted_(false) {
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, dependency);
    if (elements_kind == EXTERNAL_FLOAT_ELEMENTS ||
        elements_kind == EXTERNAL_DOUBLE_ELEMENTS) {
      set_representation(Representation::Double());
    } else {
      set_representation(Representation::Integer32());
    }
    SetGVNFlag(kDependsOnSpecializedArrayElements);
    // Native code could change the specialized array.
    SetGVNFlag(kDependsOnCalls);
    SetFlag(kUseGVN);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::External();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* dependency() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  virtual Range* InferRange(Zone* zone);

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedSpecializedArrayElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedSpecializedArrayElement()) return false;
    HLoadKeyedSpecializedArrayElement* cast_other =
        HLoadKeyedSpecializedArrayElement::cast(other);
    return elements_kind_ == cast_other->elements_kind();
  }

 private:
  virtual bool IsDeletable() const { return true; }

  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HLoadKeyedGeneric: public HTemplateInstruction<3> {
 public:
  HLoadKeyedGeneric(HValue* context, HValue* obj, HValue* key) {
    set_representation(Representation::Tagged());
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HValue* Canonicalize();

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedGeneric)
};


class HStoreNamedField: public HTemplateInstruction<2> {
 public:
  HStoreNamedField(HValue* obj,
                   Handle<String> name,
                   HValue* val,
                   bool in_object,
                   int offset)
      : name_(name),
        is_in_object_(in_object),
        offset_(offset),
        new_space_dominator_(NULL) {
    SetOperandAt(0, obj);
    SetOperandAt(1, val);
    SetFlag(kTrackSideEffectDominators);
    SetGVNFlag(kDependsOnNewSpacePromotion);
    if (is_in_object_) {
      SetGVNFlag(kChangesInobjectFields);
    } else {
      SetGVNFlag(kChangesBackingStoreFields);
    }
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedField)

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual void SetSideEffectDominator(GVNFlag side_effect, HValue* dominator) {
    ASSERT(side_effect == kChangesNewSpacePromotion);
    new_space_dominator_ = dominator;
  }
  virtual void PrintDataTo(StringStream* stream);

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  Handle<String> name() const { return name_; }
  bool is_in_object() const { return is_in_object_; }
  int offset() const { return offset_; }
  Handle<Map> transition() const { return transition_; }
  void set_transition(Handle<Map> map) { transition_ = map; }
  HValue* new_space_dominator() const { return new_space_dominator_; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value()) &&
        ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

  bool NeedsWriteBarrierForMap() {
    return ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

 private:
  Handle<String> name_;
  bool is_in_object_;
  int offset_;
  Handle<Map> transition_;
  HValue* new_space_dominator_;
};


class HStoreNamedGeneric: public HTemplateInstruction<3> {
 public:
  HStoreNamedGeneric(HValue* context,
                     HValue* object,
                     Handle<String> name,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : name_(name),
        strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, value);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }
  Handle<String> name() { return name_; }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedGeneric)

 private:
  Handle<String> name_;
  StrictModeFlag strict_mode_flag_;
};


class HStoreKeyedFastElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastElement(HValue* obj, HValue* key, HValue* val,
                         ElementsKind elements_kind = FAST_ELEMENTS)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetGVNFlag(kChangesArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    return index == 1
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  bool value_is_smi() {
    return IsFastSmiElementsKind(elements_kind_);
  }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    if (value_is_smi()) {
      return false;
    } else {
      return StoringValueNeedsWriteBarrier(value());
    }
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedFastDoubleElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastDoubleElement(HValue* elements,
                               HValue* key,
                               HValue* val)
      : index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetFlag(kDeoptimizeOnUndefined);
    SetGVNFlag(kChangesDoubleArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 1) {
      return Representation::Integer32();
    } else if (index == 2) {
      return Representation::Double();
    } else {
      return Representation::Tagged();
    }
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value());
  }

  bool NeedsCanonicalization();

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastDoubleElement)

 private:
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedSpecializedArrayElement(HValue* external_elements,
                                     HValue* key,
                                     HValue* val,
                                     ElementsKind elements_kind)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetGVNFlag(kChangesSpecializedArrayElements);
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 0) {
      return Representation::External();
    } else {
      bool float_or_double_elements =
          elements_kind() == EXTERNAL_FLOAT_ELEMENTS ||
          elements_kind() == EXTERNAL_DOUBLE_ELEMENTS;
      if (index == 2 && float_or_double_elements) {
        return Representation::Double();
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_
  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    if (index == 0) return Representation::External();
    if (index == 1) return Representation::Integer32();
    return Representation::None();
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* dependency() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  virtual Range* InferRange(Zone* zone);

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedSpecializedArrayElement)

 protected:
  virtual bool DataEquals(HValue* other) {
    if (!other->IsLoadKeyedSpecializedArrayElement()) return false;
    HLoadKeyedSpecializedArrayElement* cast_other =
        HLoadKeyedSpecializedArrayElement::cast(other);
    return elements_kind_ == cast_other->elements_kind();
  }

 private:
  virtual bool IsDeletable() const { return true; }

  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HLoadKeyedGeneric: public HTemplateInstruction<3> {
 public:
  HLoadKeyedGeneric(HValue* context, HValue* obj, HValue* key) {
    set_representation(Representation::Tagged());
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HValue* Canonicalize();

  DECLARE_CONCRETE_INSTRUCTION(LoadKeyedGeneric)
};


class HStoreNamedField: public HTemplateInstruction<2> {
 public:
  HStoreNamedField(HValue* obj,
                   Handle<String> name,
                   HValue* val,
                   bool in_object,
                   int offset)
      : name_(name),
        is_in_object_(in_object),
        offset_(offset),
        new_space_dominator_(NULL) {
    SetOperandAt(0, obj);
    SetOperandAt(1, val);
    SetFlag(kTrackSideEffectDominators);
    SetGVNFlag(kDependsOnNewSpacePromotion);
    if (is_in_object_) {
      SetGVNFlag(kChangesInobjectFields);
    } else {
      SetGVNFlag(kChangesBackingStoreFields);
    }
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedField)

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual void SetSideEffectDominator(GVNFlag side_effect, HValue* dominator) {
    ASSERT(side_effect == kChangesNewSpacePromotion);
    new_space_dominator_ = dominator;
  }
  virtual void PrintDataTo(StringStream* stream);

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  Handle<String> name() const { return name_; }
  bool is_in_object() const { return is_in_object_; }
  int offset() const { return offset_; }
  Handle<Map> transition() const { return transition_; }
  void set_transition(Handle<Map> map) { transition_ = map; }
  HValue* new_space_dominator() const { return new_space_dominator_; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value()) &&
        ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

  bool NeedsWriteBarrierForMap() {
    return ReceiverObjectNeedsWriteBarrier(object(), new_space_dominator());
  }

 private:
  Handle<String> name_;
  bool is_in_object_;
  int offset_;
  Handle<Map> transition_;
  HValue* new_space_dominator_;
};


class HStoreNamedGeneric: public HTemplateInstruction<3> {
 public:
  HStoreNamedGeneric(HValue* context,
                     HValue* object,
                     Handle<String> name,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : name_(name),
        strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, value);
    SetOperandAt(2, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }
  HValue* context() { return OperandAt(2); }
  Handle<String> name() { return name_; }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(StoreNamedGeneric)

 private:
  Handle<String> name_;
  StrictModeFlag strict_mode_flag_;
};


class HStoreKeyedFastElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastElement(HValue* obj, HValue* key, HValue* val,
                         ElementsKind elements_kind = FAST_ELEMENTS)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, obj);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetGVNFlag(kChangesArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The key is supposed to be Integer32.
    return index == 1
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  bool value_is_smi() {
    return IsFastSmiElementsKind(elements_kind_);
  }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    if (value_is_smi()) {
      return false;
    } else {
      return StoringValueNeedsWriteBarrier(value());
    }
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedFastDoubleElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastDoubleElement(HValue* elements,
                               HValue* key,
                               HValue* val)
      : index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetFlag(kDeoptimizeOnUndefined);
    SetGVNFlag(kChangesDoubleArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 1) {
      return Representation::Integer32();
    } else if (index == 2) {
      return Representation::Double();
    } else {
      return Representation::Tagged();
    }
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value());
  }

  bool NeedsCanonicalization();

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastDoubleElement)

 private:
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedSpecializedArrayElement(HValue* external_elements,
                                     HValue* key,
                                     HValue* val,
                                     ElementsKind elements_kind)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetGVNFlag(kChangesSpecializedArrayElements);
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 0) {
      return Representation::External();
    } else {
      bool float_or_double_elements =
          elements_kind() == EXTERNAL_FLOAT_ELEMENTS ||
          elements_kind() == EXTERNAL_DOUBLE_ELEMENTS;
      if (index == 2 && float_or_double_elements) {
        return Representation::Double();
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_
  bool value_is_smi() {
    return IsFastSmiElementsKind(elements_kind_);
  }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    if (value_is_smi()) {
      return false;
    } else {
      return StoringValueNeedsWriteBarrier(value());
    }
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedFastDoubleElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedFastDoubleElement(HValue* elements,
                               HValue* key,
                               HValue* val)
      : index_offset_(0), is_dehoisted_(false) {
    SetOperandAt(0, elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
    SetFlag(kDeoptimizeOnUndefined);
    SetGVNFlag(kChangesDoubleArrayElements);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 1) {
      return Representation::Integer32();
    } else if (index == 2) {
      return Representation::Double();
    } else {
      return Representation::Tagged();
    }
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value());
  }

  bool NeedsCanonicalization();

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastDoubleElement)

 private:
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedSpecializedArrayElement(HValue* external_elements,
                                     HValue* key,
                                     HValue* val,
                                     ElementsKind elements_kind)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetGVNFlag(kChangesSpecializedArrayElements);
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 0) {
      return Representation::External();
    } else {
      bool float_or_double_elements =
          elements_kind() == EXTERNAL_FLOAT_ELEMENTS ||
          elements_kind() == EXTERNAL_DOUBLE_ELEMENTS;
      if (index == 2 && float_or_double_elements) {
        return Representation::Double();
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_
    } else {
      return Representation::Tagged();
    }
  }

  HValue* elements() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  bool NeedsWriteBarrier() {
    return StoringValueNeedsWriteBarrier(value());
  }

  bool NeedsCanonicalization();

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedFastDoubleElement)

 private:
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedSpecializedArrayElement
    : public HTemplateInstruction<3>, public ArrayInstructionInterface {
 public:
  HStoreKeyedSpecializedArrayElement(HValue* external_elements,
                                     HValue* key,
                                     HValue* val,
                                     ElementsKind elements_kind)
      : elements_kind_(elements_kind), index_offset_(0), is_dehoisted_(false) {
    SetGVNFlag(kChangesSpecializedArrayElements);
    SetOperandAt(0, external_elements);
    SetOperandAt(1, key);
    SetOperandAt(2, val);
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    if (index == 0) {
      return Representation::External();
    } else {
      bool float_or_double_elements =
          elements_kind() == EXTERNAL_FLOAT_ELEMENTS ||
          elements_kind() == EXTERNAL_DOUBLE_ELEMENTS;
      if (index == 2 && float_or_double_elements) {
        return Representation::Double();
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_
      } else {
        return Representation::Integer32();
      }
    }
  }

  HValue* external_pointer() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedSpecializedArrayElement)

 private:
  ElementsKind elements_kind_;
  uint32_t index_offset_;
  bool is_dehoisted_;
};


class HStoreKeyedGeneric: public HTemplateInstruction<4> {
 public:
  HStoreKeyedGeneric(HValue* context,
                     HValue* object,
                     HValue* key,
                     HValue* value,
                     StrictModeFlag strict_mode_flag)
      : strict_mode_flag_(strict_mode_flag) {
    SetOperandAt(0, object);
    SetOperandAt(1, key);
    SetOperandAt(2, value);
    SetOperandAt(3, context);
    SetAllSideEffects();
  }

  HValue* object() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* value() { return OperandAt(2); }
  HValue* context() { return OperandAt(3); }
  StrictModeFlag strict_mode_flag() { return strict_mode_flag_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(StoreKeyedGeneric)

 private:
  StrictModeFlag strict_mode_flag_;
};


class HTransitionElementsKind: public HTemplateInstruction<1> {
 public:
  HTransitionElementsKind(HValue* object,
                          Handle<Map> original_map,
                          Handle<Map> transitioned_map)
      : original_map_(original_map),
        transitioned_map_(transitioned_map) {
    SetOperandAt(0, object);
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesElementsKind);
    if (original_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    if (transitioned_map->has_fast_double_elements()) {
      SetGVNFlag(kChangesElementsPointer);
      SetGVNFlag(kChangesNewSpacePromotion);
    }
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  Handle<Map> original_map() { return original_map_; }
  Handle<Map> transitioned_map() { return transitioned_map_; }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(TransitionElementsKind)

 protected:
  virtual bool DataEquals(HValue* other) {
    HTransitionElementsKind* instr = HTransitionElementsKind::cast(other);
    return original_map_.is_identical_to(instr->original_map()) &&
        transitioned_map_.is_identical_to(instr->transitioned_map());
  }

 private:
  Handle<Map> original_map_;
  Handle<Map> transitioned_map_;
};


class HStringAdd: public HBinaryOperation {
 public:
  HStringAdd(HValue* context, HValue* left, HValue* right)
      : HBinaryOperation(context, left, right) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::String();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringAdd)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharCodeAt: public HTemplateInstruction<3> {
 public:
  HStringCharCodeAt(HValue* context, HValue* string, HValue* index) {
    SetOperandAt(0, context);
    SetOperandAt(1, string);
    SetOperandAt(2, index);
    set_representation(Representation::Integer32());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    // The index is supposed to be Integer32.
    return index == 2
        ? Representation::Integer32()
        : Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* string() { return OperandAt(1); }
  HValue* index() { return OperandAt(2); }

  DECLARE_CONCRETE_INSTRUCTION(StringCharCodeAt)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxUtf16CodeUnit);
  }

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringCharFromCode: public HTemplateInstruction<2> {
 public:
  HStringCharFromCode(HValue* context, HValue* char_code) {
    SetOperandAt(0, context);
    SetOperandAt(1, char_code);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return index == 0
        ? Representation::Tagged()
        : Representation::Integer32();
  }
  virtual HType CalculateInferredType();

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual bool DataEquals(HValue* other) { return true; }

  DECLARE_CONCRETE_INSTRUCTION(StringCharFromCode)

  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  // private:
  //  virtual bool IsDeletable() const { return true; }
};


class HStringLength: public HUnaryOperation {
 public:
  explicit HStringLength(HValue* string) : HUnaryOperation(string) {
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    STATIC_ASSERT(String::kMaxLength <= Smi::kMaxValue);
    return HType::Smi();
  }

  DECLARE_CONCRETE_INSTRUCTION(StringLength)

 protected:
  virtual bool DataEquals(HValue* other) { return true; }

  virtual Range* InferRange(Zone* zone) {
    return new(zone) Range(0, String::kMaxLength);
  }

 private:
  virtual bool IsDeletable() const { return true; }
};


class HAllocateObject: public HTemplateInstruction<1> {
 public:
  HAllocateObject(HValue* context, Handle<JSFunction> constructor)
      : constructor_(constructor) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum instance size for which allocations will be inlined.
  static const int kMaxSize = 64 * kPointerSize;

  HValue* context() { return OperandAt(0); }
  Handle<JSFunction> constructor() { return constructor_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(AllocateObject)

 private:
  // TODO(svenpanne) Might be safe, but leave it out until we know for sure.
  //  virtual bool IsDeletable() const { return true; }

  Handle<JSFunction> constructor_;
};


template <int V>
class HMaterializedLiteral: public HTemplateInstruction<V> {
 public:
  HMaterializedLiteral<V>(int index, int depth)
      : literal_index_(index), depth_(depth) {
    this->set_representation(Representation::Tagged());
  }

  int literal_index() const { return literal_index_; }
  int depth() const { return depth_; }

 private:
  virtual bool IsDeletable() const { return true; }

  int literal_index_;
  int depth_;
};


class HFastLiteral: public HMaterializedLiteral<1> {
 public:
  HFastLiteral(HValue* context,
               Handle<JSObject> boilerplate,
               int total_size,
               int literal_index,
               int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        boilerplate_(boilerplate),
        total_size_(total_size) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  // Maximum depth and total number of elements and properties for literal
  // graphs to be considered for fast deep-copying.
  static const int kMaxLiteralDepth = 3;
  static const int kMaxLiteralProperties = 8;

  HValue* context() { return OperandAt(0); }
  Handle<JSObject> boilerplate() const { return boilerplate_; }
  int total_size() const { return total_size_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FastLiteral)

 private:
  Handle<JSObject> boilerplate_;
  int total_size_;
};


class HArrayLiteral: public HMaterializedLiteral<1> {
 public:
  HArrayLiteral(HValue* context,
                Handle<HeapObject> boilerplate_object,
                int length,
                int literal_index,
                int depth)
      : HMaterializedLiteral<1>(literal_index, depth),
        length_(length),
        boilerplate_object_(boilerplate_object) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  ElementsKind boilerplate_elements_kind() const {
    if (!boilerplate_object_->IsJSObject()) {
      return TERMINAL_FAST_ELEMENTS_KIND;
    }
    return Handle<JSObject>::cast(boilerplate_object_)->GetElementsKind();
  }
  Handle<HeapObject> boilerplate_object() const { return boilerplate_object_; }
  int length() const { return length_; }

  bool IsCopyOnWrite() const;

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ArrayLiteral)

 private:
  int length_;
  Handle<HeapObject> boilerplate_object_;
};


class HObjectLiteral: public HMaterializedLiteral<1> {
 public:
  HObjectLiteral(HValue* context,
                 Handle<FixedArray> constant_properties,
                 bool fast_elements,
                 int literal_index,
                 int depth,
                 bool has_function)
      : HMaterializedLiteral<1>(literal_index, depth),
        constant_properties_(constant_properties),
        fast_elements_(fast_elements),
        has_function_(has_function) {
    SetOperandAt(0, context);
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> constant_properties() const {
    return constant_properties_;
  }
  bool fast_elements() const { return fast_elements_; }
  bool has_function() const { return has_function_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(ObjectLiteral)

 private:
  Handle<FixedArray> constant_properties_;
  bool fast_elements_;
  bool has_function_;
};


class HRegExpLiteral: public HMaterializedLiteral<1> {
 public:
  HRegExpLiteral(HValue* context,
                 Handle<FixedArray> literals,
                 Handle<String> pattern,
                 Handle<String> flags,
                 int literal_index)
      : HMaterializedLiteral<1>(literal_index, 0),
        literals_(literals),
        pattern_(pattern),
        flags_(flags) {
    SetOperandAt(0, context);
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  Handle<FixedArray> literals() { return literals_; }
  Handle<String> pattern() { return pattern_; }
  Handle<String> flags() { return flags_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(RegExpLiteral)

 private:
  Handle<FixedArray> literals_;
  Handle<String> pattern_;
  Handle<String> flags_;
};


class HFunctionLiteral: public HTemplateInstruction<1> {
 public:
  HFunctionLiteral(HValue* context,
                   Handle<SharedFunctionInfo> shared,
                   bool pretenure)
      : shared_info_(shared), pretenure_(pretenure) {
    SetOperandAt(0, context);
    set_representation(Representation::Tagged());
    SetGVNFlag(kChangesNewSpacePromotion);
  }

  HValue* context() { return OperandAt(0); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }
  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(FunctionLiteral)

  Handle<SharedFunctionInfo> shared_info() const { return shared_info_; }
  bool pretenure() const { return pretenure_; }

 private:
  virtual bool IsDeletable() const { return true; }

  Handle<SharedFunctionInfo> shared_info_;
  bool pretenure_;
};


class HTypeof: public HTemplateInstruction<2> {
 public:
  explicit HTypeof(HValue* context, HValue* value) {
    SetOperandAt(0, context);
    SetOperandAt(1, value);
    set_representation(Representation::Tagged());
  }

  HValue* context() { return OperandAt(0); }
  HValue* value() { return OperandAt(1); }

  virtual HValue* Canonicalize();
  virtual void PrintDataTo(StringStream* stream);

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(Typeof)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HToFastProperties: public HUnaryOperation {
 public:
  explicit HToFastProperties(HValue* value) : HUnaryOperation(value) {
    // This instruction is not marked as having side effects, but
    // changes the map of the input operand. Use it only when creating
    // object literals.
    ASSERT(value->IsObjectLiteral() || value->IsFastLiteral());
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ToFastProperties)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HValueOf: public HUnaryOperation {
 public:
  explicit HValueOf(HValue* value) : HUnaryOperation(value) {
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ValueOf)

 private:
  virtual bool IsDeletable() const { return true; }
};


class HDateField: public HUnaryOperation {
 public:
  HDateField(HValue* date, Smi* index)
      : HUnaryOperation(date), index_(index) {
    set_representation(Representation::Tagged());
  }

  Smi* index() const { return index_; }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(DateField)

 private:
  Smi* index_;
};


class HDeleteProperty: public HBinaryOperation {
 public:
  HDeleteProperty(HValue* context, HValue* obj, HValue* key)
      : HBinaryOperation(context, obj, key) {
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType();

  DECLARE_CONCRETE_INSTRUCTION(DeleteProperty)

  HValue* object() { return left(); }
  HValue* key() { return right(); }
};


class HIn: public HTemplateInstruction<3> {
 public:
  HIn(HValue* context, HValue* key, HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, key);
    SetOperandAt(2, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  HValue* context() { return OperandAt(0); }
  HValue* key() { return OperandAt(1); }
  HValue* object() { return OperandAt(2); }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual HType CalculateInferredType() {
    return HType::Boolean();
  }

  virtual void PrintDataTo(StringStream* stream);

  DECLARE_CONCRETE_INSTRUCTION(In)
};


class HCheckMapValue: public HTemplateInstruction<2> {
 public:
  HCheckMapValue(HValue* value,
                 HValue* map) {
    SetOperandAt(0, value);
    SetOperandAt(1, map);
    set_representation(Representation::Tagged());
    SetFlag(kUseGVN);
    SetGVNFlag(kDependsOnMaps);
    SetGVNFlag(kDependsOnElementsKind);
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  HValue* value() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }

  DECLARE_CONCRETE_INSTRUCTION(CheckMapValue)

 protected:
  virtual bool DataEquals(HValue* other) {
    return true;
  }
};


class HForInPrepareMap : public HTemplateInstruction<2> {
 public:
  HForInPrepareMap(HValue* context,
                   HValue* object) {
    SetOperandAt(0, context);
    SetOperandAt(1, object);
    set_representation(Representation::Tagged());
    SetAllSideEffects();
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* context() { return OperandAt(0); }
  HValue* enumerable() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInPrepareMap);
};


class HForInCacheArray : public HTemplateInstruction<2> {
 public:
  HForInCacheArray(HValue* enumerable,
                   HValue* keys,
                   int idx) : idx_(idx) {
    SetOperandAt(0, enumerable);
    SetOperandAt(1, keys);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* enumerable() { return OperandAt(0); }
  HValue* map() { return OperandAt(1); }
  int idx() { return idx_; }

  HForInCacheArray* index_cache() {
    return index_cache_;
  }

  void set_index_cache(HForInCacheArray* index_cache) {
    index_cache_ = index_cache;
  }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(ForInCacheArray);

 private:
  int idx_;
  HForInCacheArray* index_cache_;
};


class HLoadFieldByIndex : public HTemplateInstruction<2> {
 public:
  HLoadFieldByIndex(HValue* object,
                    HValue* index) {
    SetOperandAt(0, object);
    SetOperandAt(1, index);
    set_representation(Representation::Tagged());
  }

  virtual Representation RequiredInputRepresentation(int index) {
    return Representation::Tagged();
  }

  HValue* object() { return OperandAt(0); }
  HValue* index() { return OperandAt(1); }

  virtual void PrintDataTo(StringStream* stream);

  virtual HType CalculateInferredType() {
    return HType::Tagged();
  }

  DECLARE_CONCRETE_INSTRUCTION(LoadFieldByIndex);

 private:
  virtual bool IsDeletable() const { return true; }
};


#undef DECLARE_INSTRUCTION
#undef DECLARE_CONCRETE_INSTRUCTION

} }  // namespace v8::internal

#endif  // V8_HYDROGEN_INSTRUCTIONS_H_