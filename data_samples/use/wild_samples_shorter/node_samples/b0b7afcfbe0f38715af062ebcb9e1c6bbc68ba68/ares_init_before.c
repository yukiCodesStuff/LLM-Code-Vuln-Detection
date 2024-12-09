
#if defined(ANDROID) || defined(__ANDROID__)
#include <sys/system_properties.h>
/* From the Bionic sources */
#define DNS_PROP_NAME_PREFIX  "net.dns"
#define MAX_DNS_PROPERTIES    8
#endif
  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, NULL,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
  *outptr = NULL;

  /* Find out size of string stored in registry */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType, NULL, &size);
  if ((res != ERROR_SUCCESS && res != ERROR_MORE_DATA) || !size)
    return 0;

  /* Allocate buffer of indicated size plus one given that string
    return 0;

  /* Get the value for real */
  res = RegQueryValueEx(hKey, leafKeyName, 0, &dataType,
                        (unsigned char *)*outptr, &size);
  if ((res != ERROR_SUCCESS) || (size == 1))
  {
    ares_free(*outptr);
  for(;;)
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

  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_9X, 0, KEY_READ,
                     &hKey_VxD_MStcp);
  if (res != ERROR_SUCCESS)
    return 0;


  *outptr = NULL;

  res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0, KEY_READ,
                     &hKey_Tcpip_Parameters);
  if (res != ERROR_SUCCESS)
    return 0;

    goto done;

  /* Try adapter specific parameters */
  res = RegOpenKeyEx(hKey_Tcpip_Parameters, "Interfaces", 0,
                     KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS,
                     &hKey_Interfaces);
  if (res != ERROR_SUCCESS)
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
  /* The metric we sort them by. */
  ULONG metric;

  /* Room enough for the string form of any IPv4 or IPv6 address that
   * ares_inet_ntop() will create.  Based on the existing c-ares practice.
   */
  char text[sizeof("ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255")];
{
  const Address * const left = arg1;
  const Address * const right = arg2;
  if(left->metric < right->metric) return -1;
  if(left->metric > right->metric) return 1;
  return 0;
}

/* There can be multiple routes to "the Internet".  And there can be different
          addresses[addressesIndex].metric = -1;
        }

        if (! ares_inet_ntop(AF_INET, &namesrvr.sa4->sin_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
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
          addresses[addressesIndex].metric = -1;
        }

        if (! ares_inet_ntop(AF_INET6, &namesrvr.sa6->sin6_addr,
                             addresses[addressesIndex].text,
                             sizeof(addresses[0].text))) {
          continue;
    }
  }

  /* Sort all of the textual addresses by their metric. */
  qsort(addresses, addressesIndex, sizeof(*addresses), compareAddresses);

  /* Join them all into a single string, removing duplicates. */
  {
  DWORD keyNameBuffSize;
  DWORD keyIdx = 0;
  char *p = NULL;
  char *pp;
  size_t len = 0;

  *outptr = NULL;

    return 0;

  /* 1. Global DNS Suffix Search List */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_NS_NT_KEY, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    if (get_REG_SZ(hKey, SEARCHLIST_KEY, outptr))
      replace_comma_by_space(*outptr);

  /* 2. Connection Specific Search List composed of:
   *  a. Primary DNS Suffix */
  if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, WIN_DNSCLIENT, 0,
      KEY_READ, &hKey) == ERROR_SUCCESS)
  {
    get_REG_SZ(hKey, PRIMARYDNSSUFFIX_KEY, outptr);
    RegCloseKey(hKey);
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
    {
      /* p can be comma separated (SearchList) */
      pp = p;
      while (len = next_suffix(&pp, len))
      {
        if (!contains_suffix(*outptr, pp, len))
          commanjoin(outptr, pp, len);
      }
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
        continue;
      }
      else if(res) {
        rc = ARES_EBADNAME;
        goto error;
      }

    } while (res != 0);
