THREADED_TEST(Regress157124) {
  v8::HandleScope scope;
  LocalContext context;
  Local<ObjectTemplate> templ = ObjectTemplate::New();
  Local<Object> obj = templ->NewInstance();
  obj->GetIdentityHash();
  obj->DeleteHiddenValue(v8_str("Bug"));
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