    else if (*type == CCV_CODE_POINT) {
      r = add_code_range(&(cc->mbuf), env, *vs, *vs);
      if (r < 0) return r;
    }
  }

  if (*state != CCS_START)
    *state = CCS_VALUE;

  *type  = CCV_CLASS;
  return 0;
}

static int
next_state_val(CClassNode* cc, OnigCodePoint *vs, OnigCodePoint v,
	       int* vs_israw, int v_israw,
	       enum CCVALTYPE intype, enum CCVALTYPE* type,
	       enum CCSTATE* state, ScanEnv* env)
{
  int r;

  switch (*state) {
  case CCS_VALUE:
    if (*type == CCV_SB)
    {
    if (*vs > 0xff)
      return ONIGERR_INVALID_CODE_POINT_VALUE;
      BITSET_SET_BIT(cc->bs, (int )(*vs));
    }