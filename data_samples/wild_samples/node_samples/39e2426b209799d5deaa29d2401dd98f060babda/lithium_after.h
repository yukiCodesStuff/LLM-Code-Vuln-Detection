  void AddInlinedClosure(Handle<JSFunction> closure) {
    inlined_closures_.Add(closure, zone());
  }

  Zone* zone() const { return info_->zone(); }

  Handle<Code> Codegen();

 protected:
  LChunk(CompilationInfo* info, HGraph* graph)
      : spill_slot_count_(0),
        info_(info),
        graph_(graph),
        instructions_(32, graph->zone()),
        pointer_maps_(8, graph->zone()),
        inlined_closures_(1, graph->zone()) { }

  int spill_slot_count_;

 private:
  CompilationInfo* info_;
  HGraph* const graph_;
  ZoneList<LInstruction*> instructions_;
  ZoneList<LPointerMap*> pointer_maps_;
  ZoneList<Handle<JSFunction> > inlined_closures_;
};


} }  // namespace v8::internal

#endif  // V8_LITHIUM_H_