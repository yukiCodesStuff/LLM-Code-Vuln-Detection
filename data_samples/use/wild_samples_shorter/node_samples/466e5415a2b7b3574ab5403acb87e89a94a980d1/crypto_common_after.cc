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

static inline void PrintAltName(const BIOPointer& out, const char* name,
                                size_t length, bool utf8,
                                const char* safe_prefix) {
  if (IsSafeAltName(name, length, utf8)) {
    // For backward-compatibility, append "safe" names without any
    // modifications.
    if (safe_prefix != nullptr) {
      BIO_printf(out.get(), "%s:", safe_prefix);
    }
    BIO_write(out.get(), name, length);
  } else {
    // If a name is not "safe", we cannot embed it without special
    // encoding. This does not usually happen, but we don't want to hide
    // it from the user either. We use JSON compatible escaping here.
    BIO_write(out.get(), "\"", 1);
    if (safe_prefix != nullptr) {
      BIO_printf(out.get(), "%s:", safe_prefix);
    }
    for (size_t j = 0; j < length; j++) {
      char c = static_cast<char>(name[j]);
      if (c == '\\') {
        BIO_write(out.get(), "\\\\", 2);
      } else if (c == '"') {
        BIO_write(out.get(), "\\\"", 2);
      } else if ((c >= ' ' && c != ',' && c <= '~') || (utf8 && (c & 0x80))) {
        // Note that the above condition explicitly excludes commas, which means
        // that those are encoded as Unicode escape sequences in the "else"
        // block. That is not strictly necessary, and Node.js itself would parse
        // it correctly either way. We only do this to account for third-party
        // code that might be splitting the string at commas (as Node.js itself
        // used to do).
        BIO_write(out.get(), &c, 1);
      } else {
        // Control character or non-ASCII character. We treat everything as
        // Latin-1, which corresponds to the first 255 Unicode code points.
        const char hex[] = "0123456789abcdef";
        char u[] = { '\\', 'u', '0', '0', hex[(c & 0xf0) >> 4], hex[c & 0x0f] };
        BIO_write(out.get(), u, sizeof(u));
      }
    }
    BIO_write(out.get(), "\"", 1);
  }
}

static inline void PrintLatin1AltName(const BIOPointer& out,
                                      const ASN1_IA5STRING* name,
                                      const char* safe_prefix = nullptr) {
  PrintAltName(out, reinterpret_cast<const char*>(name->data), name->length,
               false, safe_prefix);
}

static inline void PrintUtf8AltName(const BIOPointer& out,
                                    const ASN1_UTF8STRING* name,
                                    const char* safe_prefix = nullptr) {
  PrintAltName(out, reinterpret_cast<const char*>(name->data), name->length,
               true, safe_prefix);
}

