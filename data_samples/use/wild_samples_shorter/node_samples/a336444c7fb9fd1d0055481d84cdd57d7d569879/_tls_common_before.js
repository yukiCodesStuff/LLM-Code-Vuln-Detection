  if (!c)
    return null;

  if (c.issuer != null) c.issuer = parseCertString(c.issuer);
  if (c.issuerCertificate != null && c.issuerCertificate !== c) {
    c.issuerCertificate = translatePeerCertificate(c.issuerCertificate);
  }
  if (c.subject != null) c.subject = parseCertString(c.subject);
  if (c.infoAccess != null) {
    const info = c.infoAccess;
    c.infoAccess = ObjectCreate(null);
