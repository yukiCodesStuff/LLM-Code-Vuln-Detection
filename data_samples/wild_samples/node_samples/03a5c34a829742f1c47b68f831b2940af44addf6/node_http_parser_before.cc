namespace node {
namespace {  // NOLINT(build/namespaces)

using v8::Array;
using v8::Boolean;
using v8::Context;
using v8::EscapableHandleScope;
using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::Int32;
using v8::Integer;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Uint32;
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

const uint32_t kLenientNone = 0;
const uint32_t kLenientHeaders = 1 << 0;
const uint32_t kLenientChunkedLength = 1 << 1;
const uint32_t kLenientKeepAlive = 1 << 2;
const uint32_t kLenientTransferEncoding = 1 << 3;
const uint32_t kLenientVersion = 1 << 4;
const uint32_t kLenientDataAfterClose = 1 << 5;
const uint32_t kLenientOptionalLFAfterCR = 1 << 6;
const uint32_t kLenientOptionalCRLFAfterChunk = 1 << 7;
const uint32_t kLenientOptionalCRBeforeLF = 1 << 8;
const uint32_t kLenientSpacesAfterChunkSize = 1 << 9;
const uint32_t kLenientAll =
    kLenientHeaders | kLenientChunkedLength | kLenientKeepAlive |
    kLenientTransferEncoding | kLenientVersion | kLenientDataAfterClose |
    kLenientOptionalLFAfterCR | kLenientOptionalCRLFAfterChunk |
    kLenientOptionalCRBeforeLF | kLenientSpacesAfterChunkSize;

inline bool IsOWS(char c) {
  return c == ' ' || c == '\t';
}
    if (connectionsList_ != nullptr) {
      connectionsList_->Pop(this);
      connectionsList_->PopActive(this);
    }

    num_fields_ = num_values_ = 0;
    headers_completed_ = false;
    last_message_start_ = uv_hrtime();
    url_.Reset();
    status_message_.Reset();

    if (connectionsList_ != nullptr) {
      connectionsList_->Push(this);
      connectionsList_->PushActive(this);
    }
    if (r.IsEmpty()) {
      got_exception_ = true;
      return -1;
    }

    return 0;
  }

  // Reset nread for the next chunk
  int on_chunk_header() {
    header_nread_ = 0;
    return 0;
  }


  // Reset nread for the next chunk
  int on_chunk_complete() {
    header_nread_ = 0;
    return 0;
  }

  static void New(const FunctionCallbackInfo<Value>& args) {
    BindingData* binding_data = Realm::GetBindingData<BindingData>(args);
    new Parser(binding_data, args.This());
  }


  static void Close(const FunctionCallbackInfo<Value>& args) {
    Parser* parser;
    ASSIGN_OR_RETURN_UNWRAP(&parser, args.Holder());

    delete parser;
  }


  static void Free(const FunctionCallbackInfo<Value>& args) {
    Parser* parser;
    ASSIGN_OR_RETURN_UNWRAP(&parser, args.Holder());

    // Since the Parser destructor isn't going to run the destroy() callbacks
    // it needs to be triggered manually.
    parser->EmitTraceEventDestroy();
    parser->EmitDestroy();
  }

  static void Remove(const FunctionCallbackInfo<Value>& args) {
    Parser* parser;
    ASSIGN_OR_RETURN_UNWRAP(&parser, args.Holder());

    if (parser->connectionsList_ != nullptr) {
      parser->connectionsList_->Pop(parser);
      parser->connectionsList_->PopActive(parser);
    }
  bool IsNotIndicativeOfMemoryLeakAtExit() const override {
    // HTTP parsers are able to emit events without any GC root referring
    // to them, because they receive events directly from the underlying
    // libuv resource.
    return true;
  }


  llhttp_t parser_;
  StringPtr fields_[kMaxHeaderFieldsCount];  // header fields
  StringPtr values_[kMaxHeaderFieldsCount];  // header values
  StringPtr url_;
  StringPtr status_message_;
  size_t num_fields_;
  size_t num_values_;
  bool have_flushed_;
  bool got_exception_;
  size_t current_buffer_len_;
  const char* current_buffer_data_;
  bool headers_completed_ = false;
  bool pending_pause_ = false;
  uint64_t header_nread_ = 0;
  uint64_t max_http_header_size_;
  uint64_t last_message_start_;
  ConnectionsList* connectionsList_;

  BaseObjectPtr<BindingData> binding_data_;

  // These are helper functions for filling `http_parser_settings`, which turn
  // a member function of Parser into a C-style HTTP parser callback.
  template <typename Parser, Parser> struct Proxy;
  template <typename Parser, typename ...Args, int (Parser::*Member)(Args...)>
  struct Proxy<int (Parser::*)(Args...), Member> {
    static int Raw(llhttp_t* p, Args ... args) {
      Parser* parser = ContainerOf(&Parser::parser_, p);
      int rv = (parser->*Member)(std::forward<Args>(args)...);
      if (rv == 0) {
        rv = parser->MaybePause();
      }
      return rv;
    }
  };

  typedef int (Parser::*Call)();
  typedef int (Parser::*DataCall)(const char* at, size_t length);

  static const llhttp_settings_t settings;
};

bool ParserComparator::operator()(const Parser* lhs, const Parser* rhs) const {
  if (lhs->last_message_start_ == 0 && rhs->last_message_start_ == 0) {
    // When both parsers are idle, guarantee strict order by
    // comparing pointers as ints.
    return lhs < rhs;
  } else if (lhs->last_message_start_ == 0) {
    return true;
  } else if (rhs->last_message_start_ == 0) {
    return false;
  }

  return lhs->last_message_start_ < rhs->last_message_start_;
}

void ConnectionsList::New(const FunctionCallbackInfo<Value>& args) {
  Local<Context> context = args.GetIsolate()->GetCurrentContext();
  Environment* env = Environment::GetCurrent(context);

  new ConnectionsList(env, args.This());
}

void ConnectionsList::All(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  ConnectionsList* list;

  ASSIGN_OR_RETURN_UNWRAP(&list, args.Holder());

  std::vector<Local<Value>> result;
  result.reserve(list->all_connections_.size());
  for (auto parser : list->all_connections_) {
    result.emplace_back(parser->object());
  }

  return args.GetReturnValue().Set(
      Array::New(isolate, result.data(), result.size()));
}

void ConnectionsList::Idle(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  ConnectionsList* list;

  ASSIGN_OR_RETURN_UNWRAP(&list, args.Holder());

  std::vector<Local<Value>> result;
  result.reserve(list->all_connections_.size());
  for (auto parser : list->all_connections_) {
    if (parser->last_message_start_ == 0) {
      result.emplace_back(parser->object());
    }
  }

  return args.GetReturnValue().Set(
      Array::New(isolate, result.data(), result.size()));
}

void ConnectionsList::Active(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  ConnectionsList* list;

  ASSIGN_OR_RETURN_UNWRAP(&list, args.Holder());

  std::vector<Local<Value>> result;
  result.reserve(list->active_connections_.size());
  for (auto parser : list->active_connections_) {
    result.emplace_back(parser->object());
  }

  return args.GetReturnValue().Set(
      Array::New(isolate, result.data(), result.size()));
}

void ConnectionsList::Expired(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  ConnectionsList* list;

  ASSIGN_OR_RETURN_UNWRAP(&list, args.Holder());
  CHECK(args[0]->IsNumber());
  CHECK(args[1]->IsNumber());
  uint64_t headers_timeout =
    static_cast<uint64_t>(args[0].As<Uint32>()->Value()) * 1000000;
  uint64_t request_timeout =
    static_cast<uint64_t>(args[1].As<Uint32>()->Value()) * 1000000;

  if (headers_timeout == 0 && request_timeout == 0) {
    return args.GetReturnValue().Set(Array::New(isolate, 0));
  } else if (request_timeout > 0 && headers_timeout > request_timeout) {
    std::swap(headers_timeout, request_timeout);
  }

  // On IoT or embedded devices the uv_hrtime() may return the timestamp
  // that is smaller than configured timeout for headers or request
  // to prevent subtracting two unsigned integers
  // that can yield incorrect results we should check
  // if the 'now' is bigger than the timeout for headers or request
  const uint64_t now = uv_hrtime();
  const uint64_t headers_deadline =
      (headers_timeout > 0 && now > headers_timeout) ? now - headers_timeout
                                                     : 0;
  const uint64_t request_deadline =
      (request_timeout > 0 && now > request_timeout) ? now - request_timeout
                                                     : 0;

  if (headers_deadline == 0 && request_deadline == 0) {
    return args.GetReturnValue().Set(Array::New(isolate, 0));
  }

  auto iter = list->active_connections_.begin();
  auto end = list->active_connections_.end();

  std::vector<Local<Value>> result;
  result.reserve(list->active_connections_.size());
  while (iter != end) {
    Parser* parser = *iter;
    iter++;

    // Check for expiration.
    if (
      (!parser->headers_completed_ && headers_deadline > 0 &&
        parser->last_message_start_ < headers_deadline) ||
      (
        request_deadline > 0 &&
        parser->last_message_start_ < request_deadline)
    ) {
      result.emplace_back(parser->object());

      list->active_connections_.erase(parser);
    }
  }

  return args.GetReturnValue().Set(
      Array::New(isolate, result.data(), result.size()));
}

const llhttp_settings_t Parser::settings = {
    Proxy<Call, &Parser::on_message_begin>::Raw,
    Proxy<DataCall, &Parser::on_url>::Raw,
    Proxy<DataCall, &Parser::on_status>::Raw,

    // on_method
    nullptr,
    // on_version
    nullptr,

    Proxy<DataCall, &Parser::on_header_field>::Raw,
    Proxy<DataCall, &Parser::on_header_value>::Raw,

    // on_chunk_extension_name
    nullptr,
    // on_chunk_extension_value
    nullptr,

    Proxy<Call, &Parser::on_headers_complete>::Raw,
    Proxy<DataCall, &Parser::on_body>::Raw,
    Proxy<Call, &Parser::on_message_complete>::Raw,

    // on_url_complete
    nullptr,
    // on_status_complete
    nullptr,
    // on_method_complete
    nullptr,
    // on_version_complete
    nullptr,
    // on_header_field_complete
    nullptr,
    // on_header_value_complete
    nullptr,
    // on_chunk_extension_name_complete
    nullptr,
    // on_chunk_extension_value_complete
    nullptr,

    Proxy<Call, &Parser::on_chunk_header>::Raw,
    Proxy<Call, &Parser::on_chunk_complete>::Raw,

    // on_reset,
    nullptr,
};

void InitializeHttpParser(Local<Object> target,
                          Local<Value> unused,
                          Local<Context> context,
                          void* priv) {
  Realm* realm = Realm::GetCurrent(context);
  Environment* env = realm->env();
  Isolate* isolate = env->isolate();
  BindingData* const binding_data = realm->AddBindingData<BindingData>(target);
  if (binding_data == nullptr) return;

  Local<FunctionTemplate> t = NewFunctionTemplate(isolate, Parser::New);
  t->InstanceTemplate()->SetInternalFieldCount(Parser::kInternalFieldCount);

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "REQUEST"),
         Integer::New(env->isolate(), HTTP_REQUEST));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "RESPONSE"),
         Integer::New(env->isolate(), HTTP_RESPONSE));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnMessageBegin"),
         Integer::NewFromUnsigned(env->isolate(), kOnMessageBegin));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeaders"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeaders));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeadersComplete"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeadersComplete));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnBody"),
         Integer::NewFromUnsigned(env->isolate(), kOnBody));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnMessageComplete"),
         Integer::NewFromUnsigned(env->isolate(), kOnMessageComplete));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnExecute"),
         Integer::NewFromUnsigned(env->isolate(), kOnExecute));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnTimeout"),
         Integer::NewFromUnsigned(env->isolate(), kOnTimeout));

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientNone"),
         Integer::NewFromUnsigned(env->isolate(), kLenientNone));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientHeaders"),
         Integer::NewFromUnsigned(env->isolate(), kLenientHeaders));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientChunkedLength"),
         Integer::NewFromUnsigned(env->isolate(), kLenientChunkedLength));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientKeepAlive"),
         Integer::NewFromUnsigned(env->isolate(), kLenientKeepAlive));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientTransferEncoding"),
         Integer::NewFromUnsigned(env->isolate(), kLenientTransferEncoding));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientVersion"),
         Integer::NewFromUnsigned(env->isolate(), kLenientVersion));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientDataAfterClose"),
         Integer::NewFromUnsigned(env->isolate(), kLenientDataAfterClose));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalLFAfterCR"),
         Integer::NewFromUnsigned(env->isolate(), kLenientOptionalLFAfterCR));
  t->Set(
      FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalCRLFAfterChunk"),
      Integer::NewFromUnsigned(env->isolate(), kLenientOptionalCRLFAfterChunk));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalCRBeforeLF"),
         Integer::NewFromUnsigned(env->isolate(), kLenientOptionalCRBeforeLF));
  t->Set(
      FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientSpacesAfterChunkSize"),
      Integer::NewFromUnsigned(env->isolate(), kLenientSpacesAfterChunkSize));

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientAll"),
         Integer::NewFromUnsigned(env->isolate(), kLenientAll));

  Local<Array> methods = Array::New(env->isolate());
#define V(num, name, string)                                                  \
    methods->Set(env->context(),                                              \
        num, FIXED_ONE_BYTE_STRING(env->isolate(), #string)).Check();
  HTTP_METHOD_MAP(V)
#undef V
  target->Set(env->context(),
              FIXED_ONE_BYTE_STRING(env->isolate(), "methods"),
              methods).Check();

  t->Inherit(AsyncWrap::GetConstructorTemplate(env));
  SetProtoMethod(isolate, t, "close", Parser::Close);
  SetProtoMethod(isolate, t, "free", Parser::Free);
  SetProtoMethod(isolate, t, "remove", Parser::Remove);
  SetProtoMethod(isolate, t, "execute", Parser::Execute);
  SetProtoMethod(isolate, t, "finish", Parser::Finish);
  SetProtoMethod(isolate, t, "initialize", Parser::Initialize);
  SetProtoMethod(isolate, t, "pause", Parser::Pause<true>);
  SetProtoMethod(isolate, t, "resume", Parser::Pause<false>);
  SetProtoMethod(isolate, t, "consume", Parser::Consume);
  SetProtoMethod(isolate, t, "unconsume", Parser::Unconsume);
  SetProtoMethod(isolate, t, "getCurrentBuffer", Parser::GetCurrentBuffer);
  SetProtoMethod(isolate, t, "duration", Parser::Duration);
  SetProtoMethod(isolate, t, "headersCompleted", Parser::HeadersCompleted);

  SetConstructorFunction(context, target, "HTTPParser", t);

  Local<FunctionTemplate> c =
      NewFunctionTemplate(isolate, ConnectionsList::New);
  c->InstanceTemplate()
    ->SetInternalFieldCount(ConnectionsList::kInternalFieldCount);
  SetProtoMethod(isolate, c, "all", ConnectionsList::All);
  SetProtoMethod(isolate, c, "idle", ConnectionsList::Idle);
  SetProtoMethod(isolate, c, "active", ConnectionsList::Active);
  SetProtoMethod(isolate, c, "expired", ConnectionsList::Expired);
  SetConstructorFunction(context, target, "ConnectionsList", c);
}

}  // anonymous namespace
}  // namespace node

