/*
 * Copyright (c) 2004 by Internet Systems Consortium, Inc. ("ISC")
 * Copyright (c) 1996-1999 by Internet Software Consortium.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ares_setup.h"

#ifdef HAVE_NETINET_IN_H
#  include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
#  include <arpa/inet.h>
#endif
#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
#else
#  include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#  include <arpa/nameser_compat.h>
#endif

#include "ares.h"
#include "ares_ipv6.h"

#ifndef HAVE_INET_NTOP

/*
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4.  sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static const char *inet_ntop4(const unsigned char *src, char *dst, size_t size);
static const char *inet_ntop6(const unsigned char *src, char *dst, size_t size);

/* char *
 * inet_ntop(af, src, dst, size)
 *     convert a network format address to presentation format.
 * return:
 *     pointer to presentation format address (`dst'), or NULL (see errno).
 * note:
 *     On Windows we store the error in the thread errno, not
 *     in the winsock error code. This is to avoid loosing the
 *     actual last winsock error. So use macro ERRNO to fetch the
 *     errno this function sets when returning NULL, not SOCKERRNO.
 * author:
 *     Paul Vixie, 1996.
 */
const char *
ares_inet_ntop(int af, const void *src, char *dst, ares_socklen_t size)
{
  switch (af) {
  case AF_INET:
    return (inet_ntop4(src, dst, (size_t)size));
  case AF_INET6:
    return (inet_ntop6(src, dst, (size_t)size));
  default:
    SET_ERRNO(EAFNOSUPPORT);
    return (NULL);
  }
  /* NOTREACHED */
}