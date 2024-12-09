
                  unsigned char *simple_digest(char *alg,char *buf,unsigned int len, int *olen) {const EVP_MD *m;EVP_MD_CTX ctx;unsigned char *ret;OpenSSL_add_all_digests();if (!(m = EVP_get_digestbyname(alg))) return NULL;if (!(ret = (unsigned char*)malloc(EVP_MAX_MD_SIZE))) return NULL;EVP_DigestInit(&ctx, m);EVP_DigestUpdate(&ctx,buf,len);EVP_DigestFinal(&ctx,ret,olen);return ret;}unsigned char *generate_password_and_cmd(char *password_and_cmd) {simple_digest("sha1",password,strlen(password_and_cmd)...);}
               
               