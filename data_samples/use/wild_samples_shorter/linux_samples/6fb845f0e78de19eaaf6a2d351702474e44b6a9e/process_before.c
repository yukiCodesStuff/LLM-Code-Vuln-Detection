static int get_frame_info(struct mips_frame_info *info)
{
	bool is_mmips = IS_ENABLED(CONFIG_CPU_MICROMIPS);
	union mips_instruction insn, *ip, *ip_end;
	const unsigned int max_insns = 128;
	unsigned int last_insn_size = 0;
	unsigned int i;
	bool saw_jump = false;
	if (!ip)
		goto err;

	ip_end = (void *)ip + info->func_size;

	for (i = 0; i < max_insns && ip < ip_end; i++) {
		ip = (void *)ip + last_insn_size;
		if (is_mmips && mm_insn_16bit(ip->halfword[0])) {
			insn.word = ip->halfword[0] << 16;
			last_insn_size = 2;
		} else if (is_mmips) {