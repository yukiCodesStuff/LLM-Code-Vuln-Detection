    const BIOPointer& bio) {
  ASN1_TIME_print(bio.get(), X509_get0_notBefore(cert));
  return ToV8Value(env, bio);
}

static inline bool IsSafeAltName(const char* name, size_t length, bool utf8) {
  for (size_t i = 0; i < length; i++) {
    char c = name[i];
    switch (c) {
    case '"':
    case '\\':
      // These mess with encoding rules.
      // Fall through.
    case ',':
      // Commas make it impossible to split the list of subject alternative
      // names unambiguously, which is why we have to escape.
      // Fall through.
    case '\'':
      // Single quotes are unlikely to appear in any legitimate values, but they
      // could be used to make a value look like it was escaped (i.e., enclosed
      // in single/double quotes).
      return false;
    default:
      if (utf8) {
        // In UTF8 strings, we require escaping for any ASCII control character,
        // but NOT for non-ASCII characters. Note that all bytes of any code
        // point that consists of more than a single byte have their MSB set.
        if (static_cast<unsigned char>(c) < ' ' || c == '\x7f') {
          return false;
        }
      } else {
        // Check if the char is a control character or non-ASCII character. Note
        // that char may or may not be a signed type. Regardless, non-ASCII
        // values will always be outside of this range.
        if (c < ' ' || c > '~') {
          return false;
        }
      }
    }
  }
  return true;
}
MaybeLocal<Object> X509ToObject(Environment* env, X509* cert) {
  EscapableHandleScope scope(env->isolate());
  Local<Context> context = env->context();
  Local<Object> info = Object::New(env->isolate());

  BIOPointer bio(BIO_new(BIO_s_mem()));

  if (!Set<Value>(context,
                  info,
                  env->subject_string(),
                  GetSubject(env, bio, cert)) ||
      !Set<Value>(context,
                  info,
                  env->issuer_string(),
                  GetIssuerString(env, bio, cert)) ||
      !Set<Value>(context,
                  info,
                  env->subjectaltname_string(),
                  GetSubjectAltNameString(env, bio, cert)) ||
      !Set<Value>(context,
                  info,
                  env->infoaccess_string(),
                  GetInfoAccessString(env, bio, cert))) {
    return MaybeLocal<Object>();
  }