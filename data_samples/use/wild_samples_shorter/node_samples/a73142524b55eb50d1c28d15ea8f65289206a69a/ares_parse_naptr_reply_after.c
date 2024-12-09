          status = ARES_EBADRESP;
          break;
        }
      /* RR must contain at least 7 bytes = 2 x int16 + 3 x name */
      if (rr_len < 7)
        {
          status = ARES_EBADRESP;
          break;
        }

      /* Check if we are really looking at a NAPTR record */
      if (rr_class == C_IN && rr_type == T_NAPTR)
        {

  return ARES_SUCCESS;
}