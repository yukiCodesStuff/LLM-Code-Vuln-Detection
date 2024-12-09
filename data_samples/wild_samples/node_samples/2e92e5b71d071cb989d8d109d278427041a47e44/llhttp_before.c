enum llparse_state_e {
  s_error,
  s_n_llhttp__internal__n_closed,
  s_n_llhttp__internal__n_invoke_llhttp__after_message_complete,
  s_n_llhttp__internal__n_pause_1,
  s_n_llhttp__internal__n_invoke_is_equal_upgrade,
  s_n_llhttp__internal__n_invoke_llhttp__on_message_complete_2,
  s_n_llhttp__internal__n_chunk_data_almost_done,
  s_n_llhttp__internal__n_consume_content_length,
  s_n_llhttp__internal__n_span_start_llhttp__on_body,
  s_n_llhttp__internal__n_invoke_is_equal_content_length,
  s_n_llhttp__internal__n_chunk_size_almost_done,
  s_n_llhttp__internal__n_chunk_parameters,
  s_n_llhttp__internal__n_chunk_size_otherwise,
  s_n_llhttp__internal__n_chunk_size,
  s_n_llhttp__internal__n_chunk_size_digit,
  s_n_llhttp__internal__n_invoke_update_content_length,
  s_n_llhttp__internal__n_consume_content_length_1,
  s_n_llhttp__internal__n_span_start_llhttp__on_body_1,
  s_n_llhttp__internal__n_eof,
  s_n_llhttp__internal__n_span_start_llhttp__on_body_2,
  s_n_llhttp__internal__n_invoke_llhttp__after_headers_complete,
  s_n_llhttp__internal__n_headers_almost_done,
  s_n_llhttp__internal__n_header_field_colon_discard_ws,
  s_n_llhttp__internal__n_error_19,
  s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_value,
  s_n_llhttp__internal__n_header_value_discard_lws,
  s_n_llhttp__internal__n_header_value_discard_ws_almost_done,
  s_n_llhttp__internal__n_header_value_lws,
  s_n_llhttp__internal__n_header_value_almost_done,
  s_n_llhttp__internal__n_header_value_lenient,
  s_n_llhttp__internal__n_error_23,
  s_n_llhttp__internal__n_header_value_lenient_failed,
  s_n_llhttp__internal__n_header_value_otherwise,
  s_n_llhttp__internal__n_header_value_connection_token,
  s_n_llhttp__internal__n_header_value_connection_ws,
  s_n_llhttp__internal__n_header_value_connection_1,
  s_n_llhttp__internal__n_header_value_connection_2,
  s_n_llhttp__internal__n_header_value_connection_3,
  s_n_llhttp__internal__n_header_value_connection,
  s_n_llhttp__internal__n_error_26,
  s_n_llhttp__internal__n_error_27,
  s_n_llhttp__internal__n_header_value_content_length_ws,
  s_n_llhttp__internal__n_header_value_content_length,
  s_n_llhttp__internal__n_error_29,
  s_n_llhttp__internal__n_error_28,
  s_n_llhttp__internal__n_header_value_te_token_ows,
  s_n_llhttp__internal__n_header_value,
  s_n_llhttp__internal__n_header_value_te_token,
  s_n_llhttp__internal__n_header_value_te_chunked_last,
  s_n_llhttp__internal__n_header_value_te_chunked,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1,
  s_n_llhttp__internal__n_header_value_discard_ws,
  s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete,
  s_n_llhttp__internal__n_header_field_general_otherwise,
  s_n_llhttp__internal__n_header_field_general,
  s_n_llhttp__internal__n_header_field_colon,
  s_n_llhttp__internal__n_header_field_3,
  s_n_llhttp__internal__n_header_field_4,
  s_n_llhttp__internal__n_header_field_2,
  s_n_llhttp__internal__n_header_field_1,
  s_n_llhttp__internal__n_header_field_5,
  s_n_llhttp__internal__n_header_field_6,
  s_n_llhttp__internal__n_header_field_7,
  s_n_llhttp__internal__n_header_field,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_field,
  s_n_llhttp__internal__n_header_field_start,
  s_n_llhttp__internal__n_url_to_http_09,
  s_n_llhttp__internal__n_url_skip_to_http09,
  s_n_llhttp__internal__n_url_skip_lf_to_http09_1,
  s_n_llhttp__internal__n_url_skip_lf_to_http09,
  s_n_llhttp__internal__n_req_pri_upgrade,
  s_n_llhttp__internal__n_req_http_complete_1,
  s_n_llhttp__internal__n_req_http_complete,
  s_n_llhttp__internal__n_req_http_minor,
  s_n_llhttp__internal__n_req_http_dot,
  s_n_llhttp__internal__n_req_http_major,
  s_n_llhttp__internal__n_req_http_start_1,
  s_n_llhttp__internal__n_req_http_start_2,
  s_n_llhttp__internal__n_req_http_start_3,
  s_n_llhttp__internal__n_req_http_start,
  s_n_llhttp__internal__n_url_to_http,
  s_n_llhttp__internal__n_url_skip_to_http,
  s_n_llhttp__internal__n_url_fragment,
  s_n_llhttp__internal__n_span_end_stub_query_3,
  s_n_llhttp__internal__n_url_query,
  s_n_llhttp__internal__n_url_query_or_fragment,
  s_n_llhttp__internal__n_url_path,
  s_n_llhttp__internal__n_span_start_stub_path_2,
  s_n_llhttp__internal__n_span_start_stub_path,
  s_n_llhttp__internal__n_span_start_stub_path_1,
  s_n_llhttp__internal__n_url_server_with_at,
  s_n_llhttp__internal__n_url_server,
  s_n_llhttp__internal__n_url_schema_delim_1,
  s_n_llhttp__internal__n_url_schema_delim,
  s_n_llhttp__internal__n_span_end_stub_schema,
  s_n_llhttp__internal__n_url_schema,
  s_n_llhttp__internal__n_url_start,
  s_n_llhttp__internal__n_span_start_llhttp__on_url_1,
  s_n_llhttp__internal__n_url_entry_normal,
  s_n_llhttp__internal__n_span_start_llhttp__on_url,
  s_n_llhttp__internal__n_url_entry_connect,
  s_n_llhttp__internal__n_req_spaces_before_url,
  s_n_llhttp__internal__n_req_first_space_before_url,
  s_n_llhttp__internal__n_start_req_2,
  s_n_llhttp__internal__n_start_req_3,
  s_n_llhttp__internal__n_start_req_1,
  s_n_llhttp__internal__n_start_req_4,
  s_n_llhttp__internal__n_start_req_6,
  s_n_llhttp__internal__n_start_req_8,
  s_n_llhttp__internal__n_start_req_9,
  s_n_llhttp__internal__n_start_req_7,
  s_n_llhttp__internal__n_start_req_5,
  s_n_llhttp__internal__n_start_req_12,
  s_n_llhttp__internal__n_start_req_13,
  s_n_llhttp__internal__n_start_req_11,
  s_n_llhttp__internal__n_start_req_10,
  s_n_llhttp__internal__n_start_req_14,
  s_n_llhttp__internal__n_start_req_17,
  s_n_llhttp__internal__n_start_req_16,
  s_n_llhttp__internal__n_start_req_15,
  s_n_llhttp__internal__n_start_req_18,
  s_n_llhttp__internal__n_start_req_20,
  s_n_llhttp__internal__n_start_req_21,
  s_n_llhttp__internal__n_start_req_19,
  s_n_llhttp__internal__n_start_req_23,
  s_n_llhttp__internal__n_start_req_24,
  s_n_llhttp__internal__n_start_req_26,
  s_n_llhttp__internal__n_start_req_28,
  s_n_llhttp__internal__n_start_req_29,
  s_n_llhttp__internal__n_start_req_27,
  s_n_llhttp__internal__n_start_req_25,
  s_n_llhttp__internal__n_start_req_30,
  s_n_llhttp__internal__n_start_req_22,
  s_n_llhttp__internal__n_start_req_31,
  s_n_llhttp__internal__n_start_req_32,
  s_n_llhttp__internal__n_start_req_35,
  s_n_llhttp__internal__n_start_req_36,
  s_n_llhttp__internal__n_start_req_34,
  s_n_llhttp__internal__n_start_req_37,
  s_n_llhttp__internal__n_start_req_38,
  s_n_llhttp__internal__n_start_req_42,
  s_n_llhttp__internal__n_start_req_43,
  s_n_llhttp__internal__n_start_req_41,
  s_n_llhttp__internal__n_start_req_40,
  s_n_llhttp__internal__n_start_req_39,
  s_n_llhttp__internal__n_start_req_45,
  s_n_llhttp__internal__n_start_req_44,
  s_n_llhttp__internal__n_start_req_33,
  s_n_llhttp__internal__n_start_req_48,
  s_n_llhttp__internal__n_start_req_49,
  s_n_llhttp__internal__n_start_req_50,
  s_n_llhttp__internal__n_start_req_51,
  s_n_llhttp__internal__n_start_req_47,
  s_n_llhttp__internal__n_start_req_46,
  s_n_llhttp__internal__n_start_req_54,
  s_n_llhttp__internal__n_start_req_56,
  s_n_llhttp__internal__n_start_req_57,
  s_n_llhttp__internal__n_start_req_55,
  s_n_llhttp__internal__n_start_req_53,
  s_n_llhttp__internal__n_start_req_58,
  s_n_llhttp__internal__n_start_req_59,
  s_n_llhttp__internal__n_start_req_52,
  s_n_llhttp__internal__n_start_req_61,
  s_n_llhttp__internal__n_start_req_62,
  s_n_llhttp__internal__n_start_req_60,
  s_n_llhttp__internal__n_start_req_65,
  s_n_llhttp__internal__n_start_req_67,
  s_n_llhttp__internal__n_start_req_68,
  s_n_llhttp__internal__n_start_req_66,
  s_n_llhttp__internal__n_start_req_69,
  s_n_llhttp__internal__n_start_req_64,
  s_n_llhttp__internal__n_start_req_63,
  s_n_llhttp__internal__n_start_req,
  s_n_llhttp__internal__n_invoke_llhttp__on_status_complete,
  s_n_llhttp__internal__n_res_line_almost_done,
  s_n_llhttp__internal__n_res_status,
  s_n_llhttp__internal__n_span_start_llhttp__on_status,
  s_n_llhttp__internal__n_res_status_start,
  s_n_llhttp__internal__n_res_status_code_otherwise,
  s_n_llhttp__internal__n_res_status_code,
  s_n_llhttp__internal__n_res_http_end,
  s_n_llhttp__internal__n_res_http_minor,
  s_n_llhttp__internal__n_res_http_dot,
  s_n_llhttp__internal__n_res_http_major,
  s_n_llhttp__internal__n_start_res,
  s_n_llhttp__internal__n_req_or_res_method_2,
  s_n_llhttp__internal__n_req_or_res_method_3,
  s_n_llhttp__internal__n_req_or_res_method_1,
  s_n_llhttp__internal__n_req_or_res_method,
  s_n_llhttp__internal__n_start_req_or_res,
  s_n_llhttp__internal__n_invoke_load_type,
  s_n_llhttp__internal__n_start,
};
typedef enum llparse_state_e llparse_state_t;

