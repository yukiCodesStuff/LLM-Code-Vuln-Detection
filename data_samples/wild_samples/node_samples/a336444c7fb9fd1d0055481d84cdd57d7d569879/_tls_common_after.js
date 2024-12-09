function translatePeerCertificate(c) {
  if (!c)
    return null;

  // TODO(tniessen): can we remove parseCertString without breaking anything?
  if (typeof c.issuer === 'string') c.issuer = parseCertString(c.issuer);
  if (c.issuerCertificate != null && c.issuerCertificate !== c) {
    c.issuerCertificate = translatePeerCertificate(c.issuerCertificate);
  }