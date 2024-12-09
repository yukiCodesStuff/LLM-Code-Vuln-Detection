
                  func ExecuteGitCommand(name string, arg []string) error {
                     
                        c := exec.Command(name, arg...)
                        var err error
                        c.Path, err = exec.LookPath(name)
                        if err != nil {
                           
                              return err
                           
                        }
                     }
               
               