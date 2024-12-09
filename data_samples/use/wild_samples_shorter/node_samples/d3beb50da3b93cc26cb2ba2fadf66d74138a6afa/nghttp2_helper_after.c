  case NGHTTP2_ERR_FLOODED:
    return "Flooding was detected in this HTTP/2 session, and it must be "
           "closed";
  case NGHTTP2_ERR_TOO_MANY_SETTINGS:
    return "SETTINGS frame contained more than the maximum allowed entries";
  default:
    return "Unknown error code";
  }
}