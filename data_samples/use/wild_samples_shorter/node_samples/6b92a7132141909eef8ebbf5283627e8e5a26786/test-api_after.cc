}


THREADED_TEST(Regress260106) {
  LocalContext context;
  v8::HandleScope scope(context->GetIsolate());
  Local<FunctionTemplate> templ = FunctionTemplate::New(DummyCallHandler);
  CompileRun("for (var i = 0; i < 128; i++) Object.prototype[i] = 0;");
  Local<Function> function = templ->GetFunction();
  CHECK(!function.IsEmpty());
  CHECK(function->IsFunction());
}


#ifndef WIN32
class ThreadInterruptTest {
 public:
  ThreadInterruptTest() : sem_(NULL), sem_value_(0) { }