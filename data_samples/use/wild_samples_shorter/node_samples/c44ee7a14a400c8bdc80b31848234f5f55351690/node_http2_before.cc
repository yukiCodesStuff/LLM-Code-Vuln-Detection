using v8::String;
using v8::Uint32;
using v8::Uint32Array;
using v8::Undefined;

using node::performance::PerformanceEntry;
namespace http2 {

  outgoing_storage_.reserve(4096);
  outgoing_buffers_.reserve(32);
}

Http2Session::~Http2Session() {
  CHECK_EQ(flags_ & SESSION_STATE_HAS_SCOPE, 0);
  // Do not report if the frame was not sent due to the session closing
  if (error_code == NGHTTP2_ERR_SESSION_CLOSING ||
      error_code == NGHTTP2_ERR_STREAM_CLOSED ||
      error_code == NGHTTP2_ERR_STREAM_CLOSING) {
    return 0;
  }

  Isolate* isolate = env->isolate();
// are considered advisory only, so this has no real effect other than to
// simply let user code know that the priority has changed.
void Http2Session::HandlePriorityFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);

// Called by OnFrameReceived when a complete ALTSVC frame has been received.
void Http2Session::HandleAltSvcFrame(const nghttp2_frame* frame) {
  Isolate* isolate = env()->isolate();
  HandleScope scope(isolate);
  Local<Context> context = env()->context();
  Context::Scope context_scope(context);
    return;
  }

  // Notify the session that a ping occurred
  arg = Buffer::Copy(env(),
                      reinterpret_cast<const char*>(frame->ping.opaque_data),
                      8).ToLocalChecked();
void Http2Session::HandleSettingsFrame(const nghttp2_frame* frame) {
  bool ack = frame->hd.flags & NGHTTP2_FLAG_ACK;
  if (!ack) {
    // This is not a SETTINGS acknowledgement, notify and return
    MakeCallback(env()->http2session_on_settings_function(), 0, nullptr);
    return;
  }
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_MAX_PAYLOAD_LENGTH);
  NODE_DEFINE_CONSTANT(target, PADDING_BUF_RETURN_VALUE);

  // Method to fetch the nghttp2 string description of an nghttp2 error code
  env->SetMethod(target, "nghttp2ErrorString", HttpErrorString);

  Local<String> http2SessionClassName =