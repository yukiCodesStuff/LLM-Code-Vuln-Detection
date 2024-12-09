
                  if (cert = SSL_get_peer_certificate(ssl)) {
                        foo=SSL_get_verify_result(ssl);if ((X509_V_OK==foo) || (X509_V_ERRCERT_NOT_YET_VALID==foo))
                              
                                 
                                 //do stuff
                                 
                              
                           
                        
                     }
               
               