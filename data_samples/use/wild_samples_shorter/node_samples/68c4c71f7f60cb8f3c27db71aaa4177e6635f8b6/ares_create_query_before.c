 */

int ares_create_query(const char *name, int dnsclass, int type,
                      unsigned short id, int rd, unsigned char **buf,
                      int *buflen, int max_udp_size)
{
  int len;
  unsigned char *q;
  const char *p;

  /* Set our results early, in case we bail out early with an error. */
  *buflen = 0;
  *buf = NULL;

  /* Compute the length of the encoded name so we can check buflen.
   * Start counting at 1 for the zero-length label at the end. */
  len = 1;
  for (p = name; *p; p++)
    {
      if (*p == '\\' && *(p + 1) != 0)
        p++;
      len++;
    }
  /* If there are n periods in the name, there are n + 1 labels, and
   * thus n + 1 length fields, unless the name is empty or ends with a
   * period.  So add 1 unless name is empty or ends with a period.
   */
  if (*name && *(p - 1) != '.')
    len++;

  /* Immediately reject names that are longer than the maximum of 255
   * bytes that's specified in RFC 1035 ("To simplify implementations,
   * the total length of a domain name (i.e., label octets and label
   * length octets) is restricted to 255 octets or less."). We aren't
   * doing this just to be a stickler about RFCs. For names that are
   * too long, 'dnscache' closes its TCP connection to us immediately
   * (when using TCP) and ignores the request when using UDP, and
   * BIND's named returns ServFail (TCP or UDP). Sending a request
   * that we know will cause 'dnscache' to close the TCP connection is
   * painful, since that makes any other outstanding requests on that
   * connection fail. And sending a UDP request that we know
   * 'dnscache' will ignore is bad because resources will be tied up
   * until we time-out the request.
   */
  if (len > MAXCDNAME)
    return ARES_EBADNAME;

  *buflen = len + HFIXEDSZ + QFIXEDSZ + (max_udp_size ? EDNSFIXEDSZ : 0);
  *buf = ares_malloc(*buflen);
  if (!*buf)
      return ARES_ENOMEM;

  /* Set up the header. */
  q = *buf;
  memset(q, 0, HFIXEDSZ);
  DNS_HEADER_SET_QID(q, id);
  DNS_HEADER_SET_OPCODE(q, QUERY);
  if (rd) {
  q += HFIXEDSZ;
  while (*name)
    {
      if (*name == '.')
        return ARES_EBADNAME;

      /* Count the number of bytes in this label. */
      len = 0;
      for (p = name; *p && *p != '.'; p++)
            p++;
          len++;
        }
      if (len > MAXLABEL)
        return ARES_EBADNAME;

      /* Encode the length and copy the data. */
      *q++ = (unsigned char)len;
      for (p = name; *p && *p != '.'; p++)
  DNS_QUESTION_SET_TYPE(q, type);
  DNS_QUESTION_SET_CLASS(q, dnsclass);

  if (max_udp_size)
  {
      q += QFIXEDSZ;
      memset(q, 0, EDNSFIXEDSZ);
      q++;
      DNS_RR_SET_TYPE(q, T_OPT);
      DNS_RR_SET_CLASS(q, max_udp_size);
  }

  return ARES_SUCCESS;
}