// This function currently emulates the behavior of i2v_GENERAL_NAME in a safer
// and less ambiguous way.
// TODO(tniessen): gradually improve the format in the next major version(s)
static bool PrintGeneralName(const BIOPointer& out, const GENERAL_NAME* gen) {
  if (gen->type == GEN_DNS) {
    ASN1_IA5STRING* name = gen->d.dNSName;
    BIO_write(out.get(), "DNS:", 4);
    // Note that the preferred name syntax (see RFCs 5280 and 1034) with
    // wildcards is a subset of what we consider "safe", so spec-compliant DNS
    // names will never need to be escaped.
    PrintLatin1AltName(out, name);
  } else if (gen->type == GEN_EMAIL) {
    ASN1_IA5STRING* name = gen->d.rfc822Name;
    BIO_write(out.get(), "email:", 6);
    PrintLatin1AltName(out, name);
  } else if (gen->type == GEN_URI) {
    ASN1_IA5STRING* name = gen->d.uniformResourceIdentifier;
    BIO_write(out.get(), "URI:", 4);
    // The set of "safe" names was designed to include just about any URI,
    // with a few exceptions, most notably URIs that contains commas (see
    // RFC 2396). In other words, most legitimate URIs will not require
    // escaping.
    PrintLatin1AltName(out, name);
  } else if (gen->type == GEN_DIRNAME) {
    // For backward compatibility, use X509_NAME_oneline to print the
    // X509_NAME object. The format is non standard and should be avoided
    // elsewhere, but conveniently, the function produces ASCII and the output
    // is unlikely to contains commas or other characters that would require
    // escaping. With that in mind, note that it SHOULD NOT produce ASCII
    // output since an RFC5280 AttributeValue may be a UTF8String.
    // TODO(tniessen): switch to RFC2253 rules in a major release
    BIO_printf(out.get(), "DirName:");
    char oline[256];
    if (X509_NAME_oneline(gen->d.dirn, oline, sizeof(oline)) != nullptr) {
      PrintAltName(out, oline, strlen(oline), false, nullptr);
    } else {
      return false;
    }
  } else if (gen->type == GEN_IPADD) {
    BIO_printf(out.get(), "IP Address:");
    const ASN1_OCTET_STRING* ip = gen->d.ip;
    const unsigned char* b = ip->data;
    if (ip->length == 4) {
      BIO_printf(out.get(), "%d.%d.%d.%d", b[0], b[1], b[2], b[3]);
    } else if (ip->length == 16) {
      for (unsigned int j = 0; j < 8; j++) {
        uint16_t pair = (b[2 * j] << 8) | b[2 * j + 1];
        BIO_printf(out.get(), (j == 0) ? "%X" : ":%X", pair);
      }
    } else {
#if OPENSSL_VERSION_MAJOR >= 3
      BIO_printf(out.get(), "<invalid length=%d>", ip->length);
#else
      BIO_printf(out.get(), "<invalid>");
#endif
    }
  } else if (gen->type == GEN_RID) {
    // TODO(tniessen): unlike OpenSSL's default implementation, never print the
    // OID as text and instead always print its numeric representation, which is
    // backward compatible in practice and more future proof (see OBJ_obj2txt).
    char oline[256];
    i2t_ASN1_OBJECT(oline, sizeof(oline), gen->d.rid);
    BIO_printf(out.get(), "Registered ID:%s", oline);
  } else if (gen->type == GEN_OTHERNAME) {
    // TODO(tniessen): the format that is used here is based on OpenSSL's
    // implementation of i2v_GENERAL_NAME (as of OpenSSL 3.0.1), mostly for
    // backward compatibility. It is somewhat awkward, especially when passed to
    // translatePeerCertificate, and should be changed in the future, probably
    // to the format used by GENERAL_NAME_print (in a major release).
    bool unicode = true;
    const char* prefix = nullptr;
    // OpenSSL 1.1.1 does not support othername in i2v_GENERAL_NAME and may not
    // define these NIDs.
#if OPENSSL_VERSION_MAJOR >= 3
    int nid = OBJ_obj2nid(gen->d.otherName->type_id);
    switch (nid) {
      case NID_id_on_SmtpUTF8Mailbox:
        prefix = " SmtpUTF8Mailbox:";
        break;
      case NID_XmppAddr:
        prefix = " XmppAddr:";
        break;
      case NID_SRVName:
        prefix = " SRVName:";
        unicode = false;
        break;
      case NID_ms_upn:
        prefix = " UPN:";
        break;
      case NID_NAIRealm:
        prefix = " NAIRealm:";
        break;
    }
#endif  // OPENSSL_VERSION_MAJOR >= 3
    int val_type = gen->d.otherName->value->type;
    if (prefix == nullptr ||
        (unicode && val_type != V_ASN1_UTF8STRING) ||
        (!unicode && val_type != V_ASN1_IA5STRING)) {
      BIO_printf(out.get(), "othername:<unsupported>");
    } else {
      BIO_printf(out.get(), "othername:");
      if (unicode) {
        PrintUtf8AltName(out, gen->d.otherName->value->value.utf8string,
                         prefix);
      } else {
        PrintLatin1AltName(out, gen->d.otherName->value->value.ia5string,
                           prefix);
      }
    }
  } else if (gen->type == GEN_X400) {
    // TODO(tniessen): this is what OpenSSL does, implement properly instead
    BIO_printf(out.get(), "X400Name:<unsupported>");
  } else if (gen->type == GEN_EDIPARTY) {
    // TODO(tniessen): this is what OpenSSL does, implement properly instead
    BIO_printf(out.get(), "EdiPartyName:<unsupported>");
  } else {
    // This is safe because X509V3_EXT_d2i would have returned nullptr in this
    // case already.
    UNREACHABLE();
  }

  return true;
}

