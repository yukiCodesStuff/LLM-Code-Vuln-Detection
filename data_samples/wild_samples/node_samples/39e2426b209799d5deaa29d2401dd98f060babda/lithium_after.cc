  for (int i = 0; i < pointer_operands_.length(); ++i) {
    if (i != 0) stream->Add(";");
    pointer_operands_[i]->PrintTo(stream);
  }
  stream->Add("} @%d", position());
}


LLabel* LChunk::GetLabel(int block_id) const {
  HBasicBlock* block = graph_->blocks()->at(block_id);
  int first_instruction = block->first_instruction_index();
  return LLabel::cast(instructions_[first_instruction]);
}