NODE_BINDING_CONTEXT_AWARE_INTERNAL(http_parser, node::InitializeHttpParser)
const llhttp_settings_t Parser::settings = {
    Proxy<Call, &Parser::on_message_begin>::Raw,
    Proxy<DataCall, &Parser::on_url>::Raw,
    Proxy<DataCall, &Parser::on_status>::Raw,

    // on_method
    nullptr,
    // on_version
    nullptr,

    Proxy<DataCall, &Parser::on_header_field>::Raw,
    Proxy<DataCall, &Parser::on_header_value>::Raw,

    // on_chunk_extension_name
    nullptr,
    // on_chunk_extension_value
    nullptr,

    Proxy<Call, &Parser::on_headers_complete>::Raw,
    Proxy<DataCall, &Parser::on_body>::Raw,
    Proxy<Call, &Parser::on_message_complete>::Raw,

    // on_url_complete
    nullptr,
    // on_status_complete
    nullptr,
    // on_method_complete
    nullptr,
    // on_version_complete
    nullptr,
    // on_header_field_complete
    nullptr,
    // on_header_value_complete
    nullptr,
    // on_chunk_extension_name_complete
    nullptr,
    // on_chunk_extension_value_complete
    nullptr,

    Proxy<Call, &Parser::on_chunk_header>::Raw,
    Proxy<Call, &Parser::on_chunk_complete>::Raw,

    // on_reset,
    nullptr,
};

