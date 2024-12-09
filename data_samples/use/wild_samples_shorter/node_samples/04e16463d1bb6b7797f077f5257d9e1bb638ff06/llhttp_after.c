  s_n_llhttp__internal__n_header_value_almost_done,
  s_n_llhttp__internal__n_invoke_test_lenient_flags_17,
  s_n_llhttp__internal__n_header_value_lenient,
  s_n_llhttp__internal__n_error_54,
  s_n_llhttp__internal__n_header_value_otherwise,
  s_n_llhttp__internal__n_header_value_connection_token,
  s_n_llhttp__internal__n_header_value_connection_ws,
  s_n_llhttp__internal__n_header_value_connection_1,
  s_n_llhttp__internal__n_header_value_connection_2,
  s_n_llhttp__internal__n_header_value_connection_3,
  s_n_llhttp__internal__n_header_value_connection,
  s_n_llhttp__internal__n_error_56,
  s_n_llhttp__internal__n_error_57,
  s_n_llhttp__internal__n_header_value_content_length_ws,
  s_n_llhttp__internal__n_header_value_content_length,
  s_n_llhttp__internal__n_error_59,
  s_n_llhttp__internal__n_error_58,
  s_n_llhttp__internal__n_header_value_te_token_ows,
  s_n_llhttp__internal__n_header_value,
  s_n_llhttp__internal__n_header_value_te_token,
  s_n_llhttp__internal__n_header_value_te_chunked_last,
  s_n_llhttp__internal__n_req_http_complete,
  s_n_llhttp__internal__n_invoke_load_method_1,
  s_n_llhttp__internal__n_invoke_llhttp__on_version_complete,
  s_n_llhttp__internal__n_error_66,
  s_n_llhttp__internal__n_error_73,
  s_n_llhttp__internal__n_req_http_minor,
  s_n_llhttp__internal__n_error_74,
  s_n_llhttp__internal__n_req_http_dot,
  s_n_llhttp__internal__n_error_75,
  s_n_llhttp__internal__n_req_http_major,
  s_n_llhttp__internal__n_span_start_llhttp__on_version,
  s_n_llhttp__internal__n_req_http_start_1,
  s_n_llhttp__internal__n_req_http_start_2,
  s_n_llhttp__internal__n_after_start_req,
  s_n_llhttp__internal__n_span_start_llhttp__on_method_1,
  s_n_llhttp__internal__n_res_line_almost_done,
  s_n_llhttp__internal__n_invoke_test_lenient_flags_30,
  s_n_llhttp__internal__n_res_status,
  s_n_llhttp__internal__n_span_start_llhttp__on_status,
  s_n_llhttp__internal__n_res_status_code_otherwise,
  s_n_llhttp__internal__n_res_status_code_digit_3,
  s_n_llhttp__internal__n_res_status_code_digit_1,
  s_n_llhttp__internal__n_res_after_version,
  s_n_llhttp__internal__n_invoke_llhttp__on_version_complete_1,
  s_n_llhttp__internal__n_error_89,
  s_n_llhttp__internal__n_error_103,
  s_n_llhttp__internal__n_res_http_minor,
  s_n_llhttp__internal__n_error_104,
  s_n_llhttp__internal__n_res_http_dot,
  s_n_llhttp__internal__n_error_105,
  s_n_llhttp__internal__n_res_http_major,
  s_n_llhttp__internal__n_span_start_llhttp__on_version_1,
  s_n_llhttp__internal__n_start_res,
  s_n_llhttp__internal__n_invoke_llhttp__on_method_complete,
  return (state->flags & 512) == 512;
}

int llhttp__internal__c_test_lenient_flags_22(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 2) == 2;
  return (state->flags & 8) == 8;
}

int llhttp__internal__c_test_lenient_flags_20(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 8) == 8;
  return 0;
}

int llhttp__internal__c_test_lenient_flags_24(
    llhttp__internal_t* state,
    const unsigned char* p,
    const unsigned char* endp) {
  return (state->lenient_flags & 16) == 16;
      }
      switch (*p) {
        case 9: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_18;
        }
        case ' ': {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_18;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_load_header_state_5;
        }
          goto s_n_llhttp__internal__n_header_value_lws;
        }
        default: {
          goto s_n_llhttp__internal__n_error_53;
        }
      }
      /* UNREACHABLE */;
      abort();
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_54:
    s_n_llhttp__internal__n_error_54: {
      state->error = 0xa;
      state->reason = "Invalid header value char";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_2;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_19;
        }
      }
      /* UNREACHABLE */;
      abort();
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_56:
    s_n_llhttp__internal__n_error_56: {
      state->error = 0xb;
      state->reason = "Content-Length overflow";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_57:
    s_n_llhttp__internal__n_error_57: {
      state->error = 0xb;
      state->reason = "Invalid character in Content-Length";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_59:
    s_n_llhttp__internal__n_error_59: {
      state->error = 0xf;
      state->reason = "Invalid `Transfer-Encoding` header value";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_58:
    s_n_llhttp__internal__n_error_58: {
      state->error = 0xf;
      state->reason = "Invalid `Transfer-Encoding` header value";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
          goto s_n_llhttp__internal__n_span_end_llhttp__on_header_field_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_62;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_to_http_09;
        }
        default: {
          goto s_n_llhttp__internal__n_error_63;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_skip_lf_to_http09_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_63;
        }
      }
      /* UNREACHABLE */;
      abort();
      switch (match_seq.status) {
        case kMatchComplete: {
          p++;
          goto s_n_llhttp__internal__n_error_71;
        }
        case kMatchPause: {
          return s_n_llhttp__internal__n_req_pri_upgrade;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_72;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_headers_start;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_26;
        }
      }
      /* UNREACHABLE */;
      abort();
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_25;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_req_http_complete_crlf;
        }
        default: {
          goto s_n_llhttp__internal__n_error_70;
        }
      }
      /* UNREACHABLE */;
      abort();
        case 21:
          goto s_n_llhttp__internal__n_pause_21;
        default:
          goto s_n_llhttp__internal__n_error_67;
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_66:
    s_n_llhttp__internal__n_error_66: {
      state->error = 0x9;
      state->reason = "Invalid HTTP version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_73:
    s_n_llhttp__internal__n_error_73: {
      state->error = 0x9;
      state->reason = "Invalid minor version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_74:
    s_n_llhttp__internal__n_error_74: {
      state->error = 0x9;
      state->reason = "Expected dot";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_75:
    s_n_llhttp__internal__n_error_75: {
      state->error = 0x9;
      state->reason = "Invalid major version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
          return s_n_llhttp__internal__n_req_http_start_1;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_78;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_req_http_start_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_78;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_req_http_start_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_78;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_req_http_start_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_78;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_fragment;
        }
        default: {
          goto s_n_llhttp__internal__n_error_79;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_span_end_stub_query_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_80;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_query;
        }
        default: {
          goto s_n_llhttp__internal__n_error_81;
        }
      }
      /* UNREACHABLE */;
      abort();
        }
        case 8: {
          p++;
          goto s_n_llhttp__internal__n_error_82;
        }
        default: {
          goto s_n_llhttp__internal__n_error_83;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_server_with_at;
        }
        default: {
          goto s_n_llhttp__internal__n_error_84;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_server;
        }
        default: {
          goto s_n_llhttp__internal__n_error_85;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_schema_delim_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_85;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_schema;
        }
        default: {
          goto s_n_llhttp__internal__n_error_86;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_url_schema;
        }
        default: {
          goto s_n_llhttp__internal__n_error_87;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_req_spaces_before_url;
        }
        default: {
          goto s_n_llhttp__internal__n_error_88;
        }
      }
      /* UNREACHABLE */;
      abort();
        case 21:
          goto s_n_llhttp__internal__n_pause_26;
        default:
          goto s_n_llhttp__internal__n_error_107;
      }
      /* UNREACHABLE */;
      abort();
    }
          goto s_n_llhttp__internal__n_invoke_store_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_4;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_6;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_8;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_store_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_9;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_7;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_12;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_13;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_13;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_11;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_14;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_17;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_15;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_18;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_20;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_21;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_21;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_23;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_24;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_26;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_28;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_store_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_29;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_27;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_30;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_30;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_31;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_32;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_35;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_36;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_36;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_37;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_38;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_42;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_43;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_43;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_41;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_40;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_45;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_store_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_44;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_46;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_49;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_50;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_51;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_52;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_52;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_48;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_55;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_store_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_58;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_58;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_56;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_59;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_60;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_60;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_62;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_63;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_63;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_66;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_68;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_69;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_69;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_after_start_req_70;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_70;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_65;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_after_start_req_64;
        }
        default: {
          goto s_n_llhttp__internal__n_error_108;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_llhttp__on_status_complete;
        }
        default: {
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_29;
        }
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_invoke_test_lenient_flags_30:
    s_n_llhttp__internal__n_invoke_test_lenient_flags_30: {
      switch (llhttp__internal__c_test_lenient_flags_1(state, p, endp)) {
        case 1:
          goto s_n_llhttp__internal__n_invoke_llhttp__on_status_complete;
        default:
          goto s_n_llhttp__internal__n_error_94;
      }
      /* UNREACHABLE */;
      abort();
    }
      switch (*p) {
        case 10: {
          p++;
          goto s_n_llhttp__internal__n_invoke_test_lenient_flags_28;
        }
        case 13: {
          p++;
          goto s_n_llhttp__internal__n_res_line_almost_done;
          goto s_n_llhttp__internal__n_span_start_llhttp__on_status;
        }
        default: {
          goto s_n_llhttp__internal__n_error_95;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_mul_add_status_code_2;
        }
        default: {
          goto s_n_llhttp__internal__n_error_97;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_mul_add_status_code_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_99;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_mul_add_status_code;
        }
        default: {
          goto s_n_llhttp__internal__n_error_101;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_invoke_update_status_code;
        }
        default: {
          goto s_n_llhttp__internal__n_error_102;
        }
      }
      /* UNREACHABLE */;
      abort();
        case 21:
          goto s_n_llhttp__internal__n_pause_25;
        default:
          goto s_n_llhttp__internal__n_error_90;
      }
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_89:
    s_n_llhttp__internal__n_error_89: {
      state->error = 0x9;
      state->reason = "Invalid HTTP version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_103:
    s_n_llhttp__internal__n_error_103: {
      state->error = 0x9;
      state->reason = "Invalid minor version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_104:
    s_n_llhttp__internal__n_error_104: {
      state->error = 0x9;
      state->reason = "Expected dot";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
      /* UNREACHABLE */;
      abort();
    }
    case s_n_llhttp__internal__n_error_105:
    s_n_llhttp__internal__n_error_105: {
      state->error = 0x9;
      state->reason = "Invalid major version";
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_error;
          return s_n_llhttp__internal__n_start_res;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_109;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_req_or_res_method_2;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_106;
        }
      }
      /* UNREACHABLE */;
      abort();
          return s_n_llhttp__internal__n_req_or_res_method_3;
        }
        case kMatchMismatch: {
          goto s_n_llhttp__internal__n_error_106;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_req_or_res_method_3;
        }
        default: {
          goto s_n_llhttp__internal__n_error_106;
        }
      }
      /* UNREACHABLE */;
      abort();
          goto s_n_llhttp__internal__n_req_or_res_method_1;
        }
        default: {
          goto s_n_llhttp__internal__n_error_106;
        }
      }
      /* UNREACHABLE */;
      abort();
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_60: {
    state->error = 0xb;
    state->reason = "Content-Length can't be present with Transfer-Encoding";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_52: {
    state->error = 0xa;
    state->reason = "Unexpected whitespace after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    return s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_18: {
    switch (llhttp__internal__c_test_lenient_flags(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_load_header_state_4;
      default:
        goto s_n_llhttp__internal__n_error_52;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_2: {
    switch (llhttp__internal__c_update_header_state(state, p, endp)) {
      default:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_header_value_complete;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_53: {
    state->error = 0x3;
    state->reason = "Missing expected LF after header value";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_54;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_54;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_19: {
    switch (llhttp__internal__c_test_lenient_flags(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_header_value_lenient;
      default:
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_56;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_56;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_mul_add_content_length_1: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_57;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_57;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_55: {
    state->error = 0x4;
    state->reason = "Duplicate Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 0:
        goto s_n_llhttp__internal__n_header_value_content_length;
      default:
        goto s_n_llhttp__internal__n_error_55;
    }
    /* UNREACHABLE */;
    abort();
  }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_59;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_59;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_update_header_state_8: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_58;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_error_58;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_20: {
    switch (llhttp__internal__c_test_lenient_flags_20(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_8;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
  s_n_llhttp__internal__n_invoke_load_type_1: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_20;
      default:
        goto s_n_llhttp__internal__n_header_value_te_chunked;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_21: {
    switch (llhttp__internal__c_test_lenient_flags_20(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_header_value_9;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_19;
  s_n_llhttp__internal__n_invoke_load_type_2: {
    switch (llhttp__internal__c_load_type(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_21;
      default:
        goto s_n_llhttp__internal__n_invoke_or_flags_19;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_22: {
    switch (llhttp__internal__c_test_lenient_flags_22(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_error_60;
      default:
        goto s_n_llhttp__internal__n_header_value_discard_ws;
    }
    /* UNREACHABLE */;
  s_n_llhttp__internal__n_invoke_test_flags_4: {
    switch (llhttp__internal__c_test_flags_4(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_22;
      default:
        goto s_n_llhttp__internal__n_header_value_discard_ws;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_61: {
    state->error = 0xf;
    state->reason = "Transfer-Encoding can't be present with Content-Length";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_23: {
    switch (llhttp__internal__c_test_lenient_flags_22(state, p, endp)) {
      case 0:
        goto s_n_llhttp__internal__n_error_61;
      default:
        goto s_n_llhttp__internal__n_header_value_discard_ws;
    }
    /* UNREACHABLE */;
  s_n_llhttp__internal__n_invoke_test_flags_5: {
    switch (llhttp__internal__c_test_flags_2(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_23;
      default:
        goto s_n_llhttp__internal__n_header_value_discard_ws;
    }
    /* UNREACHABLE */;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_62: {
    state->error = 0xa;
    state->reason = "Invalid header token";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_63: {
    state->error = 0x7;
    state->reason = "Expected CRLF";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_71: {
    state->error = 0x17;
    state->reason = "Pause on PRI/Upgrade";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_72: {
    state->error = 0x9;
    state->reason = "Expected HTTP/2 Connection Preface";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_69: {
    state->error = 0x2;
    state->reason = "Expected CRLF after version";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_26: {
    switch (llhttp__internal__c_test_lenient_flags_8(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_headers_start;
      default:
        goto s_n_llhttp__internal__n_error_69;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_68: {
    state->error = 0x9;
    state->reason = "Expected CRLF after version";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_25: {
    switch (llhttp__internal__c_test_lenient_flags_1(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_req_http_complete_crlf;
      default:
        goto s_n_llhttp__internal__n_error_68;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_70: {
    state->error = 0x9;
    state->reason = "Expected CRLF after version";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_67: {
    state->error = 0x21;
    state->reason = "`on_version_complete` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_66;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_66;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_http_minor: {
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_24: {
    switch (llhttp__internal__c_test_lenient_flags_24(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_version_1;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major;
  s_n_llhttp__internal__n_invoke_store_http_minor: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_24;
    }
    /* UNREACHABLE */;
    abort();
  }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_73;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_73;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_version_3: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_74;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_74;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_store_http_major: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_75;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_75;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_65: {
    state->error = 0x8;
    state->reason = "Invalid method for HTTP/x.x request";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 46:
        goto s_n_llhttp__internal__n_span_start_llhttp__on_version;
      default:
        goto s_n_llhttp__internal__n_error_65;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_78: {
    state->error = 0x8;
    state->reason = "Expected HTTP/";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_76: {
    state->error = 0x8;
    state->reason = "Expected SOURCE method for ICE/x.x request";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 33:
        goto s_n_llhttp__internal__n_span_start_llhttp__on_version;
      default:
        goto s_n_llhttp__internal__n_error_76;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_77: {
    state->error = 0x8;
    state->reason = "Invalid method for RTSP/x.x request";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 45:
        goto s_n_llhttp__internal__n_span_start_llhttp__on_version;
      default:
        goto s_n_llhttp__internal__n_error_77;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_64: {
    state->error = 0x1a;
    state->reason = "`on_url_complete` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 21:
        goto s_n_llhttp__internal__n_pause_22;
      default:
        goto s_n_llhttp__internal__n_error_64;
    }
    /* UNREACHABLE */;
    abort();
  }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_79: {
    state->error = 0x7;
    state->reason = "Invalid char in url fragment start";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_80: {
    state->error = 0x7;
    state->reason = "Invalid char in url query";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_81: {
    state->error = 0x7;
    state->reason = "Invalid char in url path";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_82: {
    state->error = 0x7;
    state->reason = "Double @ in url";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_83: {
    state->error = 0x7;
    state->reason = "Unexpected char in url server";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_84: {
    state->error = 0x7;
    state->reason = "Unexpected char in url server";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_85: {
    state->error = 0x7;
    state->reason = "Unexpected char in url schema";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_86: {
    state->error = 0x7;
    state->reason = "Unexpected char in url schema";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_87: {
    state->error = 0x7;
    state->reason = "Unexpected start char in url";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_88: {
    state->error = 0x6;
    state->reason = "Expected space after method";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_107: {
    state->error = 0x20;
    state->reason = "`on_method_complete` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_108: {
    state->error = 0x6;
    state->reason = "Invalid method encountered";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_100: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_98: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_96: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_92: {
    state->error = 0x1b;
    state->reason = "`on_status_complete` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 21:
        goto s_n_llhttp__internal__n_pause_24;
      default:
        goto s_n_llhttp__internal__n_error_92;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_91: {
    state->error = 0xd;
    state->reason = "Invalid response status";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_28: {
    switch (llhttp__internal__c_test_lenient_flags_1(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_status_complete;
      default:
        goto s_n_llhttp__internal__n_error_91;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_93: {
    state->error = 0x2;
    state->reason = "Expected LF after CR";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_29: {
    switch (llhttp__internal__c_test_lenient_flags_8(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_invoke_llhttp__on_status_complete;
      default:
        goto s_n_llhttp__internal__n_error_93;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_94: {
    state->error = 0x19;
    state->reason = "Missing expected CR after response line";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) (p + 1);
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_invoke_test_lenient_flags_30;
      return s_error;
    }
    p++;
    goto s_n_llhttp__internal__n_invoke_test_lenient_flags_30;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_status_1: {
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_95: {
    state->error = 0xd;
    state->reason = "Invalid response status";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
  s_n_llhttp__internal__n_invoke_mul_add_status_code_2: {
    switch (llhttp__internal__c_mul_add_status_code(state, p, endp, match)) {
      case 1:
        goto s_n_llhttp__internal__n_error_96;
      default:
        goto s_n_llhttp__internal__n_res_status_code_otherwise;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_97: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
  s_n_llhttp__internal__n_invoke_mul_add_status_code_1: {
    switch (llhttp__internal__c_mul_add_status_code(state, p, endp, match)) {
      case 1:
        goto s_n_llhttp__internal__n_error_98;
      default:
        goto s_n_llhttp__internal__n_res_status_code_digit_3;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_99: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
  s_n_llhttp__internal__n_invoke_mul_add_status_code: {
    switch (llhttp__internal__c_mul_add_status_code(state, p, endp, match)) {
      case 1:
        goto s_n_llhttp__internal__n_error_100;
      default:
        goto s_n_llhttp__internal__n_res_status_code_digit_2;
    }
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_101: {
    state->error = 0xd;
    state->reason = "Invalid status code";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_102: {
    state->error = 0x9;
    state->reason = "Expected space after version";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_90: {
    state->error = 0x21;
    state->reason = "`on_version_complete` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_89;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_89;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_load_http_minor_3: {
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_test_lenient_flags_27: {
    switch (llhttp__internal__c_test_lenient_flags_24(state, p, endp)) {
      case 1:
        goto s_n_llhttp__internal__n_span_end_llhttp__on_version_6;
      default:
        goto s_n_llhttp__internal__n_invoke_load_http_major_1;
  s_n_llhttp__internal__n_invoke_store_http_minor_1: {
    switch (llhttp__internal__c_store_http_minor(state, p, endp, match)) {
      default:
        goto s_n_llhttp__internal__n_invoke_test_lenient_flags_27;
    }
    /* UNREACHABLE */;
    abort();
  }
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_103;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_103;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_span_end_llhttp__on_version_8: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_104;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_104;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_invoke_store_http_major_1: {
    if (err != 0) {
      state->error = err;
      state->error_pos = (const char*) p;
      state->_current = (void*) (intptr_t) s_n_llhttp__internal__n_error_105;
      return s_error;
    }
    goto s_n_llhttp__internal__n_error_105;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_109: {
    state->error = 0x8;
    state->reason = "Expected HTTP/";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_106: {
    state->error = 0x8;
    state->reason = "Invalid word encountered";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
    /* UNREACHABLE */;
    abort();
  }
  s_n_llhttp__internal__n_error_110: {
    state->error = 0x1f;
    state->reason = "`on_reset` callback error";
    state->error_pos = (const char*) p;
    state->_current = (void*) (intptr_t) s_error;
      case 21:
        goto s_n_llhttp__internal__n_pause_28;
      default:
        goto s_n_llhttp__internal__n_error_110;
    }
    /* UNREACHABLE */;
    abort();
  }