using v8::Undefined;
using v8::Value;

const uint32_t kOnMessageBegin = 0;
const uint32_t kOnHeaders = 1;
const uint32_t kOnHeadersComplete = 2;
const uint32_t kOnBody = 3;
const uint32_t kOnMessageComplete = 4;
const uint32_t kOnExecute = 5;
const uint32_t kOnTimeout = 6;
// Any more fields than this will be flushed into JS
const size_t kMaxHeaderFieldsCount = 32;

inline bool IsOWS(char c) {
    url_.Reset();
    status_message_.Reset();
    header_parsing_start_time_ = uv_hrtime();

    Local<Value> cb = object()->Get(env()->context(), kOnMessageBegin)
                              .ToLocalChecked();
    if (cb->IsFunction()) {
      InternalCallbackScope callback_scope(
        this, InternalCallbackScope::kSkipTaskQueues);

      MaybeLocal<Value> r = cb.As<Function>()->Call(
        env()->context(), object(), 0, nullptr);

      if (r.IsEmpty()) callback_scope.MarkAsFailed();
    }

    return 0;
  }


         Integer::New(env->isolate(), HTTP_REQUEST));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "RESPONSE"),
         Integer::New(env->isolate(), HTTP_RESPONSE));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnMessageBegin"),
         Integer::NewFromUnsigned(env->isolate(), kOnMessageBegin));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeaders"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeaders));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeadersComplete"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeadersComplete));