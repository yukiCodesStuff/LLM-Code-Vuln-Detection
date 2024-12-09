  X509Certificate* cert;
  ASSIGN_OR_RETURN_UNWRAP(&cert, args.Holder());

  EVPKeyPointer pkey(X509_get_pubkey(cert->get()));
  ManagedEVPPKey epkey(std::move(pkey));
  std::shared_ptr<KeyObjectData> key_data =
      KeyObjectData::CreateAsymmetric(kKeyTypePublic, epkey);
