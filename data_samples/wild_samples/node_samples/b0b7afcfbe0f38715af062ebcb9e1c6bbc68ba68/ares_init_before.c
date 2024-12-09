
/* Copyright 1998 by the Massachusetts Institute of Technology.
 * Copyright (C) 2007-2013 by Daniel Stenberg
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

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_ARPA_NAMESER_H
#  include <arpa/nameser.h>
#else
#  include "nameser.h"
#endif
#ifdef HAVE_ARPA_NAMESER_COMPAT_H
#  include <arpa/nameser_compat.h>
#endif

#if defined(ANDROID) || defined(__ANDROID__)
#include <sys/system_properties.h>
/* From the Bionic sources */
#define DNS_PROP_NAME_PREFIX  "net.dns"
#define MAX_DNS_PROPERTIES    8
#endif

#if defined(CARES_USE_LIBRESOLV)
#include <resolv.h>
#endif

#include "ares.h"
#include "ares_inet_net_pton.h"
#include "ares_library_init.h"
#include "ares_nowarn.h"
#include "ares_platform.h"
#include "ares_private.h"

#ifdef WATT32
#undef WIN32  /* Redefined in MingW/MSVC headers */
#endif

static int init_by_options(ares_channel channel,
                           const struct ares_options *options,
                           int optmask);
static int init_by_environment(ares_channel channel);
static int init_by_resolv_conf(ares_channel channel);
static int init_by_defaults(ares_channel channel);

#ifndef WATT32
static int config_nameserver(struct server_state **servers, int *nservers,
                             char *str);
#endif
static int set_search(ares_channel channel, const char *str);
static int set_options(ares_channel channel, const char *str);
static const char *try_option(const char *p, const char *q, const char *opt);
static int init_id_key(rc4_key* key,int key_data_len);

static int config_sortlist(struct apattern **sortlist, int *nsort,
                           const char *str);
static int sortlist_alloc(struct apattern **sortlist, int *nsort,
                          struct apattern *pat);
static int ip_addr(const char *s, ares_ssize_t len, struct in_addr *addr);
static void natural_mask(struct apattern *pat);
#if !defined(WIN32) && !defined(WATT32) && \
    !defined(ANDROID) && !defined(__ANDROID__) && !defined(CARES_USE_LIBRESOLV)
static int config_domain(ares_channel channel, char *str);
static int config_lookup(ares_channel channel, const char *str,
                         const char *bindch, const char *altbindch,
                         const char *filech);
static char *try_config(char *s, const char *opt, char scc);
#endif

#define ARES_CONFIG_CHECK(x) (x->lookups && x->nsort > -1 && \
                             x->nservers > -1 && \
                             x->ndomains > -1 && \
                             x->ndots > -1 && x->timeout > -1 && \
                             x->tries > -1)

int ares_init(ares_channel *channelptr)
{
  return ares_init_options(channelptr, NULL, 0);
}
{
  DWORD size = 0;
  int   res;

  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
     might have been stored without null termination */
  *outptr = ares_malloc(size+1);
  if (!*outptr)
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
    *outptr = NULL;
    return 0;
  }
{
  DWORD size = 0;
  int   res;

  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
     might have been stored without null termination */
  *outptr = ares_malloc(size+1);
  if (!*outptr)
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
    *outptr = NULL;
    return 0;
  }
{
  DWORD dataType = 0;
  DWORD size = 0;
  int   res;

  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
     might have been stored without null termination */
  *outptr = ares_malloc(size+1);
  if (!*outptr)
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
    *outptr = NULL;
    return 0;
  }
{
  DWORD dataType = 0;
  DWORD size = 0;
  int   res;

  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
     might have been stored without null termination */
  *outptr = ares_malloc(size+1);
  if (!*outptr)
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
    *outptr = NULL;
    return 0;
  }
  {
    enumKeyNameBuffSize = sizeof(enumKeyName);
    res = RegEnumKeyEx(hKeyParent, enumKeyIdx++, enumKeyName,
                       &enumKeyNameBuffSize, 0, NULL, NULL, NULL);
    if (res != ERROR_SUCCESS)
      break;
    res = RegOpenKeyEx(hKeyParent, enumKeyName, 0, KEY_QUERY_VALUE,
                       &hKeyEnum);
    if (res != ERROR_SUCCESS)
      continue;
    gotString = get_REG_SZ(hKeyEnum, leafKeyName, outptr);
    RegCloseKey(hKeyEnum);
    if (gotString)
      break;
  }
{
  HKEY hKey_VxD_MStcp;
  int  gotString;
  int  res;

  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_9X, 0, KEY_READ,
                     &hKey_VxD_MStcp);
  if (res != ERROR_SUCCESS)
    return 0;

  gotString = get_REG_SZ_9X(hKey_VxD_MStcp, NAMESERVER, outptr);
  RegCloseKey(hKey_VxD_MStcp);

  if (!gotString || !*outptr)
    return 0;

  return 1;
}

/*
 * get_DNS_Registry_NT()
 *
 * Functionally identical to get_DNS_Registry()
 *
 * Refs: Microsoft Knowledge Base articles KB120642 and KB314053.
 *
 * Implementation supports Windows NT 3.5 and newer.
 */
static int get_DNS_Registry_NT(char **outptr)
{
  HKEY hKey_Interfaces = NULL;
  HKEY hKey_Tcpip_Parameters;
  int  gotString;
  int  res;

  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0, KEY_READ,
                     &hKey_Tcpip_Parameters);
  if (res != ERROR_SUCCESS)
    return 0;

  /*
  ** Global DNS settings override adapter specific parameters when both
  ** are set. Additionally static DNS settings override DHCP-configured
  ** parameters when both are set.
  */

  /* Global DNS static parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, NAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Global DNS DHCP-configured parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, DHCPNAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Try adapter specific parameters */
  res = RegOpenKeyEx(hKey_Tcpip_Parameters, "Interfaces", 0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hKey_Interfaces);
  if (res != ERROR_SUCCESS)
  {
    hKey_Interfaces = NULL;
    goto done;
  }
{
  HKEY hKey_Interfaces = NULL;
  HKEY hKey_Tcpip_Parameters;
  int  gotString;
  int  res;

  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0, KEY_READ,
                     &hKey_Tcpip_Parameters);
  if (res != ERROR_SUCCESS)
    return 0;

  /*
  ** Global DNS settings override adapter specific parameters when both
  ** are set. Additionally static DNS settings override DHCP-configured
  ** parameters when both are set.
  */

  /* Global DNS static parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, NAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Global DNS DHCP-configured parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, DHCPNAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Try adapter specific parameters */
  res = RegOpenKeyEx(hKey_Tcpip_Parameters, "Interfaces", 0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hKey_Interfaces);
  if (res != ERROR_SUCCESS)
  {
    hKey_Interfaces = NULL;
    goto done;
  }
{
  HKEY hKey_Interfaces = NULL;
  HKEY hKey_Tcpip_Parameters;
  int  gotString;
  int  res;

  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0, KEY_READ,
                     &hKey_Tcpip_Parameters);
  if (res != ERROR_SUCCESS)
    return 0;

  /*
  ** Global DNS settings override adapter specific parameters when both
  ** are set. Additionally static DNS settings override DHCP-configured
  ** parameters when both are set.
  */

  /* Global DNS static parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, NAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Global DNS DHCP-configured parameters */
  gotString = get_REG_SZ(hKey_Tcpip_Parameters, DHCPNAMESERVER, outptr);
  if (gotString)
    goto done;

  /* Try adapter specific parameters */
  res = RegOpenKeyEx(hKey_Tcpip_Parameters, "Interfaces", 0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hKey_Interfaces);
  if (res != ERROR_SUCCESS)
  {
    hKey_Interfaces = NULL;
    goto done;
  }
{
  OSVERSIONINFO vinfo;
  memset(&vinfo, 0, sizeof(vinfo));
  vinfo.dwOSVersionInfoSize = sizeof(vinfo);
  if (!GetVersionEx(&vinfo) || vinfo.dwMajorVersion < 6)
    return FALSE;
  return TRUE;
}

/* A structure to hold the string form of IPv4 and IPv6 addresses so we can
 * sort them by a metric.
 */
typedef struct
{
  /* The metric we sort them by. */
  ULONG metric;

  /* Room enough for the string form of any IPv4 or IPv6 address that
   * ares_inet_ntop() will create.  Based on the existing c-ares practice.
   */
  char text[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
} Address;

/* Sort Address values \a left and \a right by metric, returning the usual
 * indicators for qsort().
 */
static int compareAddresses(const void *arg1,
                            const void *arg2)
{
  const Address * const left = arg1;
  const Address * const right = arg2;
  if(left->metric < right->metric) return -1;
  if(left->metric > right->metric) return 1;
  return 0;
}
{
  /* The metric we sort them by. */
  ULONG metric;

  /* Room enough for the string form of any IPv4 or IPv6 address that
   * ares_inet_ntop() will create.  Based on the existing c-ares practice.
   */
  char text[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
} Address;

/* Sort Address values \a left and \a right by metric, returning the usual
 * indicators for qsort().
 */
static int compareAddresses(const void *arg1,
                            const void *arg2)
{
  const Address * const left = arg1;
  const Address * const right = arg2;
  if(left->metric < right->metric) return -1;
  if(left->metric > right->metric) return 1;
  return 0;
}
{
  const Address * const left = arg1;
  const Address * const right = arg2;
  if(left->metric < right->metric) return -1;
  if(left->metric > right->metric) return 1;
  return 0;
}

/* There can be multiple routes to "the Internet".  And there can be different
 * DNS servers associated with each of the interfaces that offer those routes.
 * We have to assume that any DNS server can serve any request.  But, some DNS
 * servers may only respond if requested over their associated interface.  But
 * we also want to use "the preferred route to the Internet" whenever possible
 * (and not use DNS servers on a non-preferred route even by forcing request
 * to go out on the associated non-preferred interface).  i.e. We want to use
 * the DNS servers associated with the same interface that we would use to
 * make a general request to anything else.
 *
 * But, Windows won't sort the DNS servers by the metrics associated with the
 * routes and interfaces _even_ though it obviously sends IP packets based on
 * those same routes and metrics.  So, we must do it ourselves.
 *
 * So, we sort the DNS servers by the same metric values used to determine how
 * an outgoing IP packet will go, thus effectively using the DNS servers
 * associated with the interface that the DNS requests themselves will
 * travel.  This gives us optimal routing and avoids issues where DNS servers
 * won't respond to requests that don't arrive via some specific subnetwork
 * (and thus some specific interface).
 *
 * This function computes the metric we use to sort.  On the interface
 * identified by \a luid, it determines the best route to \a dest and combines
 * that route's metric with \a interfaceMetric to compute a metric for the
 * destination address on that interface.  This metric can be used as a weight
 * to sort the DNS server addresses associated with each interface (lower is
 * better).
 *
 * Note that by restricting the route search to the specific interface with
 * which the DNS servers are associated, this function asks the question "What
 * is the metric for sending IP packets to this DNS server?" which allows us
 * to sort the DNS servers correctly.
 */
static ULONG getBestRouteMetric(IF_LUID * const luid, /* Can't be const :( */
                                const SOCKADDR_INET * const dest,
                                const ULONG interfaceMetric)
{
  /* On this interface, get the best route to that destination. */
  MIB_IPFORWARD_ROW2 row;
  SOCKADDR_INET ignored;
  if(!ares_fpGetBestRoute2 ||
     ares_fpGetBestRoute2(/* The interface to use.  The index is ignored since we are
                           * passing a LUID.
                           */
                           luid, 0,
                           /* No specific source address. */
                           NULL,
                           /* Our destination address. */
                           dest,
                           /* No options. */
                           0,
                           /* The route row. */
                           &row,
                           /* The best source address, which we don't need. */
                           &ignored) != NO_ERROR
     /* If the metric is "unused" (-1) or too large for us to add the two
      * metrics, use the worst possible, thus sorting this last.
      */
     || row.Metric == (ULONG)-1
     || row.Metric > ((ULONG)-1) - interfaceMetric) {
    /* Return the worst possible metric. */
    return (ULONG)-1;
  }
        {
          addresses[addressesIndex].metric = -1;
        }

        if (! ares_inet_ntop(AF_INET, &namesrvr.sa4->sin_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
        }
        ++addressesIndex;
      }
      else if (namesrvr.sa->sa_family == AF_INET6)
      {
        /* Windows apparently always reports some IPv6 DNS servers that
         * prefixed with fec0:0:0:ffff. These ususally do not point to
         * working DNS servers, so we ignore them. */
        if (strncmp(addresses[addressesIndex].text, "fec0:0:0:ffff:", 14) == 0)
          continue;
        if (memcmp(&namesrvr.sa6->sin6_addr, &ares_in6addr_any,
                   sizeof(namesrvr.sa6->sin6_addr)) == 0)
          continue;

        /* Allocate room for another address, if necessary, else skip. */
        if(addressesIndex == addressesSize) {
          const size_t newSize = addressesSize + 4;
          Address * const newMem =
            (Address*)ares_realloc(addresses, sizeof(Address) * newSize);
          if(newMem == NULL) {
            continue;
          }
                             sizeof(addresses[0].text))) {
          continue;
        }
        ++addressesIndex;
      }
      else if (namesrvr.sa->sa_family == AF_INET6)
      {
        /* Windows apparently always reports some IPv6 DNS servers that
         * prefixed with fec0:0:0:ffff. These ususally do not point to
         * working DNS servers, so we ignore them. */
        if (strncmp(addresses[addressesIndex].text, "fec0:0:0:ffff:", 14) == 0)
          continue;
        if (memcmp(&namesrvr.sa6->sin6_addr, &ares_in6addr_any,
                   sizeof(namesrvr.sa6->sin6_addr)) == 0)
          continue;

        /* Allocate room for another address, if necessary, else skip. */
        if(addressesIndex == addressesSize) {
          const size_t newSize = addressesSize + 4;
          Address * const newMem =
            (Address*)ares_realloc(addresses, sizeof(Address) * newSize);
          if(newMem == NULL) {
            continue;
          }
          addresses = newMem;
          addressesSize = newSize;
        }

        /* Vista required for Luid or Ipv4Metric */
        if (ares_IsWindowsVistaOrGreater())
        {
          /* Save the address as the next element in addresses. */
          addresses[addressesIndex].metric =
            getBestRouteMetric(&ipaaEntry->Luid,
                               (SOCKADDR_INET*)(namesrvr.sa),
                               ipaaEntry->Ipv6Metric);
        }
        else
        {
          addresses[addressesIndex].metric = -1;
        }

        if (! ares_inet_ntop(AF_INET6, &namesrvr.sa6->sin6_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
        }
        ++addressesIndex;
      }
        {
          addresses[addressesIndex].metric = -1;
        }

        if (! ares_inet_ntop(AF_INET6, &namesrvr.sa6->sin6_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
        }
        ++addressesIndex;
      }
      else {
        /* Skip non-IPv4/IPv6 addresses completely. */
        continue;
      }
    }
  }

  /* Sort all of the textual addresses by their metric. */
  qsort(addresses, addressesIndex, sizeof(*addresses), compareAddresses);

  /* Join them all into a single string, removing duplicates. */
  {
    size_t i;
    for(i = 0; i < addressesIndex; ++i) {
      size_t j;
      /* Look for this address text appearing previously in the results. */
      for(j = 0; j < i; ++j) {
        if(strcmp(addresses[j].text, addresses[i].text) == 0) {
          break;
        }
      }
      /* Iff we didn't emit this address already, emit it now. */
      if(j == i) {
        /* Add that to outptr (if we can). */
        commajoin(outptr, addresses[i].text);
      }
    }
  }

done:
  ares_free(addresses);

  if (ipaa)
    ares_free(ipaa);

  if (!*outptr) {
    return 0;
  }

  return 1;
}

/*
 * get_DNS_Windows()
 *
 * Locates DNS info from Windows employing most suitable methods available at
 * run-time no matter which Windows version it is. When located, this returns
 * a pointer in *outptr to a newly allocated memory area holding a string with
 * a space or comma seperated list of DNS IP addresses, null-terminated.
 *
 * Returns 0 and nullifies *outptr upon inability to return DNSes string.
 *
 * Returns 1 and sets *outptr when returning a dynamically allocated string.
 *
 * Implementation supports Windows 95 and newer.
 */
static int get_DNS_Windows(char **outptr)
{
  /* Try using IP helper API GetAdaptersAddresses(). IPv4 + IPv6, also sorts
   * DNS servers by interface route metrics to try to use the best DNS server. */
  if (get_DNS_AdaptersAddresses(outptr))
    return 1;

  /* Try using IP helper API GetNetworkParams(). IPv4 only. */
  if (get_DNS_NetworkParams(outptr))
    return 1;

  /* Fall-back to registry information */
  return get_DNS_Registry(outptr);
}

static void replace_comma_by_space(char* str)
{
  /* replace ',' by ' ' to coincide with resolv.conf search parameter */
  char *p;
  for (p = str; *p != '\0'; p++)
  {
    if (*p == ',')
      *p = ' ';
  }
}

/* Search if 'suffix' is containted in the 'searchlist'. Returns true if yes,
 * otherwise false. 'searchlist' is a comma separated list of domain suffixes,
 * 'suffix' is one domain suffix, 'len' is the length of 'suffix'.
 * The search ignores case. E.g.:
 * contains_suffix("abc.def,ghi.jkl", "ghi.JKL") returns true  */
static bool contains_suffix(const char* const searchlist,
                            const char* const suffix, const size_t len)
{
  const char* beg = searchlist;
  const char* end;
  if (!*suffix)
    return true;
  for (;;)
  {
    while (*beg && (ISSPACE(*beg) || (*beg == ',')))
      ++beg;
    if (!*beg)
      return false;
    end = beg;
    while (*end && !ISSPACE(*end) && (*end != ','))
      ++end;
    if (len == (end - beg) && !strnicmp(beg, suffix, len))
      return true;
    beg = end;
  }
}

/* advances list to the next suffix within a comma separated search list.
 * len is the length of the next suffix. */
static size_t next_suffix(const char** list, const size_t advance)
{
  const char* beg = *list + advance;
  const char* end;
  while (*beg && (ISSPACE(*beg) || (*beg == ',')))
    ++beg;
  end = beg;
  while (*end && !ISSPACE(*end) && (*end != ','))
    ++end;
  *list = beg;
  return end - beg;
}

/*
 * get_SuffixList_Windows()
 *
 * Reads the "DNS Suffix Search List" from registry and writes the list items
 * whitespace separated to outptr. If the Search List is empty, the
 * "Primary Dns Suffix" is written to outptr.
 *
 * Returns 0 and nullifies *outptr upon inability to return the suffix list.
 *
 * Returns 1 and sets *outptr when returning a dynamically allocated string.
 *
 * Implementation supports Windows Server 2003 and newer
 */
static int get_SuffixList_Windows(char **outptr)
{
  HKEY hKey, hKeyEnum;
  char  keyName[256];
  DWORD keyNameBuffSize;
  DWORD keyIdx = 0;
  char *p = NULL;
  char *pp;
  size_t len = 0;

  *outptr = NULL;

  if (ares__getplatform() != WIN_NT)
    return 0;

  /* 1. Global DNS Suffix Search List */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);
    RegCloseKey(hKey);
    if (*outptr)
      return 1;
  }

  /* 2. Connection Specific Search List composed of:
   *  a. Primary DNS Suffix */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_DNSCLIENT, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    get_REG_SZ(hKey, PRIMARYDNSSUFFIX_KEY, outptr);
    RegCloseKey(hKey);
  }
  if (!*outptr)
    return 0;

  /*  b. Interface SearchList, Domain, DhcpDomain */
  if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY "\\" INTERFACES_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
    return 0;
  for(;;)
  {
    keyNameBuffSize = sizeof(keyName);
    if (RegEnumKeyEx(hKey, keyIdx++, keyName, &keyNameBuffSize,
        0, NULL, NULL, NULL)
        != ERROR_SUCCESS)
      break;
    if (RegOpenKeyEx(hKey, keyName, 0, KEY_QUERY_VALUE, &hKeyEnum)
        != ERROR_SUCCESS)
      continue;
    if (get_REG_SZ(hKeyEnum, SEARCHLIST_KEY, &p) ||
        get_REG_SZ(hKeyEnum, DOMAIN_KEY, &p) ||
        get_REG_SZ(hKeyEnum, DHCPDOMAIN_KEY, &p))
    {
      /* p can be comma separated (SearchList) */
      pp = p;
      while (len = next_suffix(&pp, len))
      {
        if (!contains_suffix(*outptr, pp, len))
          commanjoin(outptr, pp, len);
      }
      ares_free(p);
      p = NULL;
    }
    RegCloseKey(hKeyEnum);
  }
  RegCloseKey(hKey);
  if (*outptr)
    replace_comma_by_space(*outptr);
  return *outptr != NULL;
}

#endif

static int init_by_resolv_conf(ares_channel channel)
{
#if !defined(ANDROID) && !defined(__ANDROID__) && !defined(WATT32) && \
    !defined(CARES_USE_LIBRESOLV)
  char *line = NULL;
#endif
  int status = -1, nservers = 0, nsort = 0;
  struct server_state *servers = NULL;
  struct apattern *sortlist = NULL;

#ifdef WIN32

  if (channel->nservers > -1)  /* don't override ARES_OPT_SERVER */
     return ARES_SUCCESS;

  if (get_DNS_Windows(&line))
  {
    status = config_nameserver(&servers, &nservers, line);
    ares_free(line);
  }

  if (channel->ndomains == -1 && get_SuffixList_Windows(&line))
  {
      status = set_search(channel, line);
      ares_free(line);
  }

  if (status == ARES_SUCCESS)
    status = ARES_EOF;
  else
    /* Catch the case when all the above checks fail (which happens when there
       is no network card or the cable is unplugged) */
    status = ARES_EFILE;

#elif defined(__riscos__)

  /* Under RISC OS, name servers are listed in the
     system variable Inet$Resolvers, space separated. */

  line = getenv("Inet$Resolvers");
  status = ARES_EOF;
  if (line) {
    char *resolvers = ares_strdup(line), *pos, *space;

    if (!resolvers)
      return ARES_ENOMEM;

    pos = resolvers;
    do {
      space = strchr(pos, ' ');
      if (space)
        *space = '\0';
      status = config_nameserver(&servers, &nservers, pos);
      if (status != ARES_SUCCESS)
        break;
      pos = space + 1;
    } while (space);

    if (status == ARES_SUCCESS)
      status = ARES_EOF;

    ares_free(resolvers);
  }

#elif defined(WATT32)
  int i;

  sock_init();
  for (i = 0; def_nameservers[i]; i++)
      ;
  if (i == 0)
    return ARES_SUCCESS; /* use localhost DNS server */

  nservers = i;
  servers = ares_malloc(sizeof(struct server_state));
  if (!servers)
     return ARES_ENOMEM;
  memset(servers, 0, sizeof(struct server_state));

  for (i = 0; def_nameservers[i]; i++)
  {
    servers[i].addr.addrV4.s_addr = htonl(def_nameservers[i]);
    servers[i].addr.family = AF_INET;
    servers[i].addr.udp_port = 0;
    servers[i].addr.tcp_port = 0;
  }
  status = ARES_EOF;

#elif defined(ANDROID) || defined(__ANDROID__)
  unsigned int i;
  char propname[PROP_NAME_MAX];
  char propvalue[PROP_VALUE_MAX]="";

  for (i = 1; i <= MAX_DNS_PROPERTIES; i++) {
    snprintf(propname, sizeof(propname), "%s%u", DNS_PROP_NAME_PREFIX, i);
    if (__system_property_get(propname, propvalue) < 1) {
      status = ARES_EOF;
      break;
    }
    status = config_nameserver(&servers, &nservers, propvalue);
    if (status != ARES_SUCCESS)
      break;
    status = ARES_EOF;
  }
#elif defined(CARES_USE_LIBRESOLV)
  struct __res_state res;
  memset(&res, 0, sizeof(res));
  int result = res_ninit(&res);
  if (result == 0 && (res.options & RES_INIT)) {
    status = ARES_EOF;

    if (channel->nservers == -1) {
      union res_sockaddr_union addr[MAXNS];
      int nscount = res_getservers(&res, addr, MAXNS);
      for (int i = 0; i < nscount; ++i) {
        char str[INET6_ADDRSTRLEN];
        int config_status;
        sa_family_t family = addr[i].sin.sin_family;
        if (family == AF_INET) {
          ares_inet_ntop(family, &addr[i].sin.sin_addr, str, sizeof(str));
        } else if (family == AF_INET6) {
          ares_inet_ntop(family, &addr[i].sin6.sin6_addr, str, sizeof(str));
        } else {
          continue;
        }

        config_status = config_nameserver(&servers, &nservers, str);
        if (config_status != ARES_SUCCESS) {
          status = config_status;
          break;
        }
      }
    }
      else {
        /* Skip non-IPv4/IPv6 addresses completely. */
        continue;
      }
    }
  }

  /* Sort all of the textual addresses by their metric. */
  qsort(addresses, addressesIndex, sizeof(*addresses), compareAddresses);

  /* Join them all into a single string, removing duplicates. */
  {
    size_t i;
    for(i = 0; i < addressesIndex; ++i) {
      size_t j;
      /* Look for this address text appearing previously in the results. */
      for(j = 0; j < i; ++j) {
        if(strcmp(addresses[j].text, addresses[i].text) == 0) {
          break;
        }
      }
      /* Iff we didn't emit this address already, emit it now. */
      if(j == i) {
        /* Add that to outptr (if we can). */
        commajoin(outptr, addresses[i].text);
      }
    }
{
  HKEY hKey, hKeyEnum;
  char  keyName[256];
  DWORD keyNameBuffSize;
  DWORD keyIdx = 0;
  char *p = NULL;
  char *pp;
  size_t len = 0;

  *outptr = NULL;

  if (ares__getplatform() != WIN_NT)
    return 0;

  /* 1. Global DNS Suffix Search List */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);
    RegCloseKey(hKey);
    if (*outptr)
      return 1;
  }
{
  HKEY hKey, hKeyEnum;
  char  keyName[256];
  DWORD keyNameBuffSize;
  DWORD keyIdx = 0;
  char *p = NULL;
  char *pp;
  size_t len = 0;

  *outptr = NULL;

  if (ares__getplatform() != WIN_NT)
    return 0;

  /* 1. Global DNS Suffix Search List */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);
    RegCloseKey(hKey);
    if (*outptr)
      return 1;
  }
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);
    RegCloseKey(hKey);
    if (*outptr)
      return 1;
  }

  /* 2. Connection Specific Search List composed of:
   *  a. Primary DNS Suffix */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_DNSCLIENT, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    get_REG_SZ(hKey, PRIMARYDNSSUFFIX_KEY, outptr);
    RegCloseKey(hKey);
  }
  {
    get_REG_SZ(hKey, PRIMARYDNSSUFFIX_KEY, outptr);
    RegCloseKey(hKey);
  }
  if (!*outptr)
    return 0;

  /*  b. Interface SearchList, Domain, DhcpDomain */
  if (!RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY "\\" INTERFACES_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
    return 0;
  for(;;)
  {
    keyNameBuffSize = sizeof(keyName);
    if (RegEnumKeyEx(hKey, keyIdx++, keyName, &keyNameBuffSize,
        0, NULL, NULL, NULL)
        != ERROR_SUCCESS)
      break;
    if (RegOpenKeyEx(hKey, keyName, 0, KEY_QUERY_VALUE, &hKeyEnum)
        != ERROR_SUCCESS)
      continue;
    if (get_REG_SZ(hKeyEnum, SEARCHLIST_KEY, &p) ||
        get_REG_SZ(hKeyEnum, DOMAIN_KEY, &p) ||
        get_REG_SZ(hKeyEnum, DHCPDOMAIN_KEY, &p))
    {
      /* p can be comma separated (SearchList) */
      pp = p;
      while (len = next_suffix(&pp, len))
      {
        if (!contains_suffix(*outptr, pp, len))
          commanjoin(outptr, pp, len);
      }
      ares_free(p);
      p = NULL;
    }
    RegCloseKey(hKeyEnum);
  }
    {
      /* p can be comma separated (SearchList) */
      pp = p;
      while (len = next_suffix(&pp, len))
      {
        if (!contains_suffix(*outptr, pp, len))
          commanjoin(outptr, pp, len);
      }
  {
    servers[i].addr.addrV4.s_addr = htonl(def_nameservers[i]);
    servers[i].addr.family = AF_INET;
    servers[i].addr.udp_port = 0;
    servers[i].addr.tcp_port = 0;
  }
  status = ARES_EOF;

#elif defined(ANDROID) || defined(__ANDROID__)
  unsigned int i;
  char propname[PROP_NAME_MAX];
  char propvalue[PROP_VALUE_MAX]="";

  for (i = 1; i <= MAX_DNS_PROPERTIES; i++) {
    snprintf(propname, sizeof(propname), "%s%u", DNS_PROP_NAME_PREFIX, i);
    if (__system_property_get(propname, propvalue) < 1) {
      status = ARES_EOF;
      break;
    }
    status = config_nameserver(&servers, &nservers, propvalue);
    if (status != ARES_SUCCESS)
      break;
    status = ARES_EOF;
  }
        if(!p) {
          rc = ARES_ENOMEM;
          goto error;
        }
        hostname = p;
        continue;
      }
      else if(res) {
        rc = ARES_EBADNAME;
        goto error;
      }

    } while (res != 0);

    dot = strchr(hostname, '.');
    if (dot) {
      /* a dot was found */
      channel->domains = ares_malloc(sizeof(char *));
      if (!channel->domains) {
        rc = ARES_ENOMEM;
        goto error;
      }