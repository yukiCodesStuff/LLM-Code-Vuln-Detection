namespace crypto {
static constexpr int X509_NAME_FLAGS =
    ASN1_STRFLGS_ESC_CTRL |
    ASN1_STRFLGS_UTF8_CONVERT |
    XN_FLAG_SEP_MULTILINE |
    XN_FLAG_FN_SN;

int SSL_CTX_get_issuer(SSL_CTX* ctx, X509* cert, X509** issuer) {
  X509_STORE* store = SSL_CTX_get_cert_store(ctx);
  DeleteFnPtr<X509_STORE_CTX, X509_STORE_CTX_free> store_ctx(
      X509_STORE_CTX_new());
  return store_ctx.get() != nullptr &&
         X509_STORE_CTX_init(store_ctx.get(), store, nullptr, nullptr) == 1 &&
         X509_STORE_CTX_get1_issuer(issuer, store_ctx.get(), cert) == 1;
}

void LogSecret(
    const SSLPointer& ssl,
    const char* name,
    const unsigned char* secret,
    size_t secretlen) {
  auto keylog_cb = SSL_CTX_get_keylog_callback(SSL_get_SSL_CTX(ssl.get()));
  unsigned char crandom[32];

  if (keylog_cb == nullptr ||
      SSL_get_client_random(ssl.get(), crandom, 32) != 32) {
    return;
  }

  std::string line = name;
  line += " " + StringBytes::hex_encode(
      reinterpret_cast<const char*>(crandom), 32);
  line += " " + StringBytes::hex_encode(
      reinterpret_cast<const char*>(secret), secretlen);
  keylog_cb(ssl.get(), line.c_str());
}

bool SetALPN(const SSLPointer& ssl, const std::string& alpn) {
  return SSL_set_alpn_protos(
      ssl.get(),
      reinterpret_cast<const uint8_t*>(alpn.c_str()),
      alpn.length()) == 0;
}

bool SetALPN(const SSLPointer& ssl, Local<Value> alpn) {
  if (!alpn->IsArrayBufferView())
    return false;
  ArrayBufferViewContents<unsigned char> protos(alpn.As<ArrayBufferView>());
  return SSL_set_alpn_protos(ssl.get(), protos.data(), protos.length()) == 0;
}

MaybeLocal<Value> GetSSLOCSPResponse(
    Environment* env,
    SSL* ssl,
    Local<Value> default_value) {
  const unsigned char* resp;
  int len = SSL_get_tlsext_status_ocsp_resp(ssl, &resp);
  if (resp == nullptr)
    return default_value;

  Local<Value> ret;
  MaybeLocal<Object> maybe_buffer =
      Buffer::Copy(env, reinterpret_cast<const char*>(resp), len);

  if (!maybe_buffer.ToLocal(&ret))
    return MaybeLocal<Value>();

  return ret;
}

bool SetTLSSession(
    const SSLPointer& ssl,
    const unsigned char* buf,
    size_t length) {
  SSLSessionPointer s(d2i_SSL_SESSION(nullptr, &buf, length));
  return s == nullptr ? false : SetTLSSession(ssl, s);
}

bool SetTLSSession(
    const SSLPointer& ssl,
    const SSLSessionPointer& session) {
  return session != nullptr && SSL_set_session(ssl.get(), session.get()) == 1;
}

SSLSessionPointer GetTLSSession(Local<Value> val) {
  if (!val->IsArrayBufferView())
    return SSLSessionPointer();
  ArrayBufferViewContents<unsigned char> sbuf(val.As<ArrayBufferView>());
  return GetTLSSession(sbuf.data(), sbuf.length());
}

SSLSessionPointer GetTLSSession(const unsigned char* buf, size_t length) {
  return SSLSessionPointer(d2i_SSL_SESSION(nullptr, &buf, length));
}

long VerifyPeerCertificate(  // NOLINT(runtime/int)
    const SSLPointer& ssl,
    long def) {  // NOLINT(runtime/int)
  long err = def;  // NOLINT(runtime/int)
  if (X509* peer_cert = SSL_get_peer_certificate(ssl.get())) {
    X509_free(peer_cert);
    err = SSL_get_verify_result(ssl.get());
  } else {
    const SSL_CIPHER* curr_cipher = SSL_get_current_cipher(ssl.get());
    const SSL_SESSION* sess = SSL_get_session(ssl.get());
    // Allow no-cert for PSK authentication in TLS1.2 and lower.
    // In TLS1.3 check that session was reused because TLS1.3 PSK
    // looks like session resumption.
    if (SSL_CIPHER_get_auth_nid(curr_cipher) == NID_auth_psk ||
        (SSL_SESSION_get_protocol_version(sess) == TLS1_3_VERSION &&
         SSL_session_reused(ssl.get()))) {
      return X509_V_OK;
    }
  }
  return err;
}

int UseSNIContext(const SSLPointer& ssl, BaseObjectPtr<SecureContext> context) {
  SSL_CTX* ctx = context->ctx_.get();
  X509* x509 = SSL_CTX_get0_certificate(ctx);
  EVP_PKEY* pkey = SSL_CTX_get0_privatekey(ctx);
  STACK_OF(X509)* chain;

  int err = SSL_CTX_get0_chain_certs(ctx, &chain);
  if (err == 1) err = SSL_use_certificate(ssl.get(), x509);
  if (err == 1) err = SSL_use_PrivateKey(ssl.get(), pkey);
  if (err == 1 && chain != nullptr) err = SSL_set1_chain(ssl.get(), chain);
  return err;
}

const char* GetClientHelloALPN(const SSLPointer& ssl) {
  const unsigned char* buf;
  size_t len;
  size_t rem;

  if (!SSL_client_hello_get0_ext(
          ssl.get(),
          TLSEXT_TYPE_application_layer_protocol_negotiation,
          &buf,
          &rem) ||
      rem < 2) {
    return nullptr;
  }

  len = (buf[0] << 8) | buf[1];
  if (len + 2 != rem) return nullptr;
  return reinterpret_cast<const char*>(buf + 3);
}

const char* GetClientHelloServerName(const SSLPointer& ssl) {
  const unsigned char* buf;
  size_t len;
  size_t rem;

  if (!SSL_client_hello_get0_ext(
          ssl.get(),
          TLSEXT_TYPE_server_name,
          &buf,
          &rem) || rem <= 2) {
    return nullptr;
  }

  len = (*buf << 8) | *(buf + 1);
  if (len + 2 != rem)
    return nullptr;
  rem = len;

  if (rem == 0 || *(buf + 2) != TLSEXT_NAMETYPE_host_name) return nullptr;
  rem--;
  if (rem <= 2)
    return nullptr;
  len = (*(buf + 3) << 8) | *(buf + 4);
  if (len + 2 > rem)
    return nullptr;
  return reinterpret_cast<const char*>(buf + 5);
}

const char* GetServerName(SSL* ssl) {
  return SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
}

bool SetGroups(SecureContext* sc, const char* groups) {
  return SSL_CTX_set1_groups_list(**sc, groups) == 1;
}

const char* X509ErrorCode(long err) {  // NOLINT(runtime/int)
  const char* code = "UNSPECIFIED";
#define CASE_X509_ERR(CODE) case X509_V_ERR_##CODE: code = #CODE; break;
  switch (err) {
    // if you modify anything in here, *please* update the respective section in
    // doc/api/tls.md as well
    CASE_X509_ERR(UNABLE_TO_GET_ISSUER_CERT)
    CASE_X509_ERR(UNABLE_TO_GET_CRL)
    CASE_X509_ERR(UNABLE_TO_DECRYPT_CERT_SIGNATURE)
    CASE_X509_ERR(UNABLE_TO_DECRYPT_CRL_SIGNATURE)
    CASE_X509_ERR(UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY)
    CASE_X509_ERR(CERT_SIGNATURE_FAILURE)
    CASE_X509_ERR(CRL_SIGNATURE_FAILURE)
    CASE_X509_ERR(CERT_NOT_YET_VALID)
    CASE_X509_ERR(CERT_HAS_EXPIRED)
    CASE_X509_ERR(CRL_NOT_YET_VALID)
    CASE_X509_ERR(CRL_HAS_EXPIRED)
    CASE_X509_ERR(ERROR_IN_CERT_NOT_BEFORE_FIELD)
    CASE_X509_ERR(ERROR_IN_CERT_NOT_AFTER_FIELD)
    CASE_X509_ERR(ERROR_IN_CRL_LAST_UPDATE_FIELD)
    CASE_X509_ERR(ERROR_IN_CRL_NEXT_UPDATE_FIELD)
    CASE_X509_ERR(OUT_OF_MEM)
    CASE_X509_ERR(DEPTH_ZERO_SELF_SIGNED_CERT)
    CASE_X509_ERR(SELF_SIGNED_CERT_IN_CHAIN)
    CASE_X509_ERR(UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
    CASE_X509_ERR(UNABLE_TO_VERIFY_LEAF_SIGNATURE)
    CASE_X509_ERR(CERT_CHAIN_TOO_LONG)
    CASE_X509_ERR(CERT_REVOKED)
    CASE_X509_ERR(INVALID_CA)
    CASE_X509_ERR(PATH_LENGTH_EXCEEDED)
    CASE_X509_ERR(INVALID_PURPOSE)
    CASE_X509_ERR(CERT_UNTRUSTED)
    CASE_X509_ERR(CERT_REJECTED)
    CASE_X509_ERR(HOSTNAME_MISMATCH)
  }
#undef CASE_X509_ERR
  return code;
}
          X509_NAME_FLAGS) <= 0) {
    USE(BIO_reset(bio.get()));
    return Undefined(env->isolate());
  }

  return ToV8Value(env, bio);
}

