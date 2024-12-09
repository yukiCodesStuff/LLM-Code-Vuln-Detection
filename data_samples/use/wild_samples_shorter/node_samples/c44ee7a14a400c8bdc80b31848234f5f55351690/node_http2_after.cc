using v8::String;
using v8::Uint32;
using v8::Uint32Array;
using v8::Uint8Array;
using v8::Undefined;

using node::performance::PerformanceEntry;
namespace http2 {

  outgoing_storage_.reserve(4096);
  outgoing_buffers_.reserve(32);

  {
    // Make the js_fields_ property accessible to JS land.
    Local<ArrayBuffer> ab =
        ArrayBuffer::New(env->isolate(), js_fields_, kSessionUint8FieldCount);
    Local<Uint8Array> uint8_arr =
        Uint8Array::New(ab, 0, kSessionUint8FieldCount);
    USE(wrap->Set(env->context(), env->fields_string(), uint8_arr));
  }
}

Http2Session::~Http2Session() {
  CHECK_EQ(flags_ & SESSION_STATE_HAS_SCOPE, 0);
  // Do not report if the frame was not sent due to the session closing
  if (error_code == NGHTTP2_ERR_SESSION_CLOSING ||
      error_code == NGHTTP2_ERR_STREAM_CLOSED ||
      error_code == NGHTTP2_ERR_STREAM_CLOSING ||
      session->js_fields_[kSessionFrameErrorListenerCount] == 0) {
    return 0;
  }

  Isolate* isolate = env->isolate();
// are considered advisory only, so this has no real effect other than to
// simply let user code know that the priority has changed.
void Http2Session::HandlePriorityFrame(const nghttp2_frame* frame) {
  if (js_fields_[kSessionPriorityListenerCount] == 0) return;
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

// Called by OnFrameReceived when a complete ALTSVC frame has been received.
void Http2Session::HandleAltSvcFrame(const nghttp2_frame* frame) {
  if (!(js_fields_[kBitfield] & (1 << kSessionHasAltsvcListeners))) return;
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);
    return;
  }

  if (!(js_fields_[kBitfield] & (1 << kSessionHasPingListeners))) return;
  // Notify the session that a ping occurred
  arg = Buffer::Copy(env(),
                      reinterpret_cast<const char*>(frame->ping.opaque_data),
                      8).ToLocalChecked();
void Http2Session::HandleSettingsFrame(const nghttp2_frame* frame) {
  bool ack = frame->hd.flags & NGHTTP2_FLAG_ACK;
  if (!ack) {
    js_fields_[kBitfield] &= ~(1 << kSessionRemoteSettingsIsUpToDate);
    if (!(js_fields_[kBitfield] & (1 << kSessionHasRemoteSettingsListeners)))
      return;
    // This is not a SETTINGS acknowledgement, notify and return
    MakeCallback(env()->http2session_on_settings_function(), 0, nullptr);
    return;
  }
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_MAX_PAYLOAD_LENGTH);
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_RETURN_VALUE);

  NODE_DEFINE_CONSTANT(target, kBitfield);
  NODE_DEFINE_CONSTANT(target, kSessionPriorityListenerCount);
  NODE_DEFINE_CONSTANT(target, kSessionFrameErrorListenerCount);
  NODE_DEFINE_CONSTANT(target, kSessionUint8FieldCount);

  NODE_DEFINE_CONSTANT(target, kSessionHasRemoteSettingsListeners);
  NODE_DEFINE_CONSTANT(target, kSessionRemoteSettingsIsUpToDate);
  NODE_DEFINE_CONSTANT(target, kSessionHasPingListeners);
  NODE_DEFINE_CONSTANT(target, kSessionHasAltsvcListeners);

  // Method to fetch the nghttp2 string description of an nghttp2 error code
  env->SetMethod(target, "nghttp2ErrorString", HttpErrorString);

  Local<String> http2SessionClassName =