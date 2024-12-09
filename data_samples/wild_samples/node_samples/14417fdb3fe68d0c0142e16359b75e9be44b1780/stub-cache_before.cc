  } else if (object->IsBoolean()) {
    check = BOOLEAN_CHECK;
  }

  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, Code::CONSTANT_FUNCTION, extra_state,
                                    cache_holder, argc);
  Handle<Object> probe(map_holder->map()->FindInCodeCache(*name, flags));
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  CallStubCompiler compiler(isolate_, argc, kind, extra_state, cache_holder);
  Handle<Code> code =
      compiler.CompileCallConstant(object, holder, function, name, check);
  code->set_check_type(check);
  ASSERT_EQ(flags, code->flags());
  PROFILE(isolate_,
          CodeCreateEvent(CALL_LOGGER_TAG(kind, CALL_IC_TAG), *code, *name));
  GDBJIT(AddCode(GDBJITInterface::CALL_IC, *name, *code));
  JSObject::UpdateMapCodeCache(map_holder, name, code);
  return code;
}


Handle<Code> StubCache::ComputeCallField(int argc,
                                         Code::Kind kind,
                                         Code::ExtraICState extra_state,
                                         Handle<String> name,
                                         Handle<Object> object,
                                         Handle<JSObject> holder,
                                         int index) {
  // Compute the check type and the map.
  InlineCacheHolderFlag cache_holder =
      IC::GetCodeCacheForObject(*object, *holder);
  Handle<JSObject> map_holder(IC::GetCodeCacheHolder(*object, cache_holder));

  // TODO(1233596): We cannot do receiver map check for non-JS objects
  // because they may be represented as immediates without a
  // map. Instead, we check against the map in the holder.
  if (object->IsNumber() || object->IsBoolean() || object->IsString()) {
    object = holder;
  }
  if (object->IsNumber() || object->IsBoolean() || object->IsString()) {
    object = holder;
  }

  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, Code::FIELD, extra_state,
                                    cache_holder, argc);
  Handle<Object> probe(map_holder->map()->FindInCodeCache(*name, flags));
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  CallStubCompiler compiler(isolate_, argc, kind, extra_state, cache_holder);
  Handle<Code> code =
      compiler.CompileCallField(Handle<JSObject>::cast(object),
                                holder, index, name);
  ASSERT_EQ(flags, code->flags());
  PROFILE(isolate_,
          CodeCreateEvent(CALL_LOGGER_TAG(kind, CALL_IC_TAG), *code, *name));
  GDBJIT(AddCode(GDBJITInterface::CALL_IC, *name, *code));
  JSObject::UpdateMapCodeCache(map_holder, name, code);
  return code;
}


Handle<Code> StubCache::ComputeCallInterceptor(int argc,
                                               Code::Kind kind,
                                               Code::ExtraICState extra_state,
                                               Handle<String> name,
                                               Handle<Object> object,
                                               Handle<JSObject> holder) {
  // Compute the check type and the map.
  InlineCacheHolderFlag cache_holder =
      IC::GetCodeCacheForObject(*object, *holder);
  Handle<JSObject> map_holder(IC::GetCodeCacheHolder(*object, cache_holder));

  // TODO(1233596): We cannot do receiver map check for non-JS objects
  // because they may be represented as immediates without a
  // map. Instead, we check against the map in the holder.
  if (object->IsNumber() || object->IsBoolean() || object->IsString()) {
    object = holder;
  }
  if (object->IsNumber() || object->IsBoolean() || object->IsString()) {
    object = holder;
  }

  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, Code::INTERCEPTOR, extra_state,
                                    cache_holder, argc);
  Handle<Object> probe(map_holder->map()->FindInCodeCache(*name, flags));
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  CallStubCompiler compiler(isolate(), argc, kind, extra_state, cache_holder);
  Handle<Code> code =
      compiler.CompileCallInterceptor(Handle<JSObject>::cast(object),
                                      holder, name);
  ASSERT_EQ(flags, code->flags());
  PROFILE(isolate(),
          CodeCreateEvent(CALL_LOGGER_TAG(kind, CALL_IC_TAG), *code, *name));
  GDBJIT(AddCode(GDBJITInterface::CALL_IC, *name, *code));
  JSObject::UpdateMapCodeCache(map_holder, name, code);
  return code;
}


