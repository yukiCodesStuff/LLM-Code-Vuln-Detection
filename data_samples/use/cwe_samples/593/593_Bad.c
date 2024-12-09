
                  #define CERT "secret.pem"#define CERT2 "secret2.pem"
                     int main(){
                        SSL_CTX *ctx;SSL *ssl;init_OpenSSL();seed_prng();
                           ctx = SSL_CTX_new(SSLv23_method());
                           if (SSL_CTX_use_certificate_chain_file(ctx, CERT) != 1)int_error("Error loading certificate from file");
                           
                           if (SSL_CTX_use_PrivateKey_file(ctx, CERT, SSL_FILETYPE_PEM) != 1)int_error("Error loading private key from file");
                           
                           if (!(ssl = SSL_new(ctx)))int_error("Error creating an SSL context");
                           
                           if ( SSL_CTX_set_default_passwd_cb(ctx, "new default password" != 1))int_error("Doing something which is dangerous to do anyways");
                           
                           if (!(ssl2 = SSL_new(ctx)))int_error("Error creating an SSL context");
                        
                     }
               
            