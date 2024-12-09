    {
      ares_free_hostent(hostent);
    }

  if (naddrttls)
    {
      /* Truncated to at most *naddrttls entries */
      *naddrttls = (naddrs > *naddrttls)?*naddrttls:naddrs;
    }