struct HashTraits final {
  using AdditionalParameters = HashConfig;
  static constexpr const char* JobName = "HashJob";
  static constexpr AsyncWrap::ProviderType Provider =
      AsyncWrap::PROVIDER_HASHREQUEST;

  static v8::Maybe<bool> AdditionalConfig(
      CryptoJobMode mode,
      const v8::FunctionCallbackInfo<v8::Value>& args,
      unsigned int offset,
      HashConfig* params);

  static bool DeriveBits(
      Environment* env,
      const HashConfig& params,
      ByteSource* out);

  static v8::Maybe<bool> EncodeOutput(
      Environment* env,
      const HashConfig& params,
      ByteSource* out,
      v8::Local<v8::Value>* result);
};

using HashJob = DeriveBitsJob<HashTraits>;

}  // namespace crypto
}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS
#endif  // SRC_CRYPTO_CRYPTO_HASH_H_