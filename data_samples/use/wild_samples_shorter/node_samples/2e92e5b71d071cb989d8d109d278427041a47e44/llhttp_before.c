  s_n_llhttp__internal__n_header_value_lws,
  s_n_llhttp__internal__n_header_value_almost_done,
  s_n_llhttp__internal__n_header_value_lenient,
  s_n_llhttp__internal__n_error_23,
  s_n_llhttp__internal__n_header_value_lenient_failed,
  s_n_llhttp__internal__n_header_value_otherwise,
  s_n_llhttp__internal__n_header_value_connection_token,
  s_n_llhttp__internal__n_header_value_connection_ws,
  s_n_llhttp__internal__n_header_value_connection_1,
  return 0;
}

int llhttp__internal__c_update_header_state_2(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 6;
  return 0;
}

int llhttp__internal__c_update_header_state_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 0;
  return 0;
}

int llhttp__internal__c_update_header_state_5(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 5;
  return 0;
}

int llhttp__internal__c_update_header_state_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 7;
  return (state->flags & 8) == 8;
}

int llhttp__internal__c_test_lenient_flags_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 8) == 8;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_21;
        }
      }
      /* UNREACHABLE */;
      abort();
      }
      switch (*p) {
        case 9: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
        case ' ': {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_load_header_state_3;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_22;
        }
      }
      /* UNREACHABLE */;
      abort();
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_23:
    s_n_llhttp__internal__n_error_23: {
      state->error = 0x19;
      state->reason = "Missing expected CR after header value";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      return s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lenient_failed:
    s_n_llhttp__internal__n_header_value_lenient_failed: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lenient_failed;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_24;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_otherwise:
    s_n_llhttp__internal__n_header_value_otherwise: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_otherwise;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
        }
        case ',': {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_header_state_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_4;
        }
      }
      /* UNREACHABLE */;
      abort();
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_1;
        }
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_5;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_2;
        }
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_6;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_3;
        }
          goto s_n_llhttp__internal__n_header_value_te_token_ows;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_8;
        }
      }
      /* UNREACHABLE */;
      abort();
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_7;
        }
        case 13: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_7;
        }
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_chunked_last;
        }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws_almost_done;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_9;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_20: {
    state->error = 0xb;
    state->reason = "Empty Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
  s_n_llhttp__internal__n_invoke_load_header_state: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 2:
        goto s_n_llhttp__internal__n_error_20;
      default:
        goto s_n_llhttp__internal__n_invoke_load_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_21: {
    state->error = 0x2;
    state->reason = "Expected LF after CR";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_1: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
  s_n_llhttp__internal__n_invoke_or_flags_7: {
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_8: {
    switch (llhttp__internal__c_or_flags_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_9: {
    switch (llhttp__internal__c_or_flags_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_3: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_22: {
    state->error = 0x3;
    state->reason = "Missing expected LF after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_23;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_23;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_24: {
    state->error = 0xa;
    state->reason = "Invalid header value char";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_3: {
    switch (llhttp__internal__c_test_lenient_flags_2(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_header_value_lenient;
      default:
        goto s_n_llhttp__internal__n_header_value_lenient_failed;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_3: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
  s_n_llhttp__internal__n_invoke_or_flags_11: {
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_12: {
    switch (llhttp__internal__c_or_flags_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_13: {
    switch (llhttp__internal__c_or_flags_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_4: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_4: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_token;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_2: {
    switch (llhttp__internal__c_update_header_state_2(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_5: {
    switch (llhttp__internal__c_update_header_state_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_6: {
    switch (llhttp__internal__c_update_header_state_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_7: {
    switch (llhttp__internal__c_update_header_state_7(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_otherwise;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_4: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_7;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
  s_n_llhttp__internal__n_invoke_load_type_1: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_4;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_8: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_5: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_8;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
  s_n_llhttp__internal__n_invoke_load_type_2: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_5;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    /* UNREACHABLE */;
  s_n_llhttp__internal__n_invoke_or_flags_18: {
    switch (llhttp__internal__c_or_flags_18(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_8;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_9: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_10: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_6: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_method_1;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major;
  s_n_llhttp__internal__n_invoke_store_http_minor: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_6;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_7: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_res_http_end;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major_1;
  s_n_llhttp__internal__n_invoke_store_http_minor_1: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_7;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_header_value_lws,
  s_n_llhttp__internal__n_header_value_almost_done,
  s_n_llhttp__internal__n_header_value_lenient,
  s_n_llhttp__internal__n_error_17,
  s_n_llhttp__internal__n_header_value_lenient_failed,
  s_n_llhttp__internal__n_header_value_otherwise,
  s_n_llhttp__internal__n_header_value_connection_token,
  s_n_llhttp__internal__n_header_value_connection_ws,
  s_n_llhttp__internal__n_header_value_connection_1,
  return 0;
}

int llhttp__internal__c_update_header_state_2(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 6;
  return 0;
}

int llhttp__internal__c_update_header_state_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 0;
  return 0;
}

int llhttp__internal__c_update_header_state_5(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 5;
  return 0;
}

int llhttp__internal__c_update_header_state_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 7;
  return (state->flags & 8) == 8;
}

int llhttp__internal__c_test_lenient_flags_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 8) == 8;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
      }
      switch (*p) {
        case 9: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
        case ' ': {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_load_header_state_3;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_16;
        }
      }
      /* UNREACHABLE */;
      abort();
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_17:
    s_n_llhttp__internal__n_error_17: {
      state->error = 0x19;
      state->reason = "Missing expected CR after header value";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      return s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lenient_failed:
    s_n_llhttp__internal__n_header_value_lenient_failed: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lenient_failed;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_18;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_otherwise:
    s_n_llhttp__internal__n_header_value_otherwise: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_otherwise;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
        }
        case ',': {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_header_state_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_4;
        }
      }
      /* UNREACHABLE */;
      abort();
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_1;
        }
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_5;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_2;
        }
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_6;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_connection_3;
        }
          goto s_n_llhttp__internal__n_header_value_te_token_ows;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_8;
        }
      }
      /* UNREACHABLE */;
      abort();
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_7;
        }
        case 13: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_7;
        }
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_chunked_last;
        }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws_almost_done;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_9;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_15: {
    state->error = 0xb;
    state->reason = "Empty Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
  s_n_llhttp__internal__n_invoke_load_header_state: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 2:
        goto s_n_llhttp__internal__n_error_15;
      default:
        goto s_n_llhttp__internal__n_invoke_load_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_1: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
  s_n_llhttp__internal__n_invoke_or_flags_7: {
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_8: {
    switch (llhttp__internal__c_or_flags_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_9: {
    switch (llhttp__internal__c_or_flags_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_3: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_16: {
    state->error = 0x3;
    state->reason = "Missing expected LF after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_17;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_17;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_18: {
    state->error = 0xa;
    state->reason = "Invalid header value char";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_3: {
    switch (llhttp__internal__c_test_lenient_flags_2(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_header_value_lenient;
      default:
        goto s_n_llhttp__internal__n_header_value_lenient_failed;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_3: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
  s_n_llhttp__internal__n_invoke_or_flags_11: {
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_12: {
    switch (llhttp__internal__c_or_flags_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_or_flags_13: {
    switch (llhttp__internal__c_or_flags_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_4: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_4: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_token;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_2: {
    switch (llhttp__internal__c_update_header_state_2(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_5: {
    switch (llhttp__internal__c_update_header_state_5(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_6: {
    switch (llhttp__internal__c_update_header_state_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_ws;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_7: {
    switch (llhttp__internal__c_update_header_state_7(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_otherwise;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_4: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_7;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
  s_n_llhttp__internal__n_invoke_load_type_1: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_4;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_8: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_5: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_8;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
  s_n_llhttp__internal__n_invoke_load_type_2: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_5;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    /* UNREACHABLE */;
  s_n_llhttp__internal__n_invoke_or_flags_18: {
    switch (llhttp__internal__c_or_flags_18(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_8;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_9: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_10: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_6: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_method_1;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major;
  s_n_llhttp__internal__n_invoke_store_http_minor: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_6;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_7: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_res_http_end;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major_1;
  s_n_llhttp__internal__n_invoke_store_http_minor_1: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_7;
    }
    /* UNREACHABLE */;
    abort();
  }