void InitializeHttpParser(Local<Object> target,
                          Local<Value> unused,
                          Local<Context> context,
                          void* priv) {
  Realm* realm = Realm::GetCurrent(context);
  Environment* env = realm->env();
  Isolate* isolate = env->isolate();
  BindingData* const binding_data = realm->AddBindingData<BindingData>(target);
  if (binding_data == nullptr) return;

  Local<FunctionTemplate> t = NewFunctionTemplate(isolate, Parser::New);
  t->InstanceTemplate()->SetInternalFieldCount(Parser::kInternalFieldCount);

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "REQUEST"),
         Integer::New(env->isolate(), HTTP_REQUEST));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "RESPONSE"),
         Integer::New(env->isolate(), HTTP_RESPONSE));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnMessageBegin"),
         Integer::NewFromUnsigned(env->isolate(), kOnMessageBegin));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeaders"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeaders));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnHeadersComplete"),
         Integer::NewFromUnsigned(env->isolate(), kOnHeadersComplete));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnBody"),
         Integer::NewFromUnsigned(env->isolate(), kOnBody));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnMessageComplete"),
         Integer::NewFromUnsigned(env->isolate(), kOnMessageComplete));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnExecute"),
         Integer::NewFromUnsigned(env->isolate(), kOnExecute));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kOnTimeout"),
         Integer::NewFromUnsigned(env->isolate(), kOnTimeout));

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientNone"),
         Integer::NewFromUnsigned(env->isolate(), kLenientNone));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientHeaders"),
         Integer::NewFromUnsigned(env->isolate(), kLenientHeaders));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientChunkedLength"),
         Integer::NewFromUnsigned(env->isolate(), kLenientChunkedLength));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientKeepAlive"),
         Integer::NewFromUnsigned(env->isolate(), kLenientKeepAlive));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientTransferEncoding"),
         Integer::NewFromUnsigned(env->isolate(), kLenientTransferEncoding));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientVersion"),
         Integer::NewFromUnsigned(env->isolate(), kLenientVersion));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientDataAfterClose"),
         Integer::NewFromUnsigned(env->isolate(), kLenientDataAfterClose));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalLFAfterCR"),
         Integer::NewFromUnsigned(env->isolate(), kLenientOptionalLFAfterCR));
  t->Set(
      FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalCRLFAfterChunk"),
      Integer::NewFromUnsigned(env->isolate(), kLenientOptionalCRLFAfterChunk));
  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientOptionalCRBeforeLF"),
         Integer::NewFromUnsigned(env->isolate(), kLenientOptionalCRBeforeLF));
  t->Set(
      FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientSpacesAfterChunkSize"),
      Integer::NewFromUnsigned(env->isolate(), kLenientSpacesAfterChunkSize));

  t->Set(FIXED_ONE_BYTE_STRING(env->isolate(), "kLenientAll"),
         Integer::NewFromUnsigned(env->isolate(), kLenientAll));

  Local<Array> methods = Array::New(env->isolate());
