
                  #define OUTFILE "hello.out"
                     umask(0);FILE *out;
                     /* Ignore link following (CWE-59) for brevity */
                     
                     out = fopen(OUTFILE, "w");if (out) {fprintf(out, "hello world!\n");fclose(out);}
               
               