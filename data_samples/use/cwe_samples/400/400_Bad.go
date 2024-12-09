
                  func serve(w http.ResponseWriter, r *http.Request) {
                     
                        var body []byte
                        if r.Body != nil {
                        
                           if data, err := io.ReadAll(r.Body); err == nil {
                           
                              body = data
                           
                           }
                        }
                  }
               
               