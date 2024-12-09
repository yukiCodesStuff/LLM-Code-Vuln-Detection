
/* Copyright (C) 2009-2013 by Daniel Stenberg
 *
 * Permission to use, copy, modify, and distribute this
 * software and its documentation for any purpose and without
 * fee is hereby granted, provided that the above copyright
 * notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting
 * documentation, and that the name of M.I.T. not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 * M.I.T. makes no representations about the suitability of
 * this software for any purpose.  It is provided "as is"
 * without express or implied warranty.
 */


#include "ares_setup.h"

#include <stddef.h>

#include "ares.h"
#include "ares_data.h"
#include "ares_private.h"


/*
** ares_free_data() - c-ares external API function.
**
** This function must be used by the application to free data memory that
** has been internally allocated by some c-ares function and for which a
** pointer has already been returned to the calling application. The list
** of c-ares functions returning pointers that must be free'ed using this
** function is:
**
**   ares_get_servers()
**   ares_parse_srv_reply()
**   ares_parse_txt_reply()
*/

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