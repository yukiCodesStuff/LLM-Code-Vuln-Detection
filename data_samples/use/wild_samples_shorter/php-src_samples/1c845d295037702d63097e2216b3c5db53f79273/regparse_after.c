    }
  }

  if (*state != CCS_START)
    *state = CCS_VALUE;

  *type  = CCV_CLASS;
  return 0;
}
