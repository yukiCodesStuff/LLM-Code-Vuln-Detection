        {
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
          /* parse the NAPTR record itself */

          /* Allocate storage for this NAPTR answer appending it to the list */
          naptr_curr = ares_malloc_data(ARES_DATATYPE_NAPTR_REPLY);
          if (!naptr_curr)
            {
              status = ARES_ENOMEM;
              break;
            }
    {
      if (naptr_head)
        ares_free_data (naptr_head);
      return status;
    }

  /* everything looks fine, return the data */
  *naptr_out = naptr_head;

  return ARES_SUCCESS;
}