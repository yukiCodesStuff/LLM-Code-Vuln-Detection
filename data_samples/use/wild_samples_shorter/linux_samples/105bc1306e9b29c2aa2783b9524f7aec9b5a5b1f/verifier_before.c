	[PTR_TO_PACKET]		= "pkt",
	[PTR_TO_PACKET_META]	= "pkt_meta",
	[PTR_TO_PACKET_END]	= "pkt_end",
};

static char slot_type_char[] = {
	[STACK_INVALID]	= '?',
 */
static void __mark_reg_known(struct bpf_reg_state *reg, u64 imm)
{
	reg->id = 0;
	reg->var_off = tnum_const(imm);
	reg->smin_value = (s64)imm;
	reg->smax_value = (s64)imm;
	reg->umin_value = imm;
static void __mark_reg_const_zero(struct bpf_reg_state *reg)
{
	__mark_reg_known(reg, 0);
	reg->off = 0;
	reg->type = SCALAR_VALUE;
}

static void mark_reg_known_zero(struct bpf_verifier_env *env,
/* Mark a register as having a completely unknown (scalar) value. */
static void __mark_reg_unknown(struct bpf_reg_state *reg)
{
	reg->type = SCALAR_VALUE;
	reg->id = 0;
	reg->off = 0;
	reg->var_off = tnum_unknown;
	reg->frameno = 0;
	__mark_reg_unbounded(reg);
}
	case PTR_TO_PACKET:
	case PTR_TO_PACKET_META:
	case PTR_TO_PACKET_END:
	case CONST_PTR_TO_MAP:
		return true;
	default:
		return false;
	case BPF_PROG_TYPE_LWT_XMIT:
	case BPF_PROG_TYPE_SK_SKB:
	case BPF_PROG_TYPE_SK_MSG:
		if (meta)
			return meta->pkt_access;

		env->seen_direct_write = true;
	return -EACCES;
}

static bool __is_pointer_value(bool allow_ptr_leaks,
			       const struct bpf_reg_state *reg)
{
	if (allow_ptr_leaks)
		 * right in front, treat it the very same way.
		 */
		return check_pkt_ptr_alignment(env, reg, off, size, strict);
	case PTR_TO_MAP_VALUE:
		pointer_desc = "value ";
		break;
	case PTR_TO_CTX:
			else
				mark_reg_known_zero(env, regs,
						    value_regno);
			regs[value_regno].id = 0;
			regs[value_regno].off = 0;
			regs[value_regno].range = 0;
			regs[value_regno].type = reg_type;
		}

	} else if (reg->type == PTR_TO_STACK) {
		err = check_packet_access(env, regno, off, size, false);
		if (!err && t == BPF_READ && value_regno >= 0)
			mark_reg_unknown(env, regs, value_regno);
	} else {
		verbose(env, "R%d invalid mem access '%s'\n", regno,
			reg_type_str[reg->type]);
		return -EACCES;
	case PTR_TO_PACKET_META:
		return check_packet_access(env, regno, reg->off, access_size,
					   zero_size_allowed);
	case PTR_TO_MAP_VALUE:
		return check_map_access(env, regno, reg->off, access_size,
					zero_size_allowed);
	default: /* scalar_value|ptr_to_stack or invalid ptr */
			regs[BPF_REG_0].type = PTR_TO_MAP_VALUE_OR_NULL;
		/* There is no offset yet applied, variable or fixed */
		mark_reg_known_zero(env, regs, BPF_REG_0);
		regs[BPF_REG_0].off = 0;
		/* remember map_ptr, so that check_map_access()
		 * can check 'value_size' boundary of memory access
		 * to map element returned from bpf_map_lookup_elem()
		 */
	case PTR_TO_CTX:
	case CONST_PTR_TO_MAP:
	case PTR_TO_PACKET_END:
		/* Only valid matches are exact, which memcmp() above
		 * would have accepted
		 */
	default: