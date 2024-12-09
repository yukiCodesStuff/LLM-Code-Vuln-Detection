
v8::MaybeLocal<v8::Object> X509ToObject(
    Environment* env,
    X509* cert);

v8::MaybeLocal<v8::Value> GetValidTo(
    Environment* env,
    X509* cert,