Handle<Code> StubCache::ComputeCallGlobal(int argc,
                                          Code::Kind kind,
                                          Code::ExtraICState extra_state,
                                          Handle<String> name,
                                          Handle<JSObject> receiver,
                                          Handle<GlobalObject> holder,
                                          Handle<JSGlobalPropertyCell> cell,
                                          Handle<JSFunction> function) {
  InlineCacheHolderFlag cache_holder =
      IC::GetCodeCacheForObject(*receiver, *holder);
  Handle<JSObject> map_holder(IC::GetCodeCacheHolder(*receiver, cache_holder));
  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, Code::NORMAL, extra_state,
                                    cache_holder, argc);
  Handle<Object> probe(map_holder->map()->FindInCodeCache(*name, flags));
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  CallStubCompiler compiler(isolate(), argc, kind, extra_state, cache_holder);
  Handle<Code> code =
      compiler.CompileCallGlobal(receiver, holder, cell, function, name);
  ASSERT_EQ(flags, code->flags());
  PROFILE(isolate(),
          CodeCreateEvent(CALL_LOGGER_TAG(kind, CALL_IC_TAG), *code, *name));
  GDBJIT(AddCode(GDBJITInterface::CALL_IC, *name, *code));
  JSObject::UpdateMapCodeCache(map_holder, name, code);
  return code;
}


static void FillCache(Isolate* isolate, Handle<Code> code) {
  Handle<UnseededNumberDictionary> dictionary =
      UnseededNumberDictionary::Set(isolate->factory()->non_monomorphic_cache(),
                                    code->flags(),
                                    code);
  isolate->heap()->public_set_non_monomorphic_cache(*dictionary);
}


Code* StubCache::FindCallInitialize(int argc,
                                    RelocInfo::Mode mode,
                                    Code::Kind kind) {
  Code::ExtraICState extra_state =
      CallICBase::StringStubState::encode(DEFAULT_STRING_STUB) |
      CallICBase::Contextual::encode(mode == RelocInfo::CODE_TARGET_CONTEXT);
  Code::Flags flags =
      Code::ComputeFlags(kind, UNINITIALIZED, extra_state, Code::NORMAL, argc);

  // Use raw_unchecked... so we don't get assert failures during GC.
  UnseededNumberDictionary* dictionary =
      isolate()->heap()->raw_unchecked_non_monomorphic_cache();
  int entry = dictionary->FindEntry(isolate(), flags);
  ASSERT(entry != -1);
  Object* code = dictionary->ValueAt(entry);
  // This might be called during the marking phase of the collector
  // hence the unchecked cast.
  return reinterpret_cast<Code*>(code);
}


