extern int wasm_on_status(llhttp_t* p, const char* at, size_t length);
extern int wasm_on_header_field(llhttp_t* p, const char* at, size_t length);
extern int wasm_on_header_value(llhttp_t* p, const char* at, size_t length);
extern int wasm_on_headers_complete(llhttp_t * p, int status_code,
                                    uint8_t upgrade, int should_keep_alive);
extern int wasm_on_body(llhttp_t* p, const char* at, size_t length);
extern int wasm_on_message_complete(llhttp_t * p);

static int wasm_on_headers_complete_wrap(llhttp_t* p) {
  return wasm_on_headers_complete(p, p->status_code, p->upgrade,
                                  llhttp_should_keep_alive(p));
}

const llhttp_settings_t wasm_settings = {
  wasm_on_message_begin,
  wasm_on_url,
  wasm_on_status,
  wasm_on_header_field,
  wasm_on_header_value,
  wasm_on_headers_complete_wrap,
  wasm_on_body,
  wasm_on_message_complete,
  NULL,
  NULL,