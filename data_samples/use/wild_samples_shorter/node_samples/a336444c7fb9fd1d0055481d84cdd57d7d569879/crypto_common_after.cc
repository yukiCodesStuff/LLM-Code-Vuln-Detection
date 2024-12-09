
namespace crypto {
static constexpr int X509_NAME_FLAGS =
    ASN1_STRFLGS_ESC_2253 |
    ASN1_STRFLGS_ESC_CTRL |
    ASN1_STRFLGS_UTF8_CONVERT |
    XN_FLAG_SEP_MULTILINE |
    XN_FLAG_FN_SN;
  return ToV8Value(env, bio);
}

template <X509_NAME* get_name(const X509*)>
static MaybeLocal<Value> GetX509NameObject(Environment* env, X509* cert) {
  X509_NAME* name = get_name(cert);
  CHECK_NOT_NULL(name);

  int cnt = X509_NAME_entry_count(name);
  CHECK_GE(cnt, 0);

  Local<Object> result =
      Object::New(env->isolate(), Null(env->isolate()), nullptr, nullptr, 0);
  if (result.IsEmpty()) {
    return MaybeLocal<Value>();
  }

  for (int i = 0; i < cnt; i++) {
    X509_NAME_ENTRY* entry = X509_NAME_get_entry(name, i);
    CHECK_NOT_NULL(entry);

    // We intentionally ignore the value of X509_NAME_ENTRY_set because the
    // representation as an object does not allow grouping entries into sets
    // anyway, and multi-value RDNs are rare, i.e., the vast majority of
    // Relative Distinguished Names contains a single type-value pair only.
    const ASN1_OBJECT* type = X509_NAME_ENTRY_get_object(entry);
    const ASN1_STRING* value = X509_NAME_ENTRY_get_data(entry);

    // If OpenSSL knows the type, use the short name of the type as the key, and
    // the numeric representation of the type's OID otherwise.
    int type_nid = OBJ_obj2nid(type);
    char type_buf[80];
    const char* type_str;
    if (type_nid != NID_undef) {
      type_str = OBJ_nid2sn(type_nid);
      CHECK_NOT_NULL(type_str);
    } else {
      OBJ_obj2txt(type_buf, sizeof(type_buf), type, true);
      type_str = type_buf;
    }

    Local<String> v8_name;
    if (!String::NewFromUtf8(env->isolate(), type_str).ToLocal(&v8_name)) {
      return MaybeLocal<Value>();
    }

    // The previous implementation used X509_NAME_print_ex, which escapes some
    // characters in the value. The old implementation did not decode/unescape
    // values correctly though, leading to ambiguous and incorrect
    // representations. The new implementation only converts to Unicode and does
    // not escape anything.
    unsigned char* value_str;
    int value_str_size = ASN1_STRING_to_UTF8(&value_str, value);
    if (value_str_size < 0) {
      return Undefined(env->isolate());
    }

    Local<String> v8_value;
    if (!String::NewFromUtf8(env->isolate(),
                             reinterpret_cast<const char*>(value_str),
                             NewStringType::kNormal,
                             value_str_size).ToLocal(&v8_value)) {
      OPENSSL_free(value_str);
      return MaybeLocal<Value>();
    }

    OPENSSL_free(value_str);

    // For backward compatibility, we only create arrays if multiple values
    // exist for the same key. That is not great but there is not much we can
    // change here without breaking things. Note that this creates nested data
    // structures, yet still does not allow representing Distinguished Names
    // accurately.
    if (result->HasOwnProperty(env->context(), v8_name).ToChecked()) {
      Local<Value> accum =
          result->Get(env->context(), v8_name).ToLocalChecked();
      if (!accum->IsArray()) {
        accum = Array::New(env->isolate(), &accum, 1);
        result->Set(env->context(), v8_name, accum).Check();
      }
      Local<Array> array = accum.As<Array>();
      array->Set(env->context(), array->Length(), v8_value).Check();
    } else {
      result->Set(env->context(), v8_name, v8_value).Check();
    }
  }

  return result;
}

MaybeLocal<Value> GetCipherName(Environment* env, const SSLPointer& ssl) {
  return GetCipherName(env, SSL_get_current_cipher(ssl.get()));
}

  return result;
}

MaybeLocal<Object> X509ToObject(
    Environment* env,
    X509* cert,
    bool names_as_string) {
  EscapableHandleScope scope(env->isolate());
  Local<Context> context = env->context();
  Local<Object> info = Object::New(env->isolate());

  BIOPointer bio(BIO_new(BIO_s_mem()));

  if (names_as_string) {
    // TODO(tniessen): this branch should not have to exist. It is only here
    // because toLegacyObject() does not actually return a legacy object, and
    // instead represents subject and issuer as strings.
    if (!Set<Value>(context,
                    info,
                    env->subject_string(),
                    GetSubject(env, bio, cert)) ||
        !Set<Value>(context,
                    info,
                    env->issuer_string(),
                    GetIssuerString(env, bio, cert))) {
      return MaybeLocal<Object>();
    }
  } else {
    if (!Set<Value>(context,
                    info,
                    env->subject_string(),
                    GetX509NameObject<X509_get_subject_name>(env, cert)) ||
        !Set<Value>(context,
                    info,
                    env->issuer_string(),
                    GetX509NameObject<X509_get_issuer_name>(env, cert))) {
      return MaybeLocal<Object>();
    }
  }

  if (!Set<Value>(context,
                  info,
                  env->subjectaltname_string(),
                  GetSubjectAltNameString(env, bio, cert)) ||
      !Set<Value>(context,