int llhttp__on_url(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_header_field(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_header_value(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_body(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_status(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__internal__c_update_finish(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->finish = 2;
  return 0;
}
    const unsigned char* endp) {
  state->flags |= 8;
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
  return 0;
}

int llhttp__internal__c_test_flags_2(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->flags & 32) == 32;
}

int llhttp__internal__c_mul_add_content_length_1(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  /* Multiplication overflow */
  if (state->content_length > 0xffffffffffffffffULL / 10) {
    return 1;
  }
    const unsigned char* endp) {
  return (state->flags & 8) == 8;
}

int llhttp__internal__c_test_lenient_flags_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 8) == 8;
}

int llhttp__internal__c_or_flags_16(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 512;
  return 0;
}

int llhttp__internal__c_and_flags(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags &= -9;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_18(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_load_method(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method;
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

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
    const unsigned char* endp) {
  state->flags &= -9;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_18(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_load_method(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method;
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

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
    int match) {
  state->http_minor = match;
  return 0;
}

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_21;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lws:
    s_n_llhttp__internal__n_header_value_lws: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lws;
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
    }
    case s_n_llhttp__internal__n_header_value_almost_done:
    s_n_llhttp__internal__n_header_value_almost_done: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_almost_done;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_22;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lenient:
    s_n_llhttp__internal__n_header_value_lenient: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lenient;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_3;
        }
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_4;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lenient;
        }
      }
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
      }
      switch (*p) {
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_connection_token:
    s_n_llhttp__internal__n_header_value_connection_token: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_connection_token;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_header_value_connection_token;
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
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_22;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lenient:
    s_n_llhttp__internal__n_header_value_lenient: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lenient;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_3;
        }
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_4;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lenient;
        }
      }
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
      }
      switch (*p) {
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_connection_token:
    s_n_llhttp__internal__n_header_value_connection_token: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_connection_token;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_header_value_connection_token;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lenient;
        }
      }
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
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_connection_token:
    s_n_llhttp__internal__n_header_value_connection_token: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_connection_token;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_header_value_connection_token;
        }
        case ',': {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_header_state_4;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_2;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_5;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_6;
        }
        case 2: {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_token_ows;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_8;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_te_chunked_last:
    s_n_llhttp__internal__n_header_value_te_chunked_last: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_te_chunked_last;
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
        case ',': {
          goto s_n_llhttp__internal__n_invoke_load_type_1;
        }
        default: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_te_chunked:
    s_n_llhttp__internal__n_header_value_te_chunked: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_te_chunked;
      }
      match_seq = llparse__match_sequence_to_lower_unsafe(state, p, endp, llparse_blob6, 7);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_chunked_last;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_te_chunked;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_value;
      goto s_n_llhttp__internal__n_invoke_load_header_state_2;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_discard_ws:
    s_n_llhttp__internal__n_header_value_discard_ws: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_discard_ws;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws;
        }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws_almost_done;
        }
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete:
    s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete: {
      switch (llhttp__on_header_field_complete(state, p, endp)) {
        default:
          goto s_n_llhttp__internal__n_header_value_discard_ws;
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_general_otherwise:
    s_n_llhttp__internal__n_header_field_general_otherwise: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_general_otherwise;
      }
      switch (*p) {
        case ':': {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_30;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_general:
    s_n_llhttp__internal__n_header_field_general: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_general;
      }
      #ifdef __SSE4_2__
      if (endp - p >= 16) {
        __m128i ranges;
        __m128i input;
        int avail;
        int match_len;
      
        /* Load input */
        input = _mm_loadu_si128((__m128i const*) p);
        ranges = _mm_loadu_si128((__m128i const*) llparse_blob8);
      
        /* Find first character that does not match `ranges` */
        match_len = _mm_cmpestri(ranges, 16,
            input, 16,
            _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
              _SIDD_NEGATIVE_POLARITY);
      
        if (match_len != 0) {
          p += match_len;
          goto s_n_llhttp__internal__n_header_field_general;
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
        case ',': {
          goto s_n_llhttp__internal__n_invoke_load_type_1;
        }
        default: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case ':': {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_9;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_3:
    s_n_llhttp__internal__n_header_field_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_3;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob2, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_4:
    s_n_llhttp__internal__n_header_field_4: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_4;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob10, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_4:
    s_n_llhttp__internal__n_header_field_4: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_4;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob10, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http_09:
    s_n_llhttp__internal__n_url_to_http_09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http_09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09_1:
    s_n_llhttp__internal__n_url_skip_lf_to_http09_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_36;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_37;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_40;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_41;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_42;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_45;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_to_http:
    s_n_llhttp__internal__n_url_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        case 12: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_url_to_http;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
        5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_error_1;
        }
    switch (llhttp__internal__c_test_lenient_flags_2(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_header_field_colon_discard_ws;
      default:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_20: {
    state->error = 0xb;
    state->reason = "Empty Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_header_value: {
    const unsigned char* start;
    int err;
    
    start = state->_span_pos0;
    state->_span_pos0 = NULL;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
      return s_error;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 2:
        goto s_n_llhttp__internal__n_error_20;
      default:
        goto s_n_llhttp__internal__n_invoke_load_header_state_1;
    }
  s_n_llhttp__internal__n_error_21: {
    state->error = 0x2;
    state->reason = "Expected LF after CR";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_1: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    switch (llhttp__internal__c_or_flags_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_3: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_8;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_9;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_10;
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_8;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_9;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_10;
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_22: {
    state->error = 0x3;
    state->reason = "Missing expected LF after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1: {
    const unsigned char* start;
    int err;
    
    start = state->_span_pos0;
    state->_span_pos0 = NULL;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_header_value_almost_done;
      return s_error;
    }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_23;
      return s_error;
    }
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    switch (llhttp__internal__c_or_flags_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_4: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_12;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_13;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_14;
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_12;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_13;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_14;
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_4: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_token;
    }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_29;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_29;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_7: {
    switch (llhttp__internal__c_update_header_state_7(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_otherwise;
    }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_28;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_28;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_4: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_7;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_4;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    switch (llhttp__internal__c_or_flags_16(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_and_flags;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_5: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_8;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_5;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    switch (llhttp__internal__c_or_flags_18(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_8;
    }
  s_n_llhttp__internal__n_error_30: {
    state->error = 0xa;
    state->reason = "Invalid header token";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_9: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    switch (llhttp__internal__c_store_header_state(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_header_field_colon;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_10: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    switch (llhttp__internal__c_load_http_major(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_invoke_load_http_minor;
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_1;
      case 2:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_2;
      default:
        goto s_n_llhttp__internal__n_error_33;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_6: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_method_1;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major;
    }
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_6;
    }
    switch (llhttp__internal__c_load_http_major(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_3;
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_4;
      case 2:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_5;
      default:
        goto s_n_llhttp__internal__n_error_57;
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
    }
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_7;
    }
enum llparse_state_e {
  s_error,
  s_n_llhttp__internal__n_closed,
  s_n_llhttp__internal__n_invoke_llhttp__after_message_complete,
  s_n_llhttp__internal__n_pause_1,
  s_n_llhttp__internal__n_invoke_is_equal_upgrade,
  s_n_llhttp__internal__n_invoke_llhttp__on_message_complete_2,
  s_n_llhttp__internal__n_chunk_data_almost_done_skip,
  s_n_llhttp__internal__n_chunk_data_almost_done,
  s_n_llhttp__internal__n_consume_content_length,
  s_n_llhttp__internal__n_span_start_llhttp__on_body,
  s_n_llhttp__internal__n_invoke_is_equal_content_length,
  s_n_llhttp__internal__n_chunk_size_almost_done,
  s_n_llhttp__internal__n_chunk_parameters,
  s_n_llhttp__internal__n_chunk_size_otherwise,
  s_n_llhttp__internal__n_chunk_size,
  s_n_llhttp__internal__n_chunk_size_digit,
  s_n_llhttp__internal__n_invoke_update_content_length,
  s_n_llhttp__internal__n_consume_content_length_1,
  s_n_llhttp__internal__n_span_start_llhttp__on_body_1,
  s_n_llhttp__internal__n_eof,
  s_n_llhttp__internal__n_span_start_llhttp__on_body_2,
  s_n_llhttp__internal__n_invoke_llhttp__after_headers_complete,
  s_n_llhttp__internal__n_headers_almost_done,
  s_n_llhttp__internal__n_header_field_colon_discard_ws,
  s_n_llhttp__internal__n_error_14,
  s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_value,
  s_n_llhttp__internal__n_header_value_discard_lws,
  s_n_llhttp__internal__n_header_value_discard_ws_almost_done,
  s_n_llhttp__internal__n_header_value_lws,
  s_n_llhttp__internal__n_header_value_almost_done,
  s_n_llhttp__internal__n_header_value_lenient,
  s_n_llhttp__internal__n_error_17,
  s_n_llhttp__internal__n_header_value_lenient_failed,
  s_n_llhttp__internal__n_header_value_otherwise,
  s_n_llhttp__internal__n_header_value_connection_token,
  s_n_llhttp__internal__n_header_value_connection_ws,
  s_n_llhttp__internal__n_header_value_connection_1,
  s_n_llhttp__internal__n_header_value_connection_2,
  s_n_llhttp__internal__n_header_value_connection_3,
  s_n_llhttp__internal__n_header_value_connection,
  s_n_llhttp__internal__n_error_20,
  s_n_llhttp__internal__n_error_21,
  s_n_llhttp__internal__n_header_value_content_length_ws,
  s_n_llhttp__internal__n_header_value_content_length,
  s_n_llhttp__internal__n_error_23,
  s_n_llhttp__internal__n_error_22,
  s_n_llhttp__internal__n_header_value_te_token_ows,
  s_n_llhttp__internal__n_header_value,
  s_n_llhttp__internal__n_header_value_te_token,
  s_n_llhttp__internal__n_header_value_te_chunked_last,
  s_n_llhttp__internal__n_header_value_te_chunked,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1,
  s_n_llhttp__internal__n_header_value_discard_ws,
  s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete,
  s_n_llhttp__internal__n_header_field_general_otherwise,
  s_n_llhttp__internal__n_header_field_general,
  s_n_llhttp__internal__n_header_field_colon,
  s_n_llhttp__internal__n_header_field_3,
  s_n_llhttp__internal__n_header_field_4,
  s_n_llhttp__internal__n_header_field_2,
  s_n_llhttp__internal__n_header_field_1,
  s_n_llhttp__internal__n_header_field_5,
  s_n_llhttp__internal__n_header_field_6,
  s_n_llhttp__internal__n_header_field_7,
  s_n_llhttp__internal__n_header_field,
  s_n_llhttp__internal__n_span_start_llhttp__on_header_field,
  s_n_llhttp__internal__n_header_field_start,
  s_n_llhttp__internal__n_url_skip_to_http09,
  s_n_llhttp__internal__n_url_skip_lf_to_http09,
  s_n_llhttp__internal__n_req_pri_upgrade,
  s_n_llhttp__internal__n_req_http_complete_1,
  s_n_llhttp__internal__n_req_http_complete,
  s_n_llhttp__internal__n_req_http_minor,
  s_n_llhttp__internal__n_req_http_dot,
  s_n_llhttp__internal__n_req_http_major,
  s_n_llhttp__internal__n_req_http_start_1,
  s_n_llhttp__internal__n_req_http_start_2,
  s_n_llhttp__internal__n_req_http_start_3,
  s_n_llhttp__internal__n_req_http_start,
  s_n_llhttp__internal__n_url_skip_to_http,
  s_n_llhttp__internal__n_url_fragment,
  s_n_llhttp__internal__n_span_end_stub_query_3,
  s_n_llhttp__internal__n_url_query,
  s_n_llhttp__internal__n_url_query_or_fragment,
  s_n_llhttp__internal__n_url_path,
  s_n_llhttp__internal__n_span_start_stub_path_2,
  s_n_llhttp__internal__n_span_start_stub_path,
  s_n_llhttp__internal__n_span_start_stub_path_1,
  s_n_llhttp__internal__n_url_server_with_at,
  s_n_llhttp__internal__n_url_server,
  s_n_llhttp__internal__n_url_schema_delim_1,
  s_n_llhttp__internal__n_url_schema_delim,
  s_n_llhttp__internal__n_span_end_stub_schema,
  s_n_llhttp__internal__n_url_schema,
  s_n_llhttp__internal__n_url_start,
  s_n_llhttp__internal__n_span_start_llhttp__on_url_1,
  s_n_llhttp__internal__n_span_start_llhttp__on_url,
  s_n_llhttp__internal__n_req_spaces_before_url,
  s_n_llhttp__internal__n_req_first_space_before_url,
  s_n_llhttp__internal__n_start_req_2,
  s_n_llhttp__internal__n_start_req_3,
  s_n_llhttp__internal__n_start_req_1,
  s_n_llhttp__internal__n_start_req_4,
  s_n_llhttp__internal__n_start_req_6,
  s_n_llhttp__internal__n_start_req_8,
  s_n_llhttp__internal__n_start_req_9,
  s_n_llhttp__internal__n_start_req_7,
  s_n_llhttp__internal__n_start_req_5,
  s_n_llhttp__internal__n_start_req_12,
  s_n_llhttp__internal__n_start_req_13,
  s_n_llhttp__internal__n_start_req_11,
  s_n_llhttp__internal__n_start_req_10,
  s_n_llhttp__internal__n_start_req_14,
  s_n_llhttp__internal__n_start_req_17,
  s_n_llhttp__internal__n_start_req_16,
  s_n_llhttp__internal__n_start_req_15,
  s_n_llhttp__internal__n_start_req_18,
  s_n_llhttp__internal__n_start_req_20,
  s_n_llhttp__internal__n_start_req_21,
  s_n_llhttp__internal__n_start_req_19,
  s_n_llhttp__internal__n_start_req_23,
  s_n_llhttp__internal__n_start_req_24,
  s_n_llhttp__internal__n_start_req_26,
  s_n_llhttp__internal__n_start_req_28,
  s_n_llhttp__internal__n_start_req_29,
  s_n_llhttp__internal__n_start_req_27,
  s_n_llhttp__internal__n_start_req_25,
  s_n_llhttp__internal__n_start_req_30,
  s_n_llhttp__internal__n_start_req_22,
  s_n_llhttp__internal__n_start_req_31,
  s_n_llhttp__internal__n_start_req_32,
  s_n_llhttp__internal__n_start_req_35,
  s_n_llhttp__internal__n_start_req_36,
  s_n_llhttp__internal__n_start_req_34,
  s_n_llhttp__internal__n_start_req_37,
  s_n_llhttp__internal__n_start_req_38,
  s_n_llhttp__internal__n_start_req_42,
  s_n_llhttp__internal__n_start_req_43,
  s_n_llhttp__internal__n_start_req_41,
  s_n_llhttp__internal__n_start_req_40,
  s_n_llhttp__internal__n_start_req_39,
  s_n_llhttp__internal__n_start_req_45,
  s_n_llhttp__internal__n_start_req_44,
  s_n_llhttp__internal__n_start_req_33,
  s_n_llhttp__internal__n_start_req_48,
  s_n_llhttp__internal__n_start_req_49,
  s_n_llhttp__internal__n_start_req_50,
  s_n_llhttp__internal__n_start_req_51,
  s_n_llhttp__internal__n_start_req_47,
  s_n_llhttp__internal__n_start_req_46,
  s_n_llhttp__internal__n_start_req_54,
  s_n_llhttp__internal__n_start_req_56,
  s_n_llhttp__internal__n_start_req_57,
  s_n_llhttp__internal__n_start_req_55,
  s_n_llhttp__internal__n_start_req_53,
  s_n_llhttp__internal__n_start_req_58,
  s_n_llhttp__internal__n_start_req_59,
  s_n_llhttp__internal__n_start_req_52,
  s_n_llhttp__internal__n_start_req_61,
  s_n_llhttp__internal__n_start_req_62,
  s_n_llhttp__internal__n_start_req_60,
  s_n_llhttp__internal__n_start_req_65,
  s_n_llhttp__internal__n_start_req_67,
  s_n_llhttp__internal__n_start_req_68,
  s_n_llhttp__internal__n_start_req_66,
  s_n_llhttp__internal__n_start_req_69,
  s_n_llhttp__internal__n_start_req_64,
  s_n_llhttp__internal__n_start_req_63,
  s_n_llhttp__internal__n_start_req,
  s_n_llhttp__internal__n_invoke_llhttp__on_status_complete,
  s_n_llhttp__internal__n_res_line_almost_done,
  s_n_llhttp__internal__n_res_status,
  s_n_llhttp__internal__n_span_start_llhttp__on_status,
  s_n_llhttp__internal__n_res_status_start,
  s_n_llhttp__internal__n_res_status_code_otherwise,
  s_n_llhttp__internal__n_res_status_code,
  s_n_llhttp__internal__n_res_http_end,
  s_n_llhttp__internal__n_res_http_minor,
  s_n_llhttp__internal__n_res_http_dot,
  s_n_llhttp__internal__n_res_http_major,
  s_n_llhttp__internal__n_start_res,
  s_n_llhttp__internal__n_req_or_res_method_2,
  s_n_llhttp__internal__n_req_or_res_method_3,
  s_n_llhttp__internal__n_req_or_res_method_1,
  s_n_llhttp__internal__n_req_or_res_method,
  s_n_llhttp__internal__n_start_req_or_res,
  s_n_llhttp__internal__n_invoke_load_type,
  s_n_llhttp__internal__n_start,
};
typedef enum llparse_state_e llparse_state_t;

int llhttp__on_url(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_header_field(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_header_value(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_body(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__on_status(
    llhttp__internal_t* s, const unsigned char* p,
    const unsigned char* endp);

int llhttp__internal__c_update_finish(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->finish = 2;
  return 0;
}
    const unsigned char* endp) {
  state->flags |= 8;
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
  return 0;
}

int llhttp__internal__c_test_flags_2(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->flags & 32) == 32;
}

int llhttp__internal__c_mul_add_content_length_1(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp,
    int match) {
  /* Multiplication overflow */
  if (state->content_length > 0xffffffffffffffffULL / 10) {
    return 1;
  }
    const unsigned char* endp) {
  return (state->flags & 8) == 8;
}

int llhttp__internal__c_test_lenient_flags_4(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 8) == 8;
}

int llhttp__internal__c_or_flags_16(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 512;
  return 0;
}

int llhttp__internal__c_and_flags(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags &= -9;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_18(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_load_method(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method;
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

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
    const unsigned char* endp) {
  state->flags &= -9;
  return 0;
}

int llhttp__internal__c_update_header_state_7(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->header_state = 8;
  return 0;
}

int llhttp__internal__c_or_flags_18(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  state->flags |= 16;
  return 0;
}

int llhttp__internal__c_load_method(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->method;
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

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
    int match) {
  state->http_minor = match;
  return 0;
}

int llhttp__internal__c_test_lenient_flags_6(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
}

int llhttp__internal__c_load_http_major(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_major;
}

int llhttp__internal__c_load_http_minor(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return state->http_minor;
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
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_16;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_lenient:
    s_n_llhttp__internal__n_header_value_lenient: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_lenient;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_3;
        }
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_4;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lenient;
        }
      }
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
      }
      switch (*p) {
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_connection_token:
    s_n_llhttp__internal__n_header_value_connection_token: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_connection_token;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_header_value_connection_token;
        }
        default: {
          p++;
          goto s_n_llhttp__internal__n_header_value_lenient;
        }
      }
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
        case 13: {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_3;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_connection_token:
    s_n_llhttp__internal__n_header_value_connection_token: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_connection_token;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_header_value_connection_token;
        }
        case ',': {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_header_state_4;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_2;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_5;
        }
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_header_state_6;
        }
        case 2: {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_token_ows;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_8;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_te_chunked_last:
    s_n_llhttp__internal__n_header_value_te_chunked_last: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_te_chunked_last;
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
        case ',': {
          goto s_n_llhttp__internal__n_invoke_load_type_1;
        }
        default: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_te_chunked:
    s_n_llhttp__internal__n_header_value_te_chunked: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_te_chunked;
      }
      match_seq = llparse__match_sequence_to_lower_unsafe(state, p, endp, llparse_blob6, 7);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_value_te_chunked_last;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_value_te_chunked;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_value;
      goto s_n_llhttp__internal__n_invoke_load_header_state_2;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_value_discard_ws:
    s_n_llhttp__internal__n_header_value_discard_ws: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_value_discard_ws;
      }
      switch (*p) {
        case 9: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws;
        }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws_almost_done;
        }
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_ws;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_value_1;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete:
    s_n_llhttp__internal__n_invoke_llhttp__on_header_field_complete: {
      switch (llhttp__on_header_field_complete(state, p, endp)) {
        default:
          goto s_n_llhttp__internal__n_header_value_discard_ws;
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_general_otherwise:
    s_n_llhttp__internal__n_header_field_general_otherwise: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_general_otherwise;
      }
      switch (*p) {
        case ':': {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_24;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_general:
    s_n_llhttp__internal__n_header_field_general: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_general;
      }
      #ifdef __SSE4_2__
      if (endp - p >= 16) {
        __m128i ranges;
        __m128i input;
        int avail;
        int match_len;
      
        /* Load input */
        input = _mm_loadu_si128((__m128i const*) p);
        ranges = _mm_loadu_si128((__m128i const*) llparse_blob8);
      
        /* Find first character that does not match `ranges` */
        match_len = _mm_cmpestri(ranges, 16,
            input, 16,
            _SIDD_UBYTE_OPS | _SIDD_CMP_RANGES |
              _SIDD_NEGATIVE_POLARITY);
      
        if (match_len != 0) {
          p += match_len;
          goto s_n_llhttp__internal__n_header_field_general;
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
        case ',': {
          goto s_n_llhttp__internal__n_invoke_load_type_1;
        }
        default: {
          goto s_n_llhttp__internal__n_header_value_te_token;
        }
      }
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_value_discard_lws;
        }
        case ':': {
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_1;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_9;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_3:
    s_n_llhttp__internal__n_header_field_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_3;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob2, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_4:
    s_n_llhttp__internal__n_header_field_4: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_4;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob10, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_4:
    s_n_llhttp__internal__n_header_field_4: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_4;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob10, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_2:
    s_n_llhttp__internal__n_header_field_2: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_2;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'n': {
          p++;
          goto s_n_llhttp__internal__n_header_field_3;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_4;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_1:
    s_n_llhttp__internal__n_header_field_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_1;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob1, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_header_field_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_5:
    s_n_llhttp__internal__n_header_field_5: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_5;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob11, 15);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_5;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_6:
    s_n_llhttp__internal__n_header_field_6: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_6;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob12, 16);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_7:
    s_n_llhttp__internal__n_header_field_7: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_7;
      }
      match_seq = llparse__match_sequence_to_lower(state, p, endp, llparse_blob13, 6);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_header_state;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_header_field_7;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field:
    s_n_llhttp__internal__n_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field;
      }
      switch (((*p) >= 'A' && (*p) <= 'Z' ? (*p | 0x20) : (*p))) {
        case 'c': {
          p++;
          goto s_n_llhttp__internal__n_header_field_1;
        }
        case 'p': {
          p++;
          goto s_n_llhttp__internal__n_header_field_5;
        }
        case 't': {
          p++;
          goto s_n_llhttp__internal__n_header_field_6;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
        case 'u': {
          p++;
          goto s_n_llhttp__internal__n_header_field_7;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_update_header_state_10;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_span_start_llhttp__on_header_field:
    s_n_llhttp__internal__n_span_start_llhttp__on_header_field: {
      if (p == endp) {
        return s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
      }
      state->_span_pos0 = (void*) p;
      state->_span_cb0 = llhttp__on_header_field;
      goto s_n_llhttp__internal__n_header_field;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_header_field_start:
    s_n_llhttp__internal__n_header_field_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_header_field_start;
      }
      switch (*p) {
        case 10: {
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_headers_almost_done;
        }
        default: {
          goto s_n_llhttp__internal__n_span_start_llhttp__on_header_field;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http09:
    s_n_llhttp__internal__n_url_skip_to_http09: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http09;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_update_http_major;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_lf_to_http09:
    s_n_llhttp__internal__n_url_skip_lf_to_http09: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_lf_to_http09;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob14, 2);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_update_http_major;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_url_skip_lf_to_http09;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_25;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_pri_upgrade:
    s_n_llhttp__internal__n_req_pri_upgrade: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_pri_upgrade;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob16, 10);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_30;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_31;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete_1:
    s_n_llhttp__internal__n_req_http_complete_1: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete_1;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_complete:
    s_n_llhttp__internal__n_req_http_complete: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_complete;
      }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_header_field_start;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_minor:
    s_n_llhttp__internal__n_req_http_minor: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_minor;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_34;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_dot:
    s_n_llhttp__internal__n_req_http_dot: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_dot;
      }
      switch (*p) {
        case '.': {
          p++;
          goto s_n_llhttp__internal__n_req_http_minor;
        }
        default: {
          goto s_n_llhttp__internal__n_error_35;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_major:
    s_n_llhttp__internal__n_req_http_major: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_major;
      }
      switch (*p) {
        case '0': {
          p++;
          match = 0;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '1': {
          p++;
          match = 1;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '2': {
          p++;
          match = 2;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '3': {
          p++;
          match = 3;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '4': {
          p++;
          match = 4;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '5': {
          p++;
          match = 5;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '6': {
          p++;
          match = 6;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '7': {
          p++;
          match = 7;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '8': {
          p++;
          match = 8;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        case '9': {
          p++;
          match = 9;
          goto s_n_llhttp__internal__n_invoke_store_http_major;
        }
        default: {
          goto s_n_llhttp__internal__n_error_36;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_1:
    s_n_llhttp__internal__n_req_http_start_1: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_1;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob15, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_2:
    s_n_llhttp__internal__n_req_http_start_2: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_2;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob17, 3);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_2;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start_3:
    s_n_llhttp__internal__n_req_http_start_3: {
      llparse_match_t match_seq;
      
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start_3;
      }
      match_seq = llparse__match_sequence_id(state, p, endp, llparse_blob18, 4);
      p = match_seq.current;
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_invoke_load_method_3;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_req_http_start:
    s_n_llhttp__internal__n_req_http_start: {
      if (p == endp) {
        return s_n_llhttp__internal__n_req_http_start;
      }
      switch (*p) {
        case ' ': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start;
        }
        case 'H': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_1;
        }
        case 'I': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_2;
        }
        case 'R': {
          p++;
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_39;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_skip_to_http:
    s_n_llhttp__internal__n_url_skip_to_http: {
      if (p == endp) {
        return s_n_llhttp__internal__n_url_skip_to_http;
      }
      p++;
      goto s_n_llhttp__internal__n_invoke_llhttp__on_url_complete_1;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_url_fragment:
    s_n_llhttp__internal__n_url_fragment: {
      static uint8_t lookup_table[] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 0, 1, 3, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        4, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
      };
      if (p == endp) {
        return s_n_llhttp__internal__n_url_fragment;
      }
      switch (lookup_table[(uint8_t) *p]) {
        case 1: {
          p++;
          goto s_n_llhttp__internal__n_url_fragment;
        }
    switch (llhttp__internal__c_test_lenient_flags_2(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_header_field_colon_discard_ws;
      default:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_15: {
    state->error = 0xb;
    state->reason = "Empty Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_header_value: {
    const unsigned char* start;
    int err;
    
    start = state->_span_pos0;
    state->_span_pos0 = NULL;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
      return s_error;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 2:
        goto s_n_llhttp__internal__n_error_15;
      default:
        goto s_n_llhttp__internal__n_invoke_load_header_state_1;
    }
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_1;
    }
    switch (llhttp__internal__c_or_flags_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_3: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_8;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_9;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_10;
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_7;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_8;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_9;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_10;
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_16: {
    state->error = 0x3;
    state->reason = "Missing expected LF after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_header_value_1: {
    const unsigned char* start;
    int err;
    
    start = state->_span_pos0;
    state->_span_pos0 = NULL;
    err = llhttp__on_header_value(state, start, p);
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_header_value_almost_done;
      return s_error;
    }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_17;
      return s_error;
    }
    switch (llhttp__internal__c_or_flags_3(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_3;
    }
    switch (llhttp__internal__c_or_flags_6(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_header_state_4: {
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_12;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_13;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_14;
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    switch (llhttp__internal__c_load_header_state(state, p, endp)) {
      case 5:
        goto s_n_llhttp__internal__n_invoke_or_flags_11;
      case 6:
        goto s_n_llhttp__internal__n_invoke_or_flags_12;
      case 7:
        goto s_n_llhttp__internal__n_invoke_or_flags_13;
      case 8:
        goto s_n_llhttp__internal__n_invoke_or_flags_14;
      default:
        goto s_n_llhttp__internal__n_header_value_connection;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_4: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_connection_token;
    }
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
  s_n_llhttp__internal__n_invoke_update_header_state_7: {
    switch (llhttp__internal__c_update_header_state_7(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_value_otherwise;
    }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_22;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_22;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_4: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_7;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_4;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    switch (llhttp__internal__c_or_flags_16(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_and_flags;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_5: {
    switch (llhttp__internal__c_test_lenient_flags_4(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_8;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_5;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_17;
    }
    switch (llhttp__internal__c_or_flags_18(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_update_header_state_8;
    }
  s_n_llhttp__internal__n_error_24: {
    state->error = 0xa;
    state->reason = "Invalid header token";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_9: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    switch (llhttp__internal__c_store_header_state(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_header_field_colon;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_10: {
    switch (llhttp__internal__c_update_header_state_4(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_header_field_general;
    }
    switch (llhttp__internal__c_load_http_major(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_invoke_load_http_minor;
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_1;
      case 2:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_2;
      default:
        goto s_n_llhttp__internal__n_error_27;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_6: {
    switch (llhttp__internal__c_test_lenient_flags_6(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_method_1;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major;
    }
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_6;
    }
    switch (llhttp__internal__c_load_http_major(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_3;
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_4;
      case 2:
        goto s_n_llhttp__internal__n_invoke_load_http_minor_5;
      default:
        goto s_n_llhttp__internal__n_error_51;
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
    }
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_7;
    }