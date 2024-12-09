
                  EVP_CIPHER_CTX ctx;char key[EVP_MAX_KEY_LENGTH];char iv[EVP_MAX_IV_LENGTH];RAND_bytes(key, b);memset(iv,0,EVP_MAX_IV_LENGTH);EVP_EncryptInit(&ctx,EVP_bf_cbc(), key,iv);
               
               