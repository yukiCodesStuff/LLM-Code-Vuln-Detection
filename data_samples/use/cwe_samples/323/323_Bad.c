
                  void encryptAndSendPassword(char *password){char *nonce = "bad";...char *data = (unsigned char*)malloc(20);int para_size = strlen(nonce) + strlen(password);char *paragraph = (char*)malloc(para_size);SHA1((const unsigned char*)paragraph,parsize,(unsigned char*)data);sendEncryptedData(data)}
               
               