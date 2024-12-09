  if (!c)
    return null;

  // TODO(tniessen): can we remove parseCertString without breaking anything?
  if (typeof c.issuer === 'string') c.issuer = parseCertString(c.issuer);
  if (c.issuerCertificate != null && c.issuerCertificate !== c) {
    c.issuerCertificate = translatePeerCertificate(c.issuerCertificate);
  }
  // TODO(tniessen): can we remove parseCertString without breaking anything?
  if (typeof c.subject === 'string') c.subject = parseCertString(c.subject);
  if (c.infoAccess != null) {
    const info = c.infoAccess;
    c.infoAccess = ObjectCreate(null);
