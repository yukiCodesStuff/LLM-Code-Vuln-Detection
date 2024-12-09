function translatePeerCertificate(c) {
  if (!c)
    return null;

  if (c.issuer != null) c.issuer = parseCertString(c.issuer);
  if (c.issuerCertificate != null && c.issuerCertificate !== c) {
    c.issuerCertificate = translatePeerCertificate(c.issuerCertificate);
  }