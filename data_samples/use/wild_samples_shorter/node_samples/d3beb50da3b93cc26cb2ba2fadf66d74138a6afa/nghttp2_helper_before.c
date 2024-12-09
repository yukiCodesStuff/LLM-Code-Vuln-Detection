  case NGHTTP2_ERR_FLOODED:
    return "Flooding was detected in this HTTP/2 session, and it must be "
           "closed";
  default:
    return "Unknown error code";
  }
}