MaybeLocal<Value> GetCipherName(Environment* env, const SSLPointer& ssl) {
  return GetCipherName(env, SSL_get_current_cipher(ssl.get()));
}

MaybeLocal<Value> GetCipherStandardName(
    Environment* env,
    const SSLPointer& ssl) {
  return GetCipherStandardName(env, SSL_get_current_cipher(ssl.get()));
}

MaybeLocal<Value> GetCipherVersion(Environment* env, const SSLPointer& ssl) {
  return GetCipherVersion(env, SSL_get_current_cipher(ssl.get()));
}

MaybeLocal<Array> GetClientHelloCiphers(
    Environment* env,
    const SSLPointer& ssl) {
  EscapableHandleScope scope(env->isolate());
  const unsigned char* buf;
  size_t len = SSL_client_hello_get0_ciphers(ssl.get(), &buf);
  size_t count = len / 2;
  MaybeStackBuffer<Local<Value>, 16> ciphers(count);
  int j = 0;
  for (size_t n = 0; n < len; n += 2) {
    const SSL_CIPHER* cipher = SSL_CIPHER_find(ssl.get(), buf);
    buf += 2;
    Local<Object> obj = Object::New(env->isolate());
    if (!Set(env->context(),
             obj,
             env->name_string(),
             GetCipherName(env, cipher)) ||
        !Set(env->context(),
             obj,
             env->standard_name_string(),
             GetCipherStandardName(env, cipher)) ||
        !Set(env->context(),
             obj,
             env->version_string(),
             GetCipherVersion(env, cipher))) {
      return MaybeLocal<Array>();
    }
    ciphers[j++] = obj;
  }
           issuer_chain)) {
    return MaybeLocal<Value>();
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
                  info,
                  env->infoaccess_string(),
                  GetInfoAccessString(env, bio, cert))) {
    return MaybeLocal<Object>();
  }