
                  void GetData(char *MFAddr) {
                        char pwd[64];if (GetPasswordFromUser(pwd, sizeof(pwd))) {
                              
                                 if (ConnectToMainframe(MFAddr, pwd)) {
                                    
                                       
                                       // Interaction with mainframe
                                       
                                    
                                 }
                           }memset(pwd, 0, sizeof(pwd));
                     }
               
               