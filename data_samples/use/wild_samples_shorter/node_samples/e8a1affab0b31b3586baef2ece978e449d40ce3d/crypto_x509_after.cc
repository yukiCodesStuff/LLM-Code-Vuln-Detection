  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  // TODO(tniessen): consider checking X509_get_pubkey() when the
  // X509Certificate object is being created.
  ClearErrorOnReturn clear_error_on_return;
  EVPKeyPointer pkey(X509_get_pubkey(cert->get()));
  if (!pkey) return ThrowCryptoError(env, ERR_get_error());
  ManagedEVPPKey epkey(std::move(pkey));
  std::shared_ptr<KeyObjectData> key_data =
      KeyObjectData::CreateAsymmetric(kKeyTypePublic, epkey);