bool SafeX509SubjectAltNamePrint(const BIOPointer& out, X509_EXTENSION* ext) {
  const X509V3_EXT_METHOD* method = X509V3_EXT_get(ext);
  CHECK(method == X509V3_EXT_get_nid(NID_subject_alt_name));

  GENERAL_NAMES* names = static_cast<GENERAL_NAMES*>(X509V3_EXT_d2i(ext));
  if (names == nullptr)
    return false;

  bool ok = true;

  for (int i = 0; i < sk_GENERAL_NAME_num(names); i++) {
    GENERAL_NAME* gen = sk_GENERAL_NAME_value(names, i);

    if (i != 0)
      BIO_write(out.get(), ", ", 2);

    if (!(ok = PrintGeneralName(out, gen))) {
      break;
    }
  }
  sk_GENERAL_NAME_pop_free(names, GENERAL_NAME_free);

  return ok;
}

bool SafeX509InfoAccessPrint(const BIOPointer& out, X509_EXTENSION* ext) {
  const X509V3_EXT_METHOD* method = X509V3_EXT_get(ext);
  CHECK(method == X509V3_EXT_get_nid(NID_info_access));

  AUTHORITY_INFO_ACCESS* descs =
      static_cast<AUTHORITY_INFO_ACCESS*>(X509V3_EXT_d2i(ext));
  if (descs == nullptr)
    return false;

  bool ok = true;

  for (int i = 0; i < sk_ACCESS_DESCRIPTION_num(descs); i++) {
    ACCESS_DESCRIPTION* desc = sk_ACCESS_DESCRIPTION_value(descs, i);

    if (i != 0)
      BIO_write(out.get(), "\n", 1);

    char objtmp[80];
    i2t_ASN1_OBJECT(objtmp, sizeof(objtmp), desc->method);
    BIO_printf(out.get(), "%s - ", objtmp);
    if (!(ok = PrintGeneralName(out, desc->location))) {
      break;
    }
  }
  sk_ACCESS_DESCRIPTION_pop_free(descs, ACCESS_DESCRIPTION_free);

#if OPENSSL_VERSION_MAJOR < 3
  BIO_write(out.get(), "\n", 1);
#endif

  return ok;
}

v8::MaybeLocal<v8::Value> GetSubjectAltNameString(
    Environment* env,
    const BIOPointer& bio,
    X509* cert) {
  int index = X509_get_ext_by_NID(cert, NID_subject_alt_name, -1);
  if (index < 0)
    return Undefined(env->isolate());

  X509_EXTENSION* ext = X509_get_ext(cert, index);
  CHECK_NOT_NULL(ext);

  if (!SafeX509SubjectAltNamePrint(bio, ext)) {
    USE(BIO_reset(bio.get()));
    return v8::Null(env->isolate());
  }

  return ToV8Value(env, bio);
}

v8::MaybeLocal<v8::Value> GetInfoAccessString(
    Environment* env,
    const BIOPointer& bio,
    X509* cert) {
  int index = X509_get_ext_by_NID(cert, NID_info_access, -1);
  if (index < 0)
    return Undefined(env->isolate());

  X509_EXTENSION* ext = X509_get_ext(cert, index);
  CHECK_NOT_NULL(ext);

  if (!SafeX509InfoAccessPrint(bio, ext)) {
    USE(BIO_reset(bio.get()));
    return v8::Null(env->isolate());
  }

  return ToV8Value(env, bio);
}

MaybeLocal<Value> GetIssuerString(
    Environment* env,
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

  EVPKeyPointer pkey(X509_get_pubkey(cert));