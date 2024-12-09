
                  func serve(w http.ResponseWriter, r *http.Request) {
                     
                        var body []byte
                        const MaxRespBodyLength = 1e6
                        if r.Body != nil {
                        
                           r.Body = http.MaxBytesReader(w, r.Body, MaxRespBodyLength)
                           if data, err := io.ReadAll(r.Body); err == nil {
                           
                              body = data
                           
                           }
                        }
                  }
               
            