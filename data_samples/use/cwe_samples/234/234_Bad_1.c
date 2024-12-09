
                  void some_function(int foo, ...) {int a[3], i;va_list ap;va_start(ap, foo);for (i = 0; i < sizeof(a) / sizeof(int); i++) a[i] = va_arg(ap, int);va_end(ap);}
                     int main(int argc, char *argv[]) {some_function(17, 42);}
               
               