Handle<Code> StubCache::ComputeCallInitialize(int argc,
                                              RelocInfo::Mode mode,
                                              Code::Kind kind) {
  Code::ExtraICState extra_state =
      CallICBase::StringStubState::encode(DEFAULT_STRING_STUB) |
      CallICBase::Contextual::encode(mode == RelocInfo::CODE_TARGET_CONTEXT);
  Code::Flags flags =
      Code::ComputeFlags(kind, UNINITIALIZED, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallInitialize(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallInitialize(int argc, RelocInfo::Mode mode) {
  return ComputeCallInitialize(argc, mode, Code::CALL_IC);
}


Handle<Code> StubCache::ComputeKeyedCallInitialize(int argc) {
  return ComputeCallInitialize(argc, RelocInfo::CODE_TARGET,
                               Code::KEYED_CALL_IC);
}


Handle<Code> StubCache::ComputeCallPreMonomorphic(
    int argc,
    Code::Kind kind,
    Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, PREMONOMORPHIC, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallPreMonomorphic(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallNormal(int argc,
                                          Code::Kind kind,
                                          Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, MONOMORPHIC, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallNormal(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallArguments(int argc, Code::Kind kind) {
  ASSERT(kind == Code::KEYED_CALL_IC);
  Code::Flags flags =
      Code::ComputeFlags(kind, MEGAMORPHIC, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallArguments(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallMegamorphic(
    int argc,
    Code::Kind kind,
    Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, MEGAMORPHIC, extra_state,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallMegamorphic(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallMiss(int argc,
                                        Code::Kind kind,
                                        Code::ExtraICState extra_state) {
  // MONOMORPHIC_PROTOTYPE_FAILURE state is used to make sure that miss stubs
  // and monomorphic stubs are not mixed up together in the stub cache.
  Code::Flags flags =
      Code::ComputeFlags(kind, MONOMORPHIC_PROTOTYPE_FAILURE, extra_state,
                         Code::NORMAL, argc, OWN_MAP);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallMiss(flags);
  FillCache(isolate_, code);
  return code;
}


#ifdef ENABLE_DEBUGGER_SUPPORT
Handle<Code> StubCache::ComputeCallDebugBreak(int argc,
                                              Code::Kind kind) {
  // Extra IC state is irrelevant for debug break ICs. They jump to
  // the actual call ic to carry out the work.
  Code::Flags flags =
      Code::ComputeFlags(kind, DEBUG_BREAK, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallDebugBreak(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallDebugPrepareStepIn(int argc,
                                                      Code::Kind kind) {
  // Extra IC state is irrelevant for debug break ICs. They jump to
  // the actual call ic to carry out the work.
  Code::Flags flags =
      Code::ComputeFlags(kind, DEBUG_PREPARE_STEP_IN, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallDebugPrepareStepIn(flags);
  FillCache(isolate_, code);
  return code;
}
#endif


void StubCache::Clear() {
  Code* empty = isolate_->builtins()->builtin(Builtins::kIllegal);
  for (int i = 0; i < kPrimaryTableSize; i++) {
    primary_[i].key = heap()->empty_string();
    primary_[i].value = empty;
  }
                                          Handle<JSFunction> function) {
  InlineCacheHolderFlag cache_holder =
      IC::GetCodeCacheForObject(*receiver, *holder);
  Handle<JSObject> map_holder(IC::GetCodeCacheHolder(*receiver, cache_holder));
  Code::Flags flags =
      Code::ComputeMonomorphicFlags(kind, Code::NORMAL, extra_state,
                                    cache_holder, argc);
  Handle<Object> probe(map_holder->map()->FindInCodeCache(*name, flags));
  if (probe->IsCode()) return Handle<Code>::cast(probe);

  CallStubCompiler compiler(isolate(), argc, kind, extra_state, cache_holder);
  Handle<Code> code =
      compiler.CompileCallGlobal(receiver, holder, cell, function, name);
  ASSERT_EQ(flags, code->flags());
  PROFILE(isolate(),
          CodeCreateEvent(CALL_LOGGER_TAG(kind, CALL_IC_TAG), *code, *name));
  GDBJIT(AddCode(GDBJITInterface::CALL_IC, *name, *code));
  JSObject::UpdateMapCodeCache(map_holder, name, code);
  return code;
}


static void FillCache(Isolate* isolate, Handle<Code> code) {
  Handle<UnseededNumberDictionary> dictionary =
      UnseededNumberDictionary::Set(isolate->factory()->non_monomorphic_cache(),
                                    code->flags(),
                                    code);
  isolate->heap()->public_set_non_monomorphic_cache(*dictionary);
}


Code* StubCache::FindCallInitialize(int argc,
                                    RelocInfo::Mode mode,
                                    Code::Kind kind) {
  Code::ExtraICState extra_state =
      CallICBase::StringStubState::encode(DEFAULT_STRING_STUB) |
      CallICBase::Contextual::encode(mode == RelocInfo::CODE_TARGET_CONTEXT);
  Code::Flags flags =
      Code::ComputeFlags(kind, UNINITIALIZED, extra_state, Code::NORMAL, argc);

  // Use raw_unchecked... so we don't get assert failures during GC.
  UnseededNumberDictionary* dictionary =
      isolate()->heap()->raw_unchecked_non_monomorphic_cache();
  int entry = dictionary->FindEntry(isolate(), flags);
  ASSERT(entry != -1);
  Object* code = dictionary->ValueAt(entry);
  // This might be called during the marking phase of the collector
  // hence the unchecked cast.
  return reinterpret_cast<Code*>(code);
}


Handle<Code> StubCache::ComputeCallInitialize(int argc,
                                              RelocInfo::Mode mode,
                                              Code::Kind kind) {
  Code::ExtraICState extra_state =
      CallICBase::StringStubState::encode(DEFAULT_STRING_STUB) |
      CallICBase::Contextual::encode(mode == RelocInfo::CODE_TARGET_CONTEXT);
  Code::Flags flags =
      Code::ComputeFlags(kind, UNINITIALIZED, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallInitialize(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallInitialize(int argc, RelocInfo::Mode mode) {
  return ComputeCallInitialize(argc, mode, Code::CALL_IC);
}


Handle<Code> StubCache::ComputeKeyedCallInitialize(int argc) {
  return ComputeCallInitialize(argc, RelocInfo::CODE_TARGET,
                               Code::KEYED_CALL_IC);
}


Handle<Code> StubCache::ComputeCallPreMonomorphic(
    int argc,
    Code::Kind kind,
    Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, PREMONOMORPHIC, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallPreMonomorphic(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallNormal(int argc,
                                          Code::Kind kind,
                                          Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, MONOMORPHIC, extra_state, Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallNormal(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallArguments(int argc, Code::Kind kind) {
  ASSERT(kind == Code::KEYED_CALL_IC);
  Code::Flags flags =
      Code::ComputeFlags(kind, MEGAMORPHIC, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallArguments(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallMegamorphic(
    int argc,
    Code::Kind kind,
    Code::ExtraICState extra_state) {
  Code::Flags flags =
      Code::ComputeFlags(kind, MEGAMORPHIC, extra_state,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallMegamorphic(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallMiss(int argc,
                                        Code::Kind kind,
                                        Code::ExtraICState extra_state) {
  // MONOMORPHIC_PROTOTYPE_FAILURE state is used to make sure that miss stubs
  // and monomorphic stubs are not mixed up together in the stub cache.
  Code::Flags flags =
      Code::ComputeFlags(kind, MONOMORPHIC_PROTOTYPE_FAILURE, extra_state,
                         Code::NORMAL, argc, OWN_MAP);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallMiss(flags);
  FillCache(isolate_, code);
  return code;
}


#ifdef ENABLE_DEBUGGER_SUPPORT
Handle<Code> StubCache::ComputeCallDebugBreak(int argc,
                                              Code::Kind kind) {
  // Extra IC state is irrelevant for debug break ICs. They jump to
  // the actual call ic to carry out the work.
  Code::Flags flags =
      Code::ComputeFlags(kind, DEBUG_BREAK, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallDebugBreak(flags);
  FillCache(isolate_, code);
  return code;
}


Handle<Code> StubCache::ComputeCallDebugPrepareStepIn(int argc,
                                                      Code::Kind kind) {
  // Extra IC state is irrelevant for debug break ICs. They jump to
  // the actual call ic to carry out the work.
  Code::Flags flags =
      Code::ComputeFlags(kind, DEBUG_PREPARE_STEP_IN, Code::kNoExtraICState,
                         Code::NORMAL, argc);
  Handle<UnseededNumberDictionary> cache =
      isolate_->factory()->non_monomorphic_cache();
  int entry = cache->FindEntry(isolate_, flags);
  if (entry != -1) return Handle<Code>(Code::cast(cache->ValueAt(entry)));

  StubCompiler compiler(isolate_);
  Handle<Code> code = compiler.CompileCallDebugPrepareStepIn(flags);
  FillCache(isolate_, code);
  return code;
}
#endif


void StubCache::Clear() {
  Code* empty = isolate_->builtins()->builtin(Builtins::kIllegal);
  for (int i = 0; i < kPrimaryTableSize; i++) {
    primary_[i].key = heap()->empty_string();
    primary_[i].value = empty;
  }