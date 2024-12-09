}


LLabel* LChunk::GetLabel(int block_id) const {
  HBasicBlock* block = graph_->blocks()->at(block_id);
  int first_instruction = block->first_instruction_index();
  return LLabel::cast(instructions_[first_instruction]);