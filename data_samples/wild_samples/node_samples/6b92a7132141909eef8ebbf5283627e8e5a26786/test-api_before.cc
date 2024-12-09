THREADED_TEST(Regress157124) {
  v8::HandleScope scope;
  LocalContext context;
  Local<ObjectTemplate> templ = ObjectTemplate::New();
  Local<Object> obj = templ->NewInstance();
  obj->GetIdentityHash();
  obj->DeleteHiddenValue(v8_str("Bug"));
}


#ifndef WIN32
class ThreadInterruptTest {
 public:
  ThreadInterruptTest() : sem_(NULL), sem_value_(0) { }