#define V(num, name, string)                                                  \
    methods->Set(env->context(),                                              \
        num, FIXED_ONE_BYTE_STRING(env->isolate(), #string)).Check();
  HTTP_METHOD_MAP(V)
#undef V
  target->Set(env->context(),
              FIXED_ONE_BYTE_STRING(env->isolate(), "methods"),
              methods).Check();

  t->Inherit(AsyncWrap::GetConstructorTemplate(env));
  SetProtoMethod(isolate, t, "close", Parser::Close);
  SetProtoMethod(isolate, t, "free", Parser::Free);
  SetProtoMethod(isolate, t, "remove", Parser::Remove);
  SetProtoMethod(isolate, t, "execute", Parser::Execute);
  SetProtoMethod(isolate, t, "finish", Parser::Finish);
  SetProtoMethod(isolate, t, "initialize", Parser::Initialize);
  SetProtoMethod(isolate, t, "pause", Parser::Pause<true>);
  SetProtoMethod(isolate, t, "resume", Parser::Pause<false>);
  SetProtoMethod(isolate, t, "consume", Parser::Consume);
  SetProtoMethod(isolate, t, "unconsume", Parser::Unconsume);
  SetProtoMethod(isolate, t, "getCurrentBuffer", Parser::GetCurrentBuffer);
  SetProtoMethod(isolate, t, "duration", Parser::Duration);
  SetProtoMethod(isolate, t, "headersCompleted", Parser::HeadersCompleted);

  SetConstructorFunction(context, target, "HTTPParser", t);

  Local<FunctionTemplate> c =
      NewFunctionTemplate(isolate, ConnectionsList::New);
  c->InstanceTemplate()
    ->SetInternalFieldCount(ConnectionsList::kInternalFieldCount);
  SetProtoMethod(isolate, c, "all", ConnectionsList::All);
  SetProtoMethod(isolate, c, "idle", ConnectionsList::Idle);
  SetProtoMethod(isolate, c, "active", ConnectionsList::Active);
  SetProtoMethod(isolate, c, "expired", ConnectionsList::Expired);
  SetConstructorFunction(context, target, "ConnectionsList", c);
}