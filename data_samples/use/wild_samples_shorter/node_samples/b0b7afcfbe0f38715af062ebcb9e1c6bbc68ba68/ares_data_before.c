
void ares_free_data(void *dataptr)
{
  struct ares_data *ptr;

  if (!dataptr)
    return;

#ifdef __INTEL_COMPILER
#  pragma warning(push)
#  pragma warning(disable:1684)
   /* 1684: conversion from pointer to same-sized integral type */
#endif

  ptr = (void *)((char *)dataptr - offsetof(struct ares_data, data));

#ifdef __INTEL_COMPILER
#  pragma warning(pop)
#endif

  if (ptr->mark != ARES_DATATYPE_MARK)
    return;

  switch (ptr->type)
    {
      case ARES_DATATYPE_MX_REPLY:

        if (ptr->data.mx_reply.next)
          ares_free_data(ptr->data.mx_reply.next);
        if (ptr->data.mx_reply.host)
          ares_free(ptr->data.mx_reply.host);
        break;

      case ARES_DATATYPE_SRV_REPLY:

        if (ptr->data.srv_reply.next)
          ares_free_data(ptr->data.srv_reply.next);
        if (ptr->data.srv_reply.host)
          ares_free(ptr->data.srv_reply.host);
        break;

      case ARES_DATATYPE_TXT_REPLY:
      case ARES_DATATYPE_TXT_EXT:

        if (ptr->data.txt_reply.next)
          ares_free_data(ptr->data.txt_reply.next);
        if (ptr->data.txt_reply.txt)
          ares_free(ptr->data.txt_reply.txt);
        break;

      case ARES_DATATYPE_ADDR_NODE:

        if (ptr->data.addr_node.next)
          ares_free_data(ptr->data.addr_node.next);
        break;

      case ARES_DATATYPE_ADDR_PORT_NODE:

        if (ptr->data.addr_port_node.next)
          ares_free_data(ptr->data.addr_port_node.next);
        break;

      case ARES_DATATYPE_NAPTR_REPLY:

        if (ptr->data.naptr_reply.next)
          ares_free_data(ptr->data.naptr_reply.next);
        if (ptr->data.naptr_reply.flags)
          ares_free(ptr->data.naptr_reply.flags);
        if (ptr->data.naptr_reply.service)
          ares_free(ptr->data.naptr_reply.service);
        if (ptr->data.naptr_reply.regexp)
          ares_free(ptr->data.naptr_reply.regexp);
        if (ptr->data.naptr_reply.replacement)
          ares_free(ptr->data.naptr_reply.replacement);
        break;

      case ARES_DATATYPE_SOA_REPLY:
        if (ptr->data.soa_reply.nsname)
          ares_free(ptr->data.soa_reply.nsname);
        if (ptr->data.soa_reply.hostmaster)
          ares_free(ptr->data.soa_reply.hostmaster);
	break;

      default:
        return;
    }

  ares_free(ptr);
}


/*