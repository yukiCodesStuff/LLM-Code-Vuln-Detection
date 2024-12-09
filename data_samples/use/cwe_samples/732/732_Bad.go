
                  
                  const adminFile = "/etc/admin-users"
                  func createAdminFileIfNotExists() error {
                     
                        file, err := os.Create(adminFile)
                        if err != nil {
                        
                           return err
                        
                        }
                        return nil
                  }
                  
                  
                  func changeModeOfAdminFile() error {
                     
                        fileMode := os.FileMode(0440)
                        if err := os.Chmod(adminFile, fileMode); err != nil {
                        
                           return err
                        
                        }
                        return nil
                  }
               
               