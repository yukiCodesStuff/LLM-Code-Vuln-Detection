  virtual HValue* GetKey() = 0;
  virtual void SetKey(HValue* key) = 0;
  virtual void SetIndexOffset(uint32_t index_offset) = 0;
  virtual bool IsDehoisted() = 0;
  virtual void SetDehoisted(bool is_dehoisted) = 0;
  virtual ~ArrayInstructionInterface() { };
};
  void SetIndexOffset(uint32_t index_offset) {
    bit_field_ = IndexOffsetField::update(bit_field_, index_offset);
  }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return IsDehoistedField::decode(bit_field_); }
  void SetDehoisted(bool is_dehoisted) {
  HValue* dependency() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }
  }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }
  HValue* value() { return OperandAt(2); }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }
  ElementsKind elements_kind() const { return elements_kind_; }
  uint32_t index_offset() { return index_offset_; }
  void SetIndexOffset(uint32_t index_offset) { index_offset_ = index_offset; }
  HValue* GetKey() { return key(); }
  void SetKey(HValue* key) { SetOperandAt(1, key); }
  bool IsDehoisted() { return is_dehoisted_; }
  void SetDehoisted(bool is_dehoisted) { is_dehoisted_ = is_dehoisted; }