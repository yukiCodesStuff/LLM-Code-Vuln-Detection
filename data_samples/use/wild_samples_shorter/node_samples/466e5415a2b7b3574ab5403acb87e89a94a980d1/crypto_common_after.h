v8::MaybeLocal<v8::Object> GetRawDERCertificate(Environment* env, X509* cert);

v8::Local<v8::Value> ToV8Value(Environment* env, const BIOPointer& bio);
bool SafeX509SubjectAltNamePrint(const BIOPointer& out, X509_EXTENSION* ext);

v8::MaybeLocal<v8::Value> GetSubject(
    Environment* env,
    const BIOPointer& bio,
    const BIOPointer& bio,
    X509* cert);

v8::MaybeLocal<v8::Value> GetSubjectAltNameString(
    Environment* env,
    const BIOPointer& bio,
    X509* cert);

v8::MaybeLocal<v8::Value> GetInfoAccessString(
    Environment* env,
    const BIOPointer& bio,
    X509* cert);

}  // namespace crypto
}  // namespace node
