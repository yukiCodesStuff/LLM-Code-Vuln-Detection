
using HashJob = DeriveBitsJob<HashTraits>;

void InternalVerifyIntegrity(const v8::FunctionCallbackInfo<v8::Value>& args);

}  // namespace crypto
}  // namespace node

#endif  // defined(NODE_WANT_INTERNALS) && NODE_WANT_INTERNALS