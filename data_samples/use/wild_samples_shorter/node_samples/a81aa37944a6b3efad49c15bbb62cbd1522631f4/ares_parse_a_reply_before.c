
  if (naddrttls)
    {
      *naddrttls = naddrs;
    }

  ares__freeaddrinfo_cnames(ai.cnames);
  ares__freeaddrinfo_nodes(ai.nodes);