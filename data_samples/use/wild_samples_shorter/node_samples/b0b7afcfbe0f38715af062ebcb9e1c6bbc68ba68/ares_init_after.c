
#if defined(ANDROID) || defined(__ANDROID__)
#include <sys/system_properties.h>
#include "ares_android.h"
/* From the Bionic sources */
#define DNS_PROP_NAME_PREFIX  "net.dns"
#define MAX_DNS_PROPERTIES    8
#endif
  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueExA(hKey, leafKeyName, 0, NULL, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
    return 0;

  /* Get the value for real */
  res = RegQueryValueExA(hKey, leafKeyName, 0, NULL,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueExA(hKey, leafKeyName, 0, &dataType, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
    return 0;

  /* Get the value for real */
  res = RegQueryValueExA(hKey, leafKeyName, 0, &dataType,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
  for(;;)
  {
    enumKeyNameBuffSize = sizeof(enumKeyName);
    res = RegEnumKeyExA(hKeyParent, enumKeyIdx++, enumKeyName,
                       &enumKeyNameBuffSize, 0, NULL, NULL, NULL);
    if (res != ERROR_SUCCESS)
      break;
    res = RegOpenKeyExA(hKeyParent, enumKeyName, 0, KEY_QUERY_VALUE,
                       &hKeyEnum);
    if (res != ERROR_SUCCESS)
      continue;
    gotString = get_REG_SZ(hKeyEnum, leafKeyName, outptr);

  *outptr = NULL;

  res = RegOpenKeyExA(HKEY_LOCAL_MACHINE, WIN_NS_9X, 0, KEY_READ,
                     &hKey_VxD_MStcp);
  if (res != ERROR_SUCCESS)
    return 0;


  *outptr = NULL;

  res = RegOpenKeyExA(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0, KEY_READ,
                     &hKey_Tcpip_Parameters);
  if (res != ERROR_SUCCESS)
    return 0;

    goto done;

  /* Try adapter specific parameters */
  res = RegOpenKeyExA(hKey_Tcpip_Parameters, "Interfaces", 0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hKey_Interfaces);
  if (res != ERROR_SUCCESS)
  {
  OSVERSIONINFO vinfo;
  memset(&vinfo, 0, sizeof(vinfo));
  vinfo.dwOSVersionInfoSize = sizeof(vinfo);
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4996) /* warning C4996: 'GetVersionExW': was declared deprecated */
#endif
  if (!GetVersionEx(&vinfo) || vinfo.dwMajorVersion < 6)
    return FALSE;
  return TRUE;
#ifdef _MSC_VER
#pragma warning(pop)
#endif
}

/* A structure to hold the string form of IPv4 and IPv6 addresses so we can
 * sort them by a metric.
  /* The metric we sort them by. */
  ULONG metric;

  /* Original index of the item, used as a secondary sort parameter to make
   * qsort() stable if the metrics are equal */
  size_t orig_idx;

  /* Room enough for the string form of any IPv4 or IPv6 address that
   * ares_inet_ntop() will create.  Based on the existing c-ares practice.
   */
  char text[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
{
  const Address * const left = arg1;
  const Address * const right = arg2;
  /* Lower metric the more preferred */
  if(left->metric < right->metric) return -1;
  if(left->metric > right->metric) return 1;
  /* If metrics are equal, lower original index more preferred */
  if(left->orig_idx < right->orig_idx) return -1;
  if(left->orig_idx > right->orig_idx) return 1;
  return 0;
}

/* Validate that the ip address matches the subnet (network base and network
 * mask) specified. Addresses are specified in standard Network Byte Order as
 * 16 bytes, and the netmask is 0 to 128 (bits).
 */
static int ares_ipv6_subnet_matches(const unsigned char netbase[16],
                                    unsigned char netmask,
                                    const unsigned char ipaddr[16])
{
  unsigned char mask[16] = { 0 };
  unsigned char i;

  /* Misuse */
  if (netmask > 128)
    return 0;

  /* Quickly set whole bytes */
  memset(mask, 0xFF, netmask / 8);

  /* Set remaining bits */
  if(netmask % 8) {
    mask[netmask / 8] = (unsigned char)(0xff << (8 - (netmask % 8)));
  }

  for (i=0; i<16; i++) {
    if ((netbase[i] & mask[i]) != (ipaddr[i] & mask[i]))
      return 0;
  }

  return 1;
}

static int ares_ipv6_server_blacklisted(const unsigned char ipaddr[16])
{
  const struct {
    const char   *netbase;
    unsigned char netmask;
  } blacklist[] = {
    /* Deprecated by [RFC3879] in September 2004. Formerly a Site-Local scoped
     * address prefix. Causes known issues on Windows as these are not valid DNS
     * servers. */
    { "fec0::", 10 },
    { NULL,     0  }
  };
  size_t i;

  for (i=0; blacklist[i].netbase != NULL; i++) {
    unsigned char netbase[16];

    if (ares_inet_pton(AF_INET6, blacklist[i].netbase, netbase) != 1)
      continue;

    if (ares_ipv6_subnet_matches(netbase, blacklist[i].netmask, ipaddr))
      return 1;
  }
  return 0;
}

/* There can be multiple routes to "the Internet".  And there can be different
          addresses[addressesIndex].metric = -1;
        }

        /* Record insertion index to make qsort stable */
        addresses[addressesIndex].orig_idx = addressesIndex;

        if (! ares_inet_ntop(AF_INET, &namesrvr.sa4->sin_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
      }
      else if (namesrvr.sa->sa_family == AF_INET6)
      {
        if (memcmp(&namesrvr.sa6->sin6_addr, &ares_in6addr_any,
                   sizeof(namesrvr.sa6->sin6_addr)) == 0)
          continue;

        if (ares_ipv6_server_blacklisted(
              (const unsigned char *)&namesrvr.sa6->sin6_addr)
           )
          continue;

        /* Allocate room for another address, if necessary, else skip. */
        if(addressesIndex == addressesSize) {
          const size_t newSize = addressesSize + 4;
          Address * const newMem =
          addresses[addressesIndex].metric = -1;
        }

        /* Record insertion index to make qsort stable */
        addresses[addressesIndex].orig_idx = addressesIndex;

        if (! ares_inet_ntop(AF_INET6, &namesrvr.sa6->sin6_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
    }
  }

  /* Sort all of the textual addresses by their metric (and original index if
   * metrics are equal). */
  qsort(addresses, addressesIndex, sizeof(*addresses), compareAddresses);

  /* Join them all into a single string, removing duplicates. */
  {
  DWORD keyNameBuffSize;
  DWORD keyIdx = 0;
  char *p = NULL;
  const char *pp;
  size_t len = 0;

  *outptr = NULL;

    return 0;

  /* 1. Global DNS Suffix Search List */
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);

  /* 2. Connection Specific Search List composed of:
   *  a. Primary DNS Suffix */
  if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, WIN_DNSCLIENT, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    get_REG_SZ(hKey, PRIMARYDNSSUFFIX_KEY, outptr);
    RegCloseKey(hKey);
    return 0;

  /*  b. Interface SearchList, Domain, DhcpDomain */
  if (!RegOpenKeyExA(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY "\\" INTERFACES_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
    return 0;
  for(;;)
  {
    keyNameBuffSize = sizeof(keyName);
    if (RegEnumKeyExA(hKey, keyIdx++, keyName, &keyNameBuffSize,
        0, NULL, NULL, NULL)
        != ERROR_SUCCESS)
      break;
    if (RegOpenKeyExA(hKey, keyName, 0, KEY_QUERY_VALUE, &hKeyEnum)
        != ERROR_SUCCESS)
      continue;
    if (get_REG_SZ(hKeyEnum, SEARCHLIST_KEY, &p) ||
        get_REG_SZ(hKeyEnum, DOMAIN_KEY, &p) ||
    {
      /* p can be comma separated (SearchList) */
      pp = p;
      while ((len = next_suffix(&pp, len)) != 0)
      {
        if (!contains_suffix(*outptr, pp, len))
          commanjoin(outptr, pp, len);
      }
  unsigned int i;
  char propname[PROP_NAME_MAX];
  char propvalue[PROP_VALUE_MAX]="";
  char **dns_servers;
  size_t num_servers;

  /* Use the Android connectivity manager to get a list
   * of DNS servers. As of Android 8 (Oreo) net.dns#
   * system properties are no longer available. Google claims this
   * improves privacy. Apps now need the ACCESS_NETWORK_STATE
   * permission and must use the ConnectivityManager which
   * is Java only. */
  dns_servers = ares_get_android_server_list(MAX_DNS_PROPERTIES, &num_servers);
  if (dns_servers != NULL)
  {
    for (i = 0; i < num_servers; i++)
    {
      status = config_nameserver(&servers, &nservers, dns_servers[i]);
      if (status != ARES_SUCCESS)
        break;
      status = ARES_EOF;
    }
    for (i = 0; i < num_servers; i++)
    {
      ares_free(dns_servers[i]);
    }
    ares_free(dns_servers);
  }

#  ifdef HAVE___SYSTEM_PROPERTY_GET
  /* Old way using the system property still in place as
   * a fallback. Older android versions can still use this.
   * it's possible for older apps not not have added the new
   * permission and we want to try to avoid breaking those.
   *
   * We'll only run this if we don't have any dns servers
   * because this will get the same ones (if it works). */
  if (status != ARES_EOF) {
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
  }
#  endif /* HAVE___SYSTEM_PROPERTY_GET */
#elif defined(CARES_USE_LIBRESOLV)
  struct __res_state res;
  memset(&res, 0, sizeof(res));
  int result = res_ninit(&res);
        continue;
      }
      else if(res) {
        /* Lets not treat a gethostname failure as critical, since we
         * are ok if gethostname doesn't even exist */
        *hostname = '\0';
        break;
      }

    } while (res != 0);
