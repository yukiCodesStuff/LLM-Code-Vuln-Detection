    const unsigned char* endp) {
  state->flags |= 512;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_17(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_store_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  state->http_major = match;
  return 0;
}

int llhttp__internal__c_store_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  state->http_minor = match;
  return 0;
}

int llhttp__internal__c_is_equal_method_1(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method == 33;
}

int llhttp__internal__c_update_status_code(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->status_code = 0;
  return 0;
}

int llhttp__internal__c_mul_add_status_code(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  /* Multiplication overflow */
  if (state->status_code > 0xffff / 10) {
    return 1;
  }
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_16: {
    switch (llhttp__internal__c_or_flags_16(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    const unsigned char* endp) {
  state->flags |= 512;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_17(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_store_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  state->http_major = match;
  return 0;
}

int llhttp__internal__c_store_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  state->http_minor = match;
  return 0;
}

int llhttp__internal__c_is_equal_method_1(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method == 33;
}

int llhttp__internal__c_update_status_code(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->status_code = 0;
  return 0;
}

int llhttp__internal__c_mul_add_status_code(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  /* Multiplication overflow */
  if (state->status_code > 0xffff / 10) {
    return 1;
  }
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_16: {
    switch (llhttp__internal__c_or_flags_16(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }