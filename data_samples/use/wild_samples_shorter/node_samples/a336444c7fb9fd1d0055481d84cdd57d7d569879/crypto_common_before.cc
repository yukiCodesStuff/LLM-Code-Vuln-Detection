
namespace crypto {
static constexpr int X509_NAME_FLAGS =
    ASN1_STRFLGS_ESC_CTRL |
    ASN1_STRFLGS_UTF8_CONVERT |
    XN_FLAG_SEP_MULTILINE |
    XN_FLAG_FN_SN;
  return ToV8Value(env, bio);
}

MaybeLocal<Value> GetCipherName(Environment* env, const SSLPointer& ssl) {
  return GetCipherName(env, SSL_get_current_cipher(ssl.get()));
